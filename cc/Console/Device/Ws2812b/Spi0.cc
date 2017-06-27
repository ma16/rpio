// BSD 2-Clause License, see github.com/ma16/rpio

#include "Spi0.h"
#include <cassert>

void Console::Ws2812b::Spi0::BitStream::push_back(bool bit,double ticks)
{
  for (auto i=0u ; i<static_cast<unsigned>(ticks+.5) ; ++i)
    // [todo] watch overflows
    this->push_back(bit) ;
}

void Console::Ws2812b::Spi0::BitStream::push_back(Timing const &t,double f,uint32_t rgb)
{
  for (uint32_t mask=(1u<<23) ; mask!=0 ; mask>>=1) {
    if (0 == (rgb & mask)) {
      this->push_back(1,t.t0h*f) ;
      this->push_back(0,t.t0l*f) ;
    }
    else {
      this->push_back(1,t.t1h*f) ;
      this->push_back(0,t.t1l*f) ;
    }
  }
}
  
void Console::Ws2812b::Spi0::BitStream::push_back(Timing const &t,double f,uint32_t rgb,size_t n)
{
  for (unsigned i=0 ; i<n ; ++i)
    this->push_back(t,f,rgb) ;
}

std::vector<uint8_t> Console::Ws2812b::Spi0::BitStream::to_uint8() const
{
  assert(0 == (this->q.size() % 8)) ;
  auto n = (this->q.size()+7) / 8 ;
  std::vector<uint8_t> v ;
  v.reserve(n) ;
  unsigned i = 0 ;
  uint8_t bits = 0 ; // := prevents warning
  for (auto bit : this->q) {
    bits = static_cast<uint8_t>(bits << 1) ;
    bits = static_cast<uint8_t>(bits | bit) ;
    if (++i < 8)
      continue ;
    v.push_back(bits) ;
    i = 0 ;
  }
  return v ;
}

void Console::Ws2812b::Spi0::send(uint32_t nleds,uint32_t rgb)
{
  BitStream q ;
  q.push_back(0,t.res*f+1) ; // +1 to make sure mininum duration
  q.push_back(t,f,rgb,nleds) ;
  q.push_back(0,t.res*f+1) ; // +1 to make sure mininum duration
  while (0 != (q.size() % 8))
    q.push_back(0) ;
  auto idata = q.to_uint8() ;
  auto odata = idata ; odata.resize(0) ;
  this->spi.setDlen(2) ; // any value ge 2
  uint32_t c = 0 ;
  c |= Rpi::Spi0::ClearRx ;
  c |= Rpi::Spi0::ClearTx ;
  c |= Rpi::Spi0::Ta  ;
  this->spi.setControl(c) ;
  for (auto p=idata.begin() ; p!=idata.end() ; ) {
    auto c = this->spi.getControl() ;
    if (c & Rpi::Spi0::Rxd) odata.push_back(this->spi.read()) ;
    if (c & Rpi::Spi0::Txd) this->spi.write(*p++) ;
  }
  while (odata.size() < idata.size()) {
    auto c = this->spi.getControl() ;
    if (c & Rpi::Spi0::Rxd) {
      odata.push_back(this->spi.read()) ;
    }
  }
  c = this->spi.getControl() ;
  c &= ~Rpi::Spi0::Ta ;
  this->spi.setControl(c) ;
}
