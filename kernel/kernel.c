#include <stddef.h>
#include <stdint.h>
#include "../libs/scheduler.h"
#include "../libs/fork.h"
#include "../libs/utils.h"
#include "../libs/syscalls.h"
#include "../drivers/uart/uart.h"
#include "../drivers/timer/timer.h"
#include "../drivers/irq/controller.h"
#include "../drivers/sd/sd.h"
#include "../drivers/sd/sd_filesystem.h"

void kernel_process();
void user_process();
void user_process1(char*);

void kernel_main(uint64_t dtb_ptr32, uint64_t x1, uint64_t x2, uint64_t x3)
{
    uart_init();
    uart_puts("Hello, kernel world!\r\n");

    irq_vector_init();
    timer_init();
    enable_interrupt_controller();
    enable_irq();

    int res = fork(PF_KTHREAD, (unsigned long)&kernel_process, 0, 0);
}

void kernel_process() {
    uart_puts("Kernel process started.\n");

    int fs_ok = sd_filesystem_init();
    if (fs_ok == SD_FILESYSTEM_INIT_OK) {
        uart_puts("[DEBUG] SD filesystem init successful.\n");
    } else {
        uart_puts("[DEBUG] SD filesystem init error.\n");
    }

    int error = move_to_user_mode((unsigned long)&user_process);
    if (error < 0) {
        uart_puts("[ERROR] Cannot move process from kernel mode to user mode\n");
    }
}

void user_process() {
    call_syscall_write("[DEBUG] User process started\n");

    int err = call_syscall_create_dir("temp");
    if (err) {
        call_syscall_write("[ERROR] Cannot create 'temp' dir.\n");
    }

    call_syscall_exit();
}

void user_process1(char* array) {
    char buffer[2] = {0};
    while (1) {
        for (int i = 0; i < 5; i++) {
            buffer[0] = array[i];
            call_syscall_write(buffer);
            delay(100000);
        }
    }
}
