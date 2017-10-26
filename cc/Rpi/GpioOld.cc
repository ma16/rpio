// BSD 2-Clause License, see github.com/ma16/rpio

#include "GpioOld.h"
#include "Error.h"

Rpi::GpioOld::Mode Rpi::GpioOld::getMode(Pin pin) const
{
  auto i = Page::Index::make(0x0u + pin.value()/10u) ; // GPFSEL0..GPFSEL5
  auto bits = this->page->at(i) ; // 10 pins per register
  auto r = 3u * (pin.value()%10u) ; // 3 bits for each pin
  bits >>= r ;
  return ModeN::make(bits & 7u).e() ;
}

void Rpi::GpioOld::setMode(Pin pin,Mode mode)
{
  auto i = Page::Index::make(0x0u + pin.value()/10u) ; // GPFSEL0..GPFSEL5
  auto bits = this->page->at(i) ; // 10 pins per register
  auto r = 3u * (pin.value()%10u) ; // 3 bits for each pin
  bits &= ~(7u << r) ; 
  bits |= (static_cast<uint32_t>(ModeN(mode).n()) << r) ;
  this->page->at(i) = bits ;
}

void Rpi::GpioOld::setMode(uint32_t set,Mode mode)
{
  auto i = Pin::first() ; 
  do {
    if (0 == (set & (1u << i.value())))
      continue ;
    this->setMode(i,mode) ;
  } while (i.next()) ;
}

uint32_t Rpi::GpioOld::rmw(uint32_t volatile &r,uint32_t set,bool on)
{
  auto prev = r ;
  auto next = prev & ~set ; 
  if (on)
    next |= set ; 
  if (next != prev)
    r = next ;
  return prev ;
}

void Rpi::GpioOld::setPull(uint32_t set,Pull how)
{
  auto &GPPUD     = this->page->at<0x94/4>() ; 
  auto &GPPUDCLK0 = this->page->at<0x98/4>() ;
  
  // 1. Write to GPPUD to set the required control signal (i.e. Pull-
  //    up or Pull-Down or neither to remove the current Pull-up/down)
  GPPUD = PullN(how).n() ;

  // 2. Wait 150 cycles – this provides the required set-up time for 
  //    the control signal
  for (unsigned i=0 ; i<150 ; ++i) {
    auto x=GPPUD ; (void)x ;
  }
  // ...but what is a "cycle"? [todo]

  // 3. Write to GPPUDCLK0/1 to clock the control signal into the GPIO 
  //    pads you wish to modify – NOTE only the pads which receive a 
  //    clock will be modified, all others will retain their previous 
  //    state.
  GPPUDCLK0 = set ;

  // 4. Wait 150 cycles – this provides the required hold time for the 
  //    control signal
  for (unsigned i=0 ; i<150 ; ++i) {
    auto x=GPPUD ; (void)x ;
  }
  // ...again: what is a "cycle"?  [todo]

  // 5. Write to GPPUD to remove the control signal
  GPPUD = 0u ; 
  // ...write what? I guess it doesn't matter. [todo]

  // 6. Write to GPPUDCLK0/1 to remove the clock
  GPPUDCLK0 = 0 ; 
  // ...write what? I guess zero is ok to "close" the gate. [todo]
}
