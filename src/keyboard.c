#include "keyboard.h"
#include "event.h"
#include "signal.h"
#include "util.h"
#include <errno.h>
#include <fcntl.h>
#include <linux/uinput.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static void get_key_evt_codes(int32_t fd, uint64_t *p_bitsring) {
  int32_t ret = ioctl(fd, EVIOCGBIT(EV_KEY, KEY_MAX), p_bitsring);
  if (ret < 0) {
    fprintf(stderr, "EVIOCGBIT ioctl() failed: %s\n", strerror(errno));
  }
}

static int32_t get_physical_keyboard_fd(int32_t evtfd[], uint32_t ndev) {
  for (uint32_t i = 0; i < ndev; i++) {
    uint64_t evt_type_bits = 0;
    uint64_t evt_code_bits[KEY_MAX / BITS_PER_LONG];
    memset(&evt_code_bits, 0, sizeof(evt_code_bits));

    get_evt_types(evtfd[i], &evt_type_bits);
    if (!is_bit_set(&evt_type_bits, EV_KEY))
      continue;

    get_key_evt_codes(evtfd[i], evt_code_bits);
    if (is_bit_set(evt_code_bits, KEY_C)) {
      if (is_bit_set(evt_code_bits, KEY_LEFTCTRL) ||
          is_bit_set(evt_code_bits, KEY_RIGHTCTRL)) {
        return evtfd[i];
      }
    }
  }

  fprintf(stderr, "Can't find the physical keyboard on this computer\n");
  return NOT_FOUND;
}

static void release_key(int32_t fd, uint16_t key) {
  write_event(fd, EV_MSC, MSC_SCAN, key);
  usleep(1);
  write_event(fd, EV_KEY, key, 0);
  usleep(1);
  write_event(fd, EV_SYN, SYN_REPORT, 0);
  usleep(100);
}

void release_ctrl_c_key(int32_t evtfd[], uint32_t ndev) {
  int32_t fd = get_physical_keyboard_fd(evtfd, ndev);
  if (fd == NOT_FOUND) {
    return;
  }

  release_key(fd, KEY_LEFTCTRL);
  release_key(fd, KEY_RIGHTCTRL);
  release_key(fd, KEY_C);
}
