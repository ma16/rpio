// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _arm_h_
#define _arm_h_

// --------------------------------------------------------------------
// ARM1176JZF-S Technical Reference Manual: 3.2.22. c7, Cache operations
// --Data Synchronization Barrier (DSB)
// --Data Memory Barrier (DMB)
// --Clean Data Cache Range (CDCL)
// http://infocenter.arm.com/help/topic/com.arm.doc.ddi0301h/Babhejba.html
// http://www.ethernut.de/en/documents/arm-inline-asm.html
// --------------------------------------------------------------------

namespace arm
{
  inline void dsb()
  {
    // This instruction acts as an explicit memory barrier. This
    // instruction completes when all explicit memory transactions
    // occurring in program order before this instruction are completed.
    // No instructions occurring in program order after this instruction
    // are executed until this instruction completes. Therefore, no
    // explicit memory transactions occurring in program order after
    // this instruction are started until this instruction completes. 
    // It can be used instead of Strongly Ordered memory when the timing
    // of specific stores to the memory system has to be controlled. For
    // example, when a store to an interrupt acknowledge location must
    // be completed before interrupts are enabled.
    __asm__ __volatile__ ("mcr p15,#0,%0,c7,c10,#4" : : "r" (0) : "memory") ;
  }

  inline void dmb()
  {
    // The purpose of the Data Memory Barrier operation is to ensure
    // that all outstanding explicit memory transactions complete before
    // any following explicit memory transactions begin. This ensures
    // that data in memory is up to date before any memory transaction
    // that depends on it. 
    __asm__ __volatile__ ("mcr p15,#0,%0,c7,c10,#5" : : "r" (0) : "memory") ;
  }

  inline void cdcl(void const *begin,void const *end)
  {
    // The Start Address is the first VA of the block transfer. It uses
    // the VA bits [31:5]. The End Address is the VA where the block
    // transfer stops. This address is at the start of the line
    // containing the last address to be handled by the block transfer.
    // It uses the VA bits [31:5].
    __asm__ __volatile__ ("mcrr p15,#0,%0,%1,c12" : : "r"(end),"r"(begin) : "memory" ) ;
  }

  inline void cache_flush(void const *begin,void const *end)
  {
    // Raspbian provides a cashflush(2) man page:
    // "cacheflush - flush contents of instruction and/or data cache
    //   #include <asm/cachectl.h>
    //   int cacheflush(char *addr, int nbytes, int cache);"
    // However, such a syscall doesn't appear to exist.
    //
    // Instead there is the cacheflush syscall(0xf0002) that is
    //   do_cache_op(unsigned long start, unsigned long end) 
    // github.com/raspberrypi/linux/blob/rpi-4.4.y/arch/arm/kernel/traps.c
    __builtin___clear_cache((char*)begin,(char*)end) ;
  }
}
  
// https://www.raspberrypi.org/forums/viewtopic.php?f=63&t=1410
// http://stackoverflow.com/questions/18896812/flush-cache-to-dram#18897339

#endif // _arm_h_
