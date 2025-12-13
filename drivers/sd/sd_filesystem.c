#include "sd_filesystem.h"
#include "sd.h"
#include "../../libs/allocator.h"
#include "../../libs/fat32/fat.h"
#include "../../drivers/uart/uart.h"
#include "../../libs/syscalls.h"

bool sd_read_adapter(uint8_t* buffer, uint32_t sector) {
    int res = sd_readblock(sector, buffer, 1);
    return res != 0;
}

bool sd_write_adapter(const uint8_t* buffer, uint32_t sector) {
    int res = sd_writeblock(buffer, sector, 1);
    return res != 0;
}

int sd_filesystem_init() {
    int sd_result = sd_init();
    if (sd_result == SD_OK) {
        uart_puts("[DEBUG] SD init successful.\n");
    } else {
        uart_puts("[DEBUG] SD init error.\n");
    }

    DiskOps disk_operations = {sd_read_adapter, sd_write_adapter};

    int probe_error = fat_probe(&disk_operations, 0);
    if (!probe_error) {
        uart_puts("[DEBUG] Filesystem probed.\n");
    } else {
        uart_puts("[ERROR] Cannot probe filesystem.\n");
        return SD_FILESYSTEM_INIT_ERROR;
    }

    int mount_error = fat_mount(&disk_operations, 0, &g_fat, "mnt");
    if (mount_error) {
        uart_puts("[ERROR] Cannot mount filesystem.\n");
        return SD_FILESYSTEM_INIT_ERROR;
    } else {
        uart_puts("[DEBUG] Filesystem mounted.\n");
    }

    return SD_FILESYSTEM_INIT_OK;
}
