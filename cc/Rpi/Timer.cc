// BSD 2-Clause License, see github.com/ma16/rpio

#include "Timer.h"

constexpr Rpi::Bus::Address Rpi::Timer::cLoAddr ;
// [note] http://stackoverflow.com/questions/8016780/undefined-reference-to-static-constexpr-char#8016853

uint64_t Rpi::Timer::clock() const
{
  auto hi  = this->cHi() ;
  auto lo  = this->cLo() ;
  auto chk = this->cHi() ;
  while (chk != hi) {
    hi  = chk ;
    lo  = this->cLo() ;
    chk = this->cHi() ;
  }
  auto i = static_cast<uint64_t>(hi) ;
  i <<= 32 ;
  i |= lo ;
  return i ;
}
