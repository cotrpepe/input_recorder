#pragma once

#include <linux/input.h>
#include <stdbool.h>

bool is_mouse_move_event(struct input_event evt);
