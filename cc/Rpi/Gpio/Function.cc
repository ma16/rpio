// BSD 2-Clause License, see github.com/ma16/rpio

#include "Function.h"

Rpi::Gpio::Function::Type Rpi::Gpio::Function::get(Pin pin) const
{
    static_assert(pin.max < 54,"") ;
    auto i = Page::Index::coset(0x0u + pin.value()/10u) ;
    // ...10 pins per bank
    auto r = 3u * (pin.value()%10u) ;
    // ...3 bits for each pin
    auto w = this->page->at(i) ;
    w >>= r ;
    return Numerator::coset(w).e() ;
}

void Rpi::Gpio::Function::set(Pin pin,Type mode)
{
    static_assert(pin.max < 54,"") ;
    auto i = Page::Index::coset(0x0u + pin.value()/10u) ;
    // ...10 pins per bank
    auto r = 3u * (pin.value()%10u) ; 
    // ...3 bits for each pin
    auto w = this->page->at(i) ;
    w &= ~(7u << r) ; 
    w |= Numerator(mode).n() << r ;
    this->page->at(i) = w ;
    // [todo] read back and check if the same (race conditions)
}

/*
void Rpi::Gpio::Function::setType(uint32_t set,Type mode)
{
  auto i = Pin::first() ; 
  do {
    if (0 == (set & (1u << i.value())))
      continue ;
    this->setType(i,mode) ;
  } while (i.next()) ;
}
*/
