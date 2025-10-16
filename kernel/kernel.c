#include <stddef.h>
#include <stdint.h>
#include "../drivers/uart.h"
#include "../drivers/framebuffer.h"

void kernel_main(uint64_t dtb_ptr32, uint64_t x1, uint64_t x2, uint64_t x3)
{
  uart_init();
  uart_puts("Hello, kernel world!\r\n");

  int ok = framebuffer_init();
  if (ok) {
    framebuffer_print(0, 0, "Hello, framebuffer world!\n");
  } else {
    uart_puts("Initialization of framebuffer failed.\r\n");
  }
  
  while (1)
    uart_putc(uart_getc());
}
