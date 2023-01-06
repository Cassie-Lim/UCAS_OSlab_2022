#include <asm.h>
#include <csr.h>

ENTRY(setup_exception)

  /* TODO: [p2-task3] save exception_handler_entry into STVEC */
  la t0, exception_handler_entry
  csrw stvec, t0

  // 对于主核：其sip清零后不会再收到软件中断
  // 对于从核，其收到软件中断后sip已经为0，此步相当于空操作
  csrw sip, zero
  
  /* TODO: [p2-task4] enable interrupts globally */
  li t0, SR_SIE
  csrs sstatus, t0
  // li t0, SIE_SSIE | SIE_STIE | SIE_SEIE
  // csrs sie, t0
  ret
ENDPROC(setup_exception)
