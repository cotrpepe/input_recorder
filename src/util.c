#include "util.h"
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const int32_t NOT_EOF = 1;
const int32_t NOT_FOUND = -1;
const int32_t CORRECT_NUM_TAB_PER_LINE = 5;
const int32_t BITS_PER_LONG = sizeof(long) * 8;

void die(char *format, ...) {
  va_list args;

  va_start(args, format);
  vfprintf(stderr, format, args);
  fprintf(stderr, ": %s\n", strerror(errno));
  va_end(args);

  exit(EXIT_FAILURE);
}

bool is_bit_set(uint64_t *bitstring, int32_t digit) {
  int32_t array_index = digit / BITS_PER_LONG;
  int32_t digit_in_long = digit % BITS_PER_LONG;
  return bitstring[array_index] & (1 << digit_in_long);
}
