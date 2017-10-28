// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Rpi_GpioOld_h_
#define _Rpi_GpioOld_h_

// --------------------------------------------------------------------
// BCM2835 ARM Peripherals: Chapter 6: General Purpose I/O (GPIO)
// --------------------------------------------------------------------

#include "Peripheral.h"
#include "Pin.h"
#include <Neat/Numerator.h>

namespace Rpi { struct GpioOld 
{
  GpioOld(Peripheral *p) : page(p->page(Peripheral::PNo::make<0x200>())) {}

  // ----[ Event ]---------------------------------------------------

  enum class Event : unsigned { Rise=0,Fall=1,High=2,Low=3,AsyncRise=4,AsyncFall=5 } ;
    
  using EventN = Neat::Numerator<Event,Event::AsyncFall> ;
    
  uint32_t enable(uint32_t set,Event e,bool on)
  {
    // returns the previous set
    auto i = 0x4c + EventN(e).n() * 0xc ;
    // 0x4c : GP-R -EN0 
    // 0x58 : GP-F -EN0 
    // 0x64 : GP-H -EN0
    // 0x70 : GP-L -EN0 
    // 0x7c : GP-AR-EN0 
    // 0x88 : GP-AF-EN0
    return rmw(page->at(Page::Index::make(i/4)),set,on) ; 
  }

  bool enable(Pin pin,Event e,bool on)
  {
    return 1& (enable(1u<<pin.value(),e,on) >> pin.value()) ;
  }
    
  uint32_t getEvents() const
  {
    return page->at<0x40/4>() ; /* GPEDS0 */
  }
    
  bool getEvent(Pin pin) const
  {
    return 0 != ((1u<<pin.value()) & getEvents()) ;
  }

  void reset(uint32_t set)
  {
    page->at<0x40/4>() = set ; /* GPEDS0 */
  }
    
  void reset(Pin pin)
  {
    reset(1u<<pin.value()) ;
  }

private:

  std::shared_ptr<Page> page ;

  static uint32_t rmw(uint32_t volatile &r,uint32_t set,bool on) ;
  
} ; }

#endif // _Rpi_GpioOld_h_
