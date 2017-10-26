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
  
  // ----[ Mode ]----------------------------------------------------

  enum class Mode : unsigned char { In=0,Out=1,Alt5=2,Alt4=3,Alt0=4,Alt1=5,Alt2=6,Alt3=7 } ;

  using ModeN = Neat::Numerator<Mode,Mode::Alt3> ;
    
  Mode getMode(Pin pin) const ;
    
  void setMode(uint32_t set,Mode mode) ;
  void setMode(Pin      pin,Mode mode) ;

  // ----[ Level ]---------------------------------------------------
    
  uint32_t getLevels() const
  {
    return page->at<0x34/4>() ; /* GPLEV0 */
  } 

  bool getLevel(Pin pin) const
  {
    return 0 != ((1u<<pin.value()) & getLevels()) ;
  }

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

  // ----[ Pull ]----------------------------------------------------

  enum class Pull : unsigned char { Off=0,Down=1,Up=2 } ;

  using PullN = Neat::Numerator<Pull,Pull::Up> ;
    
  void setPull(uint32_t set,Pull how) ;
  void setPull(Pin      pin,Pull how) ;

  // ----[ Output ]--------------------------------------------------

  enum class Output : unsigned char { Hi=0,Lo=1 } ;

  using OutputN = Neat::Numerator<Output,Output::Lo> ;

  template<Output O> void setOutput(uint32_t set)
  {
    page->at<(0x1c+OutputN(O).n()*0xc)/4>() = set ; // 0x1c:GPSET0 ; 0x28:GPCLR0
  }
    
  template<Output O> void setOutput(Pin pin)
  {
    setOutput<O>(1u << pin.value()) ;
  }
    
  void setOutput(uint32_t set,Output o)
  {
    switch (o) {
    case Output::Hi: setOutput<Output::Hi>(set) ; break ;
    case Output::Lo: setOutput<Output::Lo>(set) ; break ;
    }
  }
    
  void setOutput(Pin pin,Output o)
  {
    setOutput(1u << pin.value(),o) ;
  }

private:

  std::shared_ptr<Page> page ;

  static uint32_t rmw(uint32_t volatile &r,uint32_t set,bool on) ;
  
} ; }

#endif // _Rpi_GpioOld_h_
