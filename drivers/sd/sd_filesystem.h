#ifndef __SD_FILESYSTEM_H
#define __SD_FILESYSTEM_H

#define SD_FILESYSTEM_INIT_OK      0
#define SD_FILESYSTEM_INIT_ERROR   1

#include "../../libs/fat32/fat.h"

static Fat g_fat;
static Dir dir;

int sd_filesystem_init();

#endif
