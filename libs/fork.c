#include "fork.h"
#include "../drivers/irq/entry.h"
#include "../drivers/uart/uart.h"
#include "allocator.h"
#include "scheduler.h"

// Creates a new process that executes the given function
int copy_process(unsigned long clone_flags, unsigned long function,
         unsigned long argument, unsigned long stack) {
  // I disable the preempt to avoid this function to be interrupted
  preempt_disable();

  // I ask the allocator a free page for the new PCB
  struct PCB *new_process;
  new_process = (struct PCB *)allocate_kernel_page();
  if (!new_process) {
    return 1;
  }

  struct pt_regs *child_registers = task_pt_regs(new_process);
  memzero((unsigned long)child_registers, sizeof(struct pt_regs));
  memzero((unsigned long)&new_process->cpu_context, sizeof(struct cpu_context));
  memzero((unsigned long)&new_process->files, sizeof(new_process->files));

  if (clone_flags & PF_KTHREAD) {
    // If we are running a kernel thread, we only need to specify the function
    new_process->cpu_context.x19 = function;
    new_process->cpu_context.x20 = argument;
  } else {
    // If we are running a user thread, I need to allocate a new stack
    struct pt_regs *current_registers = task_pt_regs(current_process);
    *child_registers = *current_registers;
    child_registers->registers[0] = 0;
    child_registers->sp = stack + PAGE_SIZE;
    new_process->stack = stack;
  }

  int process_id = n_processes++;

  new_process->flags = clone_flags;
  new_process->priority = current_process->priority;
  new_process->state = PROCESS_RUNNING;
  new_process->counter = current_process->priority;
  new_process->preempt_disabled = 1;
  new_process->pid = process_id;

  // x19 and x20 will be used in the assembly to call the function
  new_process->cpu_context.pc = (unsigned long)ret_from_fork;
  new_process->cpu_context.sp = (unsigned long)child_registers;

  processes[process_id] = new_process;

  preempt_enable();

  return process_id;
}

int move_to_user_mode(unsigned long start, unsigned long size, unsigned long pc) {
  struct pt_regs *regs = task_pt_regs(current_process);
  // memzero((unsigned long)regs, sizeof(*regs));

  regs->pc = pc;
  regs->pstate = PSR_MODE_EL0t;
  regs->sp = PAGE_SIZE;
  unsigned long code_page_virtual_address = 15 * PAGE_SIZE;

  unsigned long page_virtual_address = 0;
  for (int i = 0; i < 16; i++) {
      allocate_user_page(current_process, page_virtual_address);
      page_virtual_address += PAGE_SIZE;
  }

  unsigned long code_page_physical_address = current_process->mm.user_pages[15].physical_address;
  if (code_page_physical_address == 0) {
      return -1;
  }

  uart_puts("Copio il codice utente ("); uart_hex(start); uart_puts(", di dimensione "); uart_hex(size); uart_puts(") nella pagina codice\n");
  memcpy((void*)code_page_virtual_address, (void*)start, size);

  set_pgd(current_process->mm.pgd);

  return 0;
}

struct pt_regs *task_pt_regs(struct PCB *process) {
  unsigned long p =
      (unsigned long)process + THREAD_SIZE - sizeof(struct pt_regs);
  return (struct pt_regs *)p;
}
