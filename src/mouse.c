#include "mouse.h"
#include "util.h"

bool is_mouse_move_event(struct input_event evt) {
  if (evt.type == EV_REL) {
    if (evt.code == REL_X || evt.code == REL_Y) {
      return true;
    }
  }
  return false;
}
