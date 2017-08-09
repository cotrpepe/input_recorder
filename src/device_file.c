#include "device_file.h"
#include "util.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int is_device_file(const struct dirent *dir) {
  return strncmp("event", dir->d_name, 5) == 0;
}

uint32_t enumerate_all_device_files(struct dirent **namelist[]) {
  uint32_t ndev = scandir("/dev/input", namelist, is_device_file, alphasort);
  if (ndev <= 0) {
    fprintf(stderr, "Can't open '/dev/input': %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  return ndev;
}

void open_all_device_files(int32_t evtfd[], uint32_t ndev,
                           struct dirent *namelist[], int32_t mode) {
  char evname[BUF_SIZE] = {0};

  for (uint32_t i = 0; i < ndev; i++) {
    snprintf(evname, sizeof(evname), "/dev/input/%s", namelist[i]->d_name);
    evtfd[i] = open(evname, mode);
    if (evtfd[i] < 0) {
      fprintf(stderr, "Can't open '%s': %s\n", evname, strerror(errno));
      continue;
    }
  }
}

void close_all_device_files(int32_t evtfd[], uint32_t ndev) {
  for (uint32_t i = 0; i < ndev; i++) {
    close(evtfd[i]);
  }
}
