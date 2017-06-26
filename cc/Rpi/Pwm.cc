// BSD 2-Clause License, see github.com/ma16/rpio

#include "Error.h"
#include "Pwm.h"
#include <cassert>

constexpr Rpi::Bus::Address Rpi::Pwm::fifoAddr ;

Rpi::Pwm::Control::Channel Rpi::Pwm::Control::get(Index i) const
{
  Channel x ;
  if (i.value() == 0) {
    x.pwen = 0 != this->cpwen1() ;
    x.mode = 0 != this->cmode1() ;
    x.rptl = 0 != this->crptl1() ;
    x.sbit = 0 != this->csbit1() ;
    x.pola = 0 != this->cpola1() ;
    x.usef = 0 != this->cusef1() ;
    x.msen = 0 != this->cmsen1() ;
  }
  else {
    assert(i.value() == 1) ;
    x.pwen = 0 != this->cpwen2() ;
    x.mode = 0 != this->cmode2() ;
    x.rptl = 0 != this->crptl2() ;
    x.sbit = 0 != this->csbit2() ;
    x.pola = 0 != this->cpola2() ;
    x.usef = 0 != this->cusef2() ;
    x.msen = 0 != this->cmsen2() ;
  }
  return x ;
}

void Rpi::Pwm::Control::set(Index i,Channel x)
{
  if (i.value() == 0) {
    pwen1() = x.pwen ;
    mode1() = x.mode ;
    rptl1() = x.rptl ;
    sbit1() = x.sbit ;
    pola1() = x.pola ;
    usef1() = x.usef ;
    msen1() = x.msen ;
  }
  else {
    assert(i.value() == 1) ;
    pwen2() = x.pwen ;
    mode2() = x.mode ;
    rptl2() = x.rptl ;
    sbit2() = x.sbit ;
    pola2() = x.pola ;
    usef2() = x.usef ;
    msen2() = x.msen ;
  }
}
    
void Rpi::Pwm::setRange(Index i,uint32_t r)
{
  if (i.value() == 0) this->page->at<0x10/4>() = r ;
  else                this->page->at<0x20/4>() = r ;
}

uint32_t Rpi::Pwm::getRange(Index i) const
{
  if (i.value() == 0) return this->page->at<0x10/4>() ;
  else                return this->page->at<0x20/4>() ;
}
  
void Rpi::Pwm::setData(Index i ,uint32_t d)
{
  if (i.value() == 0) this->page->at<0x14/4>() = d ;
  else                this->page->at<0x24/4>() = d ;
}
  
uint32_t Rpi::Pwm::getData(Index i) const
{
  if (i.value() == 0) return this->page->at<0x14/4>() ;
  else                return this->page->at<0x24/4>() ;
}

Rpi::Pwm::Dmac Rpi::Pwm::getDmac() const
{
  auto r = this->page->at<0x8/4>() ;
  bool enable = 0 != (r & (1u<<31)) ;
  auto panic = static_cast<uint8_t>(r >> 8) ;
  auto dreq  = static_cast<uint8_t>(r     ) ;
  return Dmac(enable,panic,dreq) ;
}

void Rpi::Pwm::setDmac(Rpi::Pwm::Dmac dmac)
{
  uint32_t r = 0 ;
  r |= static_cast<uint32_t>(dmac.enable) << 31 ;
  r |= static_cast<uint32_t>(dmac. panic) <<  8 ;
  r |= static_cast<uint32_t>(dmac.  dreq)       ;
  this->page->at<0x8/4>() = r ;
}
