// BSD 2-Clause License, see github.com/ma16/rpio

#include "Error.h"
#include "Pwm.h"
#include <cassert>

constexpr Rpi::Bus::Address Rpi::Pwm::fifoAddr ;

Rpi::Pwm::Control::Control(uint32_t w)
{
    // [todo] use initializer list

    this->clear = w & Clrf ;
    
    this->channel[0].pwen = 0 != (w & Pwen1) ;
    this->channel[0].mode = 0 != (w & Mode1) ;
    this->channel[0].rptl = 0 != (w & Rptl1) ;
    this->channel[0].sbit = 0 != (w & Sbit1) ;
    this->channel[0].pola = 0 != (w & Pola1) ;
    this->channel[0].usef = 0 != (w & Usef1) ;
    this->channel[0].msen = 0 != (w & Msen1) ;
    
    this->channel[1].pwen = 0 != (w & Pwen2) ;
    this->channel[1].mode = 0 != (w & Mode2) ;
    this->channel[1].rptl = 0 != (w & Rptl2) ;
    this->channel[1].sbit = 0 != (w & Sbit2) ;
    this->channel[1].pola = 0 != (w & Pola2) ;
    this->channel[1].usef = 0 != (w & Usef2) ;
    this->channel[1].msen = 0 != (w & Msen2) ;
}

uint32_t Rpi::Pwm::Control::value() const
{
    uint32_t w = 0 ;

    if (this->clear) w |= Clrf ;

    if (this->channel[0].pwen) w |= Pwen1 ;
    if (this->channel[0].mode) w |= Mode1 ;
    if (this->channel[0].rptl) w |= Rptl1 ;
    if (this->channel[0].sbit) w |= Sbit1 ;
    if (this->channel[0].pola) w |= Pola1 ;
    if (this->channel[0].usef) w |= Usef1 ;
    if (this->channel[0].msen) w |= Msen1 ;
    
    if (this->channel[1].pwen) w |= Pwen2 ;
    if (this->channel[1].mode) w |= Mode2 ;
    if (this->channel[1].rptl) w |= Rptl2 ;
    if (this->channel[1].sbit) w |= Sbit2 ;
    if (this->channel[1].pola) w |= Pola2 ;
    if (this->channel[1].usef) w |= Usef2 ;
    if (this->channel[1].msen) w |= Msen2 ;
    
    return w ;
}
    
Rpi::Pwm::Status::Status(uint32_t w)
{
    // [todo] use initializer list

    this->full = w & Full ;
    this->empt = w & Empt ;
    this->werr = w & Werr ;
    this->rerr = w & Rerr ;
    this->berr = w & Berr ;
    
    this->channel[0].gapo = 0 != (w & Gapo1) ;
    this->channel[0].sta  = 0 != (w &  Sta1) ;

    this->channel[1].gapo = 0 != (w & Gapo2) ;
    this->channel[1].sta  = 0 != (w &  Sta2) ;
}

uint32_t Rpi::Pwm::Status::value() const
{
    uint32_t w = 0 ;

    if (this->full) w |= Full ;
    if (this->empt) w |= Empt ;
    if (this->werr) w |= Werr ;
    if (this->rerr) w |= Rerr ;
    if (this->berr) w |= Berr ;

    if (this->channel[0].gapo) w |= Gapo1 ;
    if (this->channel[0].sta ) w |=  Sta1 ;
    
    if (this->channel[1].gapo) w |= Gapo2 ;
    if (this->channel[1].sta ) w |=  Sta2 ;
    
    return w ;
    // [todo] not in write mask: (Full,Empt)
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
