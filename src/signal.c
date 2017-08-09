#include "signal.h"
#include "util.h"
#include <signal.h>
#include <stdlib.h>
#include <string.h>

void set_sig_handler(void (*sig_handler)(int32_t)) {
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));

  sa.sa_handler = sig_handler;
  sa.sa_flags = SA_RESTART;
  sigfillset(&sa.sa_mask);

  if (sigaction(SIGINT, &sa, NULL) != 0) {
    die("Can't set SIGINT handler");
  }
}
