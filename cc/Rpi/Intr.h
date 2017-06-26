// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Rpi_Intr_h_
#define _Rpi_Intr_h_

// --------------------------------------------------------------------
// BCM2835 ARM Peripherals: Chapter 7: Interrupts
//
//   64 x GPU Interrupt Sources (in two 32-bit banks)
//    8 x ARM Interrupt Sources (in one 32-bit bank )
//
// This implementation does neither cover pending interrupts nor FIQ
// control. It's basically to enable/disable interrupts that are used
// by (an intering) Raspbian
//
// On Pi-2 (Raspbian) it appears that IRQ 52 is enabled to catch 
//   event detect interrupts. This may freeze the system. So it 
//   may be a good idea to disable them.
//
// --------------------------------------------------------------------

#include "Peripheral.h"

namespace Rpi { struct Intr
{
  Intr(Peripheral *p) : page(p->page(Peripheral::PNo::make<0x00b>())) {}
  
  struct Vector
  {
    uint32_t lo ; // GPU:  0..31
    uint32_t hi ; // GPU: 32..63
    uint8_t base ; // ARM: 0..7
    Vector(uint32_t lo,uint32_t hi,uint8_t base) : lo(lo),hi(hi),base(base) {}
  } ;

  Vector status() const
  {
    return Vector(page->at<0x210/4>(),
		  page->at<0x214/4>(),
		  static_cast<uint8_t>(page->at<0x218/4>())) ;
  }
    
  void enable(Vector const &v)
  {
    page->at<0x210/4>() = v.lo ;
    page->at<0x214/4>() = v.hi ;
    page->at<0x218/4>() = v.base ;
  }
    
  void disable(Vector const &v)
  {
    page->at<0x21c/4>() = v.lo ;
    page->at<0x220/4>() = v.hi ;
    page->at<0x224/4>() = v.base ;
  }
    
private: std::shared_ptr<Page> page ;
  
} ; }

#endif // _Rpi_Intr_h_
