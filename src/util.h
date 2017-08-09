#pragma once

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>

#define OUT_DEVNAME "%s"
#define IN_DEVNAME "%[^\t]"
#define ALL_EVT_TYPES "%" PRIu64
#define EVT_TIME "%ld.%06ld"
#define EVT_TYPE "%hu"
#define EVT_CODE "%hu"
#define EVT_VALUE "%d"

#define BASE_FORMAT                                                            \
  ALL_EVT_TYPES                                                                \
  "\t" EVT_TIME "\t" EVT_TYPE "\t" EVT_CODE "\t" EVT_VALUE

#define OUT_FORMAT OUT_DEVNAME "\t" BASE_FORMAT "\n"
#define IN_FORMAT IN_DEVNAME "\t" BASE_FORMAT "\n"

#define MOUSE_X "%d"
#define MOUSE_Y "%d"
#define MOUSE_MOVE_OUT_FORMAT                                                  \
  OUT_DEVNAME                                                                  \
  "\t" BASE_FORMAT "\t" MOUSE_X "\t" MOUSE_Y "\n"
#define MOUSE_MOVE_IN_FORMAT                                                   \
  IN_DEVNAME                                                                   \
  "\t" BASE_FORMAT "\t" MOUSE_X "\t" MOUSE_Y "\n"

#define MAX_NUM_DEV_FILE 100
#define BUF_SIZE 256

extern const int32_t NOT_EOF;
extern const int32_t NOT_FOUND;
extern const int32_t CORRECT_NUM_TAB_PER_LINE;
extern const int32_t BITS_PER_LONG;

void die(char *format, ...);
bool is_bit_set(uint64_t *bitstring, int32_t digit);
