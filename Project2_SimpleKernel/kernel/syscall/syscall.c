#include <sys/syscall.h>

long (*syscall[NUM_SYSCALLS])();

void handle_syscall(regs_context_t *regs, uint64_t interrupt, uint64_t cause)
{
    /* TODO: [p2-task3] handle syscall exception */
    /**
     * HINT: call syscall function like syscall[fn](arg0, arg1, arg2),
     * and pay attention to the return value and sepc
     */
    // pass a0, a1, a2 (get index in regs.h), store return value in a0
    // regs->regs[10] = syscall[cause & 0x7FFFFFFF](regs->regs[10], regs->regs[11], regs->regs[12]); 
    regs->regs[10] = syscall[regs->regs[17]](regs->regs[10], regs->regs[11], regs->regs[12]); 
    // for syscall, return sepc should increase
    regs->sepc += 4;
}
