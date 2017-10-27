// BSD 2-Clause License, see github.com/ma16/rpio

#include "GpioOld.h"
#include "Error.h"

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
