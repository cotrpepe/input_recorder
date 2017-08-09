#include "event.h"
#include "util.h"
#include <errno.h>
#include <linux/input.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void get_devname(int32_t evtfd, uint8_t *devname) {
  int32_t ret =
      ioctl(evtfd, EVIOCGNAME(sizeof(char) * BUF_SIZE), (char *)devname);
  if (ret < 0) {
    fprintf(stderr, "EVIOCGNAME ioctl() failed: %s\n", strerror(errno));
  }
}

void get_evt_types(int32_t fd, uint64_t *p_evt_type_bits) {
  int32_t ret = ioctl(fd, EVIOCGBIT(0, EV_MAX), p_evt_type_bits);
  if (ret < 0) {
    fprintf(stderr, "EVIOCGBIT ioctl() failed: %s\n", strerror(errno));
  }
}

void write_event(int32_t fd, uint16_t type, uint16_t code, int32_t value) {
  struct input_event evt = {};

  gettimeofday(&evt.time, NULL);
  evt.type = type;
  evt.code = code;
  evt.value = value;

  if (write(fd, &evt, sizeof(evt)) < 0) {
    die("Can't write on /dev/input");
  }
}
