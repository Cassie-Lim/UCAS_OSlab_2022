/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2012 Regents of the University of California
 */

#include <asm.h>
#include <csr.h>

// #define KERNEL_STACK		0x50500000
// #define PAGE_SIZE 4096
#define M_KERNEL_SP 0x50501000
#define S_KERNEL_SP 0x50502000

.section ".entry_function","ax"
ENTRY(_start)
  // 主从核都需屏蔽所有中断（从核已被唤醒，无需开软件中断）
  csrw CSR_SIE, zero
  csrw CSR_SIP, zero

  csrr t0, CSR_MHARTID
  bnez t0, s_start

// 清空bss段由主核来做
  /* TODO: [p1-task2] clear BSS for flat non-ELF images */
  la x5, __bss_start         // bss起始地址
  la x6, __BSS_END__         // bss终止地址
do_clear:  
  sw  x0, (x5)          // store 0 to x5
  add x5, x5, 0x04      // point to next word
  ble x5, x6, do_clear

  /* TODO: [p1-task2] setup C environment */
  la tp, m_pid0_pcb
  la sp, M_KERNEL_SP   // set stack pointer to the given addr
  call main
// 从核只需设置初始tp和sp
s_start:
  la tp, s_pid0_pcb
  la sp, S_KERNEL_SP
  call main
loop:
  wfi
  j loop

END(_start)
