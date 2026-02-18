#include "sd_filesystem.h"
#include "../../drivers/uart/uart.h"
#include "../../libs/fat32/fat.h"
#include "sd.h"

Fat g_fat;

bool sd_read_adapter(uint8_t *buffer, uint32_t sector) {
  int res = sd_readblock(sector, buffer, 1);
  return res != 0;
}

bool sd_write_adapter(const uint8_t *buffer, uint32_t sector) {
  int res = sd_writeblock((unsigned char*)buffer, sector, 1);
  return res != 0;
}

int sd_filesystem_init() {
  int sd_result = sd_init();
  if (sd_result != SD_OK) {
    return SD_FILESYSTEM_INIT_ERROR;
  }

  DiskOps disk_operations = {sd_read_adapter, sd_write_adapter};

  int probe_error = fat_probe(&disk_operations, 0);
  if (probe_error) {
    uart_puts("[ERROR] Cannot probe filesystem.\n");
    return SD_FILESYSTEM_INIT_ERROR;
  }

  int mount_error = fat_mount(&disk_operations, 0, &g_fat, "mnt");
  if (mount_error) {
    return SD_FILESYSTEM_INIT_ERROR;
  }

  return SD_FILESYSTEM_INIT_OK;
}
