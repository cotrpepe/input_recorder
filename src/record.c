#include "record.h"
#include "device_file.h"
#include "event.h"
#include "mouse.h"
#include "signal.h"
#include "util.h"
#include <X11/Xlib.h>
#include <fcntl.h>
#include <linux/input.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static volatile sig_atomic_t is_running = true;

static uint32_t get_max_fd(int32_t evtfd[], uint32_t ndev) {
  uint32_t max_evtfd = 0;

  for (uint32_t i = 0; i < ndev; i++) {
    if (max_evtfd < evtfd[i]) {
      max_evtfd = evtfd[i];
    }
  }

  return max_evtfd;
}

static void set_all_evtfd(int32_t evtfd[], uint32_t ndev, fd_set *p_evtfd_set) {
  for (uint32_t i = 0; i < ndev; i++) {
    FD_SET(evtfd[i], p_evtfd_set);
  }
}

static uint64_t sec2nanosec(double sec) {
  return (uint64_t)(sec * 100000000);
}

static void set_pselect(int32_t max_evtfd, fd_set *p_evtfd_set) {
  sigset_t sigset;
  sigemptyset(&sigset);
  sigaddset(&sigset, SIGINT);

  struct timespec ts = {0};
  ts.tv_nsec = sec2nanosec(0.5);

  int32_t ret = pselect(max_evtfd + 1, p_evtfd_set, NULL, NULL, &ts, &sigset);
  if (ret < 0) {
    fprintf(stderr, "pselect() failed\n");
  }
}

static void get_evt(int32_t evtfd, struct input_event *p_evt) {
  int32_t ret = read(evtfd, p_evt, sizeof(*p_evt));
  if (ret < 0) {
    die("read() failed");
  }
}

static void get_absolute_mouse_position(Display *display, int32_t *p_mouse_x,
                                        int32_t *p_mouse_y) {
  int32_t unused_win_x = 0;
  int32_t unused_win_y = 0;
  uint32_t unused_mask = 0;
  Window unused_root_return = 0;
  Window unused_child_return = 0;

  XQueryPointer(display, XDefaultRootWindow(display), &unused_root_return,
                &unused_child_return, p_mouse_x, p_mouse_y, &unused_win_x,
                &unused_win_y, &unused_mask);
}

static void sig_handler(int32_t sig) {
  is_running = false;
}

void record() {
  // Variables
  struct dirent **namelist = NULL;
  int32_t evtfd[MAX_NUM_DEV_FILE] = {0};

  // Xlib variables
  Display *display = XOpenDisplay(NULL);
  int32_t mouse_x = 0;
  int32_t mouse_y = 0;

  // This function starts from here
  uint32_t ndev = enumerate_all_device_files(&namelist);
  open_all_device_files(evtfd, ndev, namelist, O_RDONLY);

  for (uint32_t i = 0; i < ndev; i++) {
    free(namelist[i]);
  }
  free(namelist);

  uint32_t max_evtfd = get_max_fd(evtfd, ndev);

  set_sig_handler(sig_handler);

  fprintf(stderr, "Recording ...\n");
  while (is_running) {
    fd_set evtfd_set;

    FD_ZERO(&evtfd_set);
    set_all_evtfd(evtfd, ndev, &evtfd_set);
    set_pselect(max_evtfd, &evtfd_set);

    for (uint32_t i = 0; i < ndev; i++) {
      if (FD_ISSET(evtfd[i], &evtfd_set)) {
        uint64_t evt_type_bits = 0;
        struct input_event evt = {};
        uint8_t devname[BUF_SIZE] = {0};

        get_evt_types(evtfd[i], &evt_type_bits);
        get_devname(evtfd[i], devname);
        get_evt(evtfd[i], &evt);

        // Mouse device writes its relative movements on the device file
        // not cursor's absolute positions. In that case, there are two
        // problems.
        // 1) If the initial mouse cursor's positions aren't identical
        //    between recording and replaying, recording's mouse movement
        //    and replaying's it will not be identical.
        // 2) If the mouse speed settings aren't identical between
        //    recording and replaying, recording's mouse movement and
        //    replaying's it will not be identical.
        // Therefore, in the case of the only mouse move event, this
        // program records the mouse cursor's absolute positions by Xlib
        // functions.
        if (is_mouse_move_event(evt)) {
          get_absolute_mouse_position(display, &mouse_x, &mouse_y);

          printf(MOUSE_MOVE_OUT_FORMAT, devname, evt_type_bits, evt.time.tv_sec,
                 evt.time.tv_usec, evt.type, evt.code, evt.value, mouse_x,
                 mouse_y);
        } else {
          printf(OUT_FORMAT, devname, evt_type_bits, evt.time.tv_sec,
                 evt.time.tv_usec, evt.type, evt.code, evt.value);
        }
      }
    }
  }

  XCloseDisplay(display);
  close_all_device_files(evtfd, ndev);
  fflush(stdout);
}
