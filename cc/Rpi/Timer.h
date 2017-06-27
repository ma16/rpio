// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Rpi_Timer_h_
#define _Rpi_Timer_h_

// --------------------------------------------------------------------
// BCM2835 ARM Peripherals: chapter 12: System Timer
// --------------------------------------------------------------------

#include "Bus/Address.h"
#include "Peripheral.h"

namespace Rpi { struct Timer
{
  Timer(Peripheral *p) : page(p->page(Peripheral::PNo::make<0x003>())) {}
  
  uint64_t clock() const ;
  uint32_t cLo() const { return page->at<0x4/4>() ; }
  uint32_t cHi() const { return page->at<0x8/4>() ; }

  static constexpr auto cLoAddr = Bus::Address(0x7e003004) ;
    
private:

  std::shared_ptr<Page> page ;
  
} ; }

#endif // _Rpi_Timer_h_