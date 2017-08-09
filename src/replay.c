#include "replay.h"
#include "device_file.h"
#include "event.h"
#include "keyboard.h"
#include "mouse.h"
#include "signal.h"
#include "util.h"
#include <X11/Xlib.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <linux/uinput.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static volatile sig_atomic_t is_running = true;

void print_log(int32_t evtfd, uint64_t evt_type_bits, struct input_event evt,
               int32_t mouse_x, int32_t mouse_y) {
  uint8_t devname[BUF_SIZE] = {0};

  get_devname(evtfd, devname);
  if (is_mouse_move_event(evt)) {
    printf(MOUSE_MOVE_OUT_FORMAT, devname, evt_type_bits, evt.time.tv_sec,
           evt.time.tv_usec, evt.type, evt.code, evt.value, mouse_x, mouse_y);
  } else {
    printf(OUT_FORMAT, devname, evt_type_bits, evt.time.tv_sec,
           evt.time.tv_usec, evt.type, evt.code, evt.value);
  }
}

static bool is_timeval_initialized(struct timeval *time) {
  if (time->tv_sec == 0 && time->tv_usec == 0) {
    return true;
  } else {
    return false;
  }
}

static void wait(struct timeval now, struct timeval *p_prev) {
  if (is_timeval_initialized(p_prev)) {
    *p_prev = now;
    return;
  }

  useconds_t dsec = now.tv_sec - p_prev->tv_sec;
  useconds_t dusec = dsec * 1000000 + (now.tv_usec - p_prev->tv_usec);

  if (dusec > 0) {
    usleep(dusec);
  }

  *p_prev = now;
}

static bool is_invalid_line(char line[], int32_t size) {
  uint32_t ntab = 0;

  for (uint32_t i = 0; i < size; i++) {
    if (line[i] == '\n')
      break;
    if (line[i] == '\t')
      ntab++;
  }

  return ntab >= CORRECT_NUM_TAB_PER_LINE;
}

static int32_t read_line(FILE *recfp, uint64_t *p_evt_type_bits,
                         struct input_event *p_evt, int32_t *p_mouse_x,
                         int32_t *p_mouse_y) {
  char buf[BUF_SIZE] = {0};
  char *p_ret = fgets(buf, sizeof(buf), recfp);
  if (p_ret == NULL || !is_invalid_line(buf, sizeof(buf))) {
    return EOF;
  }

  uint64_t tmp_evt_type_bits = 0;
  int32_t tmp_mouse_x = 0;
  int32_t tmp_mouse_y = 0;
  uint8_t unused_devname[BUF_SIZE] = {0};
  struct input_event tmp_evt = {};

  int32_t ret =
      sscanf(buf, MOUSE_MOVE_IN_FORMAT, unused_devname, &tmp_evt_type_bits,
             &tmp_evt.time.tv_sec, &tmp_evt.time.tv_usec, &tmp_evt.type,
             &tmp_evt.code, &tmp_evt.value, &tmp_mouse_x, &tmp_mouse_y);

  if (ret < 0) {
    fprintf(stderr, "sscanf() failed: %s\n", strerror(errno));
  }

  *p_evt_type_bits = tmp_evt_type_bits;
  *p_evt = tmp_evt;
  *p_mouse_x = tmp_mouse_x;
  *p_mouse_y = tmp_mouse_y;

  return NOT_EOF;
}

int32_t get_matching_evtfd(int32_t evtfd[], uint32_t ndev,
                           uint64_t query_evt_type_bits) {
  for (uint32_t i = 0; i < ndev; i++) {
    uint64_t target_evt_type_bits = 0;
    get_evt_types(evtfd[i], &target_evt_type_bits);
    if (query_evt_type_bits == target_evt_type_bits) {
      return evtfd[i];
    }
  }

  fprintf(stderr,
          "Can't find the input device whose 'all event types' (logical AND) "
          "is %" PRIu64 ".\n"
          "This may be happened because this computer doesn't have a needed "
          "input device.\n",
          query_evt_type_bits);
  return NOT_FOUND;
}

static void move_mouse_cursor(Display *display, int32_t mouse_x,
                              int32_t mouse_y) {
  XWarpPointer(display, None, XDefaultRootWindow(display), 0, 0, 0, 0, mouse_x,
               mouse_y);
  XFlush(display);
}

static bool is_mouse_position_changed(int32_t current_mouse_x,
                                      int32_t current_mouse_y,
                                      int32_t *p_prev_mouse_x,
                                      int32_t *p_prev_mouse_y) {
  if (current_mouse_x != *p_prev_mouse_x ||
      current_mouse_y != *p_prev_mouse_y) {
    *p_prev_mouse_x = current_mouse_x;
    *p_prev_mouse_y = current_mouse_y;
    return true;
  } else {
    return false;
  }
}

static void sig_handler(int sig) {
  is_running = false;
}

void write_all_events(FILE *recfp, bool is_verbose) {
  // Variables
  struct dirent **namelist = NULL;
  int32_t evtfd[MAX_NUM_DEV_FILE] = {0};
  struct timeval prev_time = {0};

  // Xlib variables
  Display *display;
  display = XOpenDisplay(0);
  int32_t current_mouse_x = 0;
  int32_t current_mouse_y = 0;
  int32_t prev_mouse_x = 0;
  int32_t prev_mouse_y = 0;

  // This function starts from here
  set_sig_handler(sig_handler);

  uint32_t ndev = enumerate_all_device_files(&namelist);
  open_all_device_files(evtfd, ndev, namelist, O_WRONLY | O_SYNC);

  for (uint32_t i = 0; i < ndev; i++) {
    free(namelist[i]);
  }
  free(namelist);

  fprintf(stderr, "Replaying ...\n");
  while (is_running) {
    uint64_t evt_type_bits = 0;
    struct input_event evt = {};

    int32_t ret = read_line(recfp, &evt_type_bits, &evt, &current_mouse_x,
                            &current_mouse_y);
    if (ret == EOF) {
      break;
    }

    int32_t matching_evtfd = get_matching_evtfd(evtfd, ndev, evt_type_bits);
    if (matching_evtfd == NOT_FOUND) {
      continue;
    }

    wait(evt.time, &prev_time);

    if (is_verbose) {
      print_log(matching_evtfd, evt_type_bits, evt, current_mouse_x,
                current_mouse_y);
    }

    if (is_mouse_move_event(evt)) {
      if (is_mouse_position_changed(current_mouse_x, current_mouse_y,
                                    &prev_mouse_x, &prev_mouse_y)) {
        move_mouse_cursor(display, current_mouse_x, current_mouse_y);
      }
    } else {
      write_event(matching_evtfd, evt.type, evt.code, evt.value);
    }
  }

  // If this program is executed on the computer directly
  // not via SSH, the unnecessary Ctrl+C key stroke,
  // which terminates this program, is recorded.
  // In that case, pushed Ctrl+C state is recorded,
  // but released Ctrl+C state is not recorded.
  // Therefore, after replaying it, Ctrl+C key will be
  // continued to be pushed forever until it is released.
  release_ctrl_c_key(evtfd, ndev);

  XCloseDisplay(display);
  close_all_device_files(evtfd, ndev);
}

void replay(char *filename, bool is_verbose) {
  FILE *recfp = NULL;

  recfp = fopen(filename, "r");
  if (recfp == NULL) {
    die("Can't open '%s'", filename);
  }

  write_all_events(recfp, is_verbose);
  fclose(recfp);
}
