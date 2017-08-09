#include "mouse.h"
#include "record.h"
#include "replay.h"
#include "util.h"
#include <stdbool.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef enum { RECORD, REPLAY } MODE;

static void print_usage() {
  printf("[Usage]                                           \n"
         "Record your operation:                            \n"
         "  sudo input_recorder -r > [file name]            \n\n"

         "Record your operation via SSH                     \n"
         "  sudo DISPLAY=:0 input_recorder -r > [file name] \n\n"

         "Replay your operation:                     　　　　\n"
         "  sudo input_recorder -p [file name]              \n\n"

         "Replay your operation via SSH:             　　　　\n"
         "  sudo DISPLAY=:0 input_recorder -p [file name]   \n\n"

         "Replay your operation with verbose logging 　　　　\n"
         "  sudo input_recorder -v -p [file name]           \n");
}

MODE parse_options(int32_t argc, char *argv[], char **p_rec_file,
                   bool *p_is_verbose) {
  int result = -1;
  MODE mode;

  while ((result = getopt(argc, argv, "rvp:")) != -1) {
    switch (result) {
    case 'r':
      mode = RECORD;
      break;
    case 'p':
      mode = REPLAY;
      *p_rec_file = optarg;
      break;
    case 'v':
      *p_is_verbose = true;
      break;
    default:
      print_usage();
      return EXIT_SUCCESS;
    }
  }

  return mode;
}

int main(int32_t argc, char *argv[]) {
  char *rec_file = NULL;
  bool is_verbose = false;

  MODE mode = parse_options(argc, argv, &rec_file, &is_verbose);

  if (mode == RECORD) {
    record();
  } else if (mode == REPLAY) {
    replay(rec_file, is_verbose);
  } else {
    print_usage();
  }

  return EXIT_SUCCESS;
}
