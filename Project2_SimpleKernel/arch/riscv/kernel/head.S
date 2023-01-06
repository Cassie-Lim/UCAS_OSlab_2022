/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2012 Regents of the University of California
 */

#include <asm.h>
#include <csr.h>

#define KERNEL_STACK		0x50500000

.section ".entry_function","ax"
ENTRY(_start)
  /* Mask all interrupts */
  csrw CSR_SIE, zero
  csrw CSR_SIP, zero

  /* TODO: [p1-task2] clear BSS for flat non-ELF images */
  la x5, __bss_start         // bss起始地址
  la x6, __BSS_END__         // bss终止地址
do_clear:  
  sw  x0, (x5)          // store 0 to x5
  add x5, x5, 0x04      // point to next word
  ble x5, x6, do_clear

  /* TODO: [p1-task2] setup C environment */
  la sp, KERNEL_STACK   // set stack pointer to the given addr
  call main

loop:
  wfi
  j loop

END(_start)
