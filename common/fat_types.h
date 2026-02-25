#ifndef __FAT_TYPES_H
#define __FAT_TYPES_H

// This isn't the right thing to do, but the submodule doesn't provide a separate header for enums
#include "../libs/fat32/fat.h"

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint32_t size;
    char name[64];
    bool is_dir;
} FatEntryInfo;

#endif
