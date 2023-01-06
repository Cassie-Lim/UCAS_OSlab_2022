#include <asm.h>

.section ".entry_function","ax"
ENTRY(_start)

    /* TODO: [p1-task3] setup C runtime environment for the user program */
    addi sp, sp, -16 
    sd s0, 8(sp)            // store frame pointer in stack
    sd ra, (sp)             // store return address in stack
    addi s0, sp, 16          // frame pointer points to original sp
    
    la x5, __bss_start         // bss起始地址
    la x6, __BSS_END__         // bss终止地址
do_clear:  
    sw  x0, (x5)          // store 0 to x5
    add x5, x5, 0x04      // point to next word
    ble x5, x6, do_clear

    /* TODO: [p1-task3] enter main function */
    call main

    /* TODO: [p1-task3] finish task and return to the kernel */
    /* NOTE: You need to replace this with new mechanism in p3-task2! */
    // ld ra, (sp)
    // ld s0, 8(sp)
    // addi sp, sp, 16
    // jr ra

    /* TODO: [P3] call sys_exit */
    li a7, 1    # SYS_EXIT
    ecall

// while(1) loop, unreachable here
loop:
    wfi
    j loop

END(_start)
