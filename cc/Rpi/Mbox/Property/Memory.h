// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Rpi_Mbox_Property_Memory_h_
#define _Rpi_Mbox_Property_Memory_h_

//---------------------------------------------------------------------
// memory modes:
//
// discardable = can be resized to 0 at any time. use for cached data.

// coherent : direct
// 0:0 (normal)   = normal allocating. don't use from ARM. (bus address 0x0)
// 0:1 (direct)   = L2-uncached.                           (bus address 0xC)
// 1:0 (coherent) = L2-uncached but coherent.              (bus address 0x8)
// 1:1 (cached)   = L2-cached                              (bus address 0x4)
//
// zero      = initialise buffer to all zeros.
// init      = don't initialise (default is initialise to all ones).
// permaLock = likely to be locked for long periods of time.
//
// see github.com/raspberrypi/firmware/wiki/Mailbox-property-interface
//---------------------------------------------------------------------

#include "../Interface.h"

namespace Rpi { namespace Mbox { namespace Property
{
  struct Memory
  {
    struct Mode
    {
      unsigned discardable : 1 ;
      unsigned direct      : 1 ;
      unsigned coherent    : 1 ;
      unsigned zero        : 1 ;
      unsigned noInit      : 1 ;
      unsigned permaLock   : 1 ;
      static Mode deserialize(uint32_t) ; uint32_t serialize() const ;
    } ;

    static uint32_t allocate(Interface*,uint32_t size,uint32_t alignment,Mode mode) ;
    // ...returns a new handle (zero on problems)

    static uint32_t lock(Interface*,uint32_t handle) ;
    // ...returns a (VideoCore) bus address
  
    static bool unlock(Interface*,uint32_t handle) ;
    // ...returns success (never false as it appears)
  
    static bool release(Interface*,uint32_t handle) ; 
    // ...returns success (never false as it appears)
  } ;

} /* Property */ } /* Mbox */ } /* Rpi */

#endif // _Rpi_Mbox_Property_Memory_h_
