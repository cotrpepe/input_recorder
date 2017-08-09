#pragma once

#include <dirent.h>
#include <stdint.h>

uint32_t enumerate_all_device_files(struct dirent **namelist[]);
void open_all_device_files(int32_t evtfd[], uint32_t ndev,
                           struct dirent *namelist[], int32_t mode);
void close_all_device_files(int32_t evtfd[], uint32_t ndev);
