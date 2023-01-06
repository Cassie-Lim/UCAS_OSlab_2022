#include <asm.h>
#include <csr.h>

ENTRY(setup_exception)

  /* TODO: [p2-task3] save exception_handler_entry into STVEC */
  la t0, exception_handler_entry
  csrw stvec, t0
  /* TODO: [p2-task4] enable interrupts globally */
  li t0, SR_SIE
  csrw sstatus, t0
  // li t0, SIE_SSIE | SIE_STIE | SIE_SEIE
  // csrw sie, t0
  ret
ENDPROC(setup_exception)
