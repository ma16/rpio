// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Rpi_Bus_Coherency_h_
#define _Rpi_Bus_Coherency_h_

// --------------------------------------------------------------------
// VideoCore (GPU) Memory Coherency: BCM2835 ARM Peripherals page 5
//
// github.com/raspberrypi/firmware/wiki/Accessing-mailboxes:
//
// For example, if you have created a framebuffer description structure
// in memory (without having enabled the ARM MMU) at 0x00010000 and you
// have not changed config.txt to disable the L2 cache, to send it to
// channel 1 you would send 0x40010001 (0x40000000 | 0x00010000 | 0x1)
// to the mailbox. Your structure would be updated to include a
// framebuffer address starting from 0x40000000 (e.g. 0x4D385000) and
// you would write to it using the corresponding ARM physical address
// (e.g. 0x0D385000).
// --------------------------------------------------------------------

#include <Neat/Enum.h>

namespace Rpi { namespace Bus { struct Coherency
{
  static constexpr Coherency x0() { return (0x0u<<28) ; } // L1+L2-cached
  static constexpr Coherency x4() { return (0x4u<<28) ; } // L2-cached (non-allocating)
  static constexpr Coherency x8() { return (0x8u<<28) ; } // L2-cached
  static constexpr Coherency xC() { return (0xCu<<28) ; } // direct uncached

  using Enum = Neat::Enum<uint32_t,3> ; // [todo] uint2_t
  
  constexpr Coherency(Enum e) : u32(e.value() << 30) {}

  uint32_t prefix() const { return u32 ; }
  
private:
  
  uint32_t u32 ; constexpr Coherency(uint32_t u32) : u32(u32) {}
  
} ; } }

#endif // _Rpi_Bus_Coherency_h_
