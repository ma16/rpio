// BSD 2-Clause License, see github.com/ma16/rpio

#include "Event.h"
#include "../Error.h"

constexpr Rpi::Peripheral::PNo Rpi::Gpio::Event::PNo ;

uint32_t Rpi::Gpio::Event::rmw(uint32_t volatile &r,uint32_t set,bool on)
{
    auto prev = r ;
    auto next = prev & ~set ; 
    if (on)
	next |= set ; 
    if (next != prev)
	r = next ;
    return prev ;
}
