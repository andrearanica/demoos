#ifndef __FAT_TYPES_H
#define __FAT_TYPES_H

/**
 * fat_types.h contains the definition of the constants and structures which will be used to handle
 * the filesystem functions
 */

// This isn't the right thing to do, but the submodule doesn't provide a separate header for enums
#include "../libs/fat32/fat.h"

#include <stdint.h>
#include <stdbool.h>

#define FAT_MAX_NAME_SIZE 64
#define FAT_MAX_PATH_SIZE 128

// Struct which contains the data about a resource in the FAT filesystem. This struct is returned 
// from the syscalls which gets the FAT resources data (like syscall_get_next_entry)
typedef struct {
    uint32_t size;
    char name[FAT_MAX_NAME_SIZE];
    bool is_dir;
} FatEntryInfo;

#endif
