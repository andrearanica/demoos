#ifndef __FAT_TYPES_H
#define __FAT_TYPES_H

// This isn't the right thing to do, but the submodule doesn't provide a separate header for enums
#include "../libs/fat32/fat.h"

#include <stdint.h>
#include <stdbool.h>

#define FAT_MAX_NAME_SIZE 64

typedef struct {
    uint32_t size;
    char name[FAT_MAX_NAME_SIZE];
    bool is_dir;
} FatEntryInfo;

#endif
