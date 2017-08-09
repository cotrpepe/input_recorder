#pragma once

#include <stdint.h>

void get_devname(int32_t evtfd, uint8_t *devname);
void get_evt_types(int32_t fd, uint64_t *p_evt_type_bits);
void write_event(int32_t fd, uint16_t type, uint16_t code, int32_t value);
