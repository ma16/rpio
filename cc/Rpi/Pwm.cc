// BSD 2-Clause License, see github.com/ma16/rpio

#include "Error.h"
#include "Pwm.h"
#include <cassert>

constexpr Rpi::Bus::Address Rpi::Pwm::fifoAddr ;

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
