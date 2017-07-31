// BSD 2-Clause License, see github.com/ma16/rpio

#include "Error.h"
#include "Pwm.h"
#include <cassert>

using Control = Rpi::Pwm::Control ;
using Status  = Rpi::Pwm:: Status ;

constexpr Control::Word::Digit Control:: Clrf ;
constexpr Control::Word::Digit Control::Mode1 ;
constexpr Control::Word::Digit Control::Mode2 ;
constexpr Control::Word::Digit Control::Msen1 ;
constexpr Control::Word::Digit Control::Msen2 ;
constexpr Control::Word::Digit Control::Pola1 ;
constexpr Control::Word::Digit Control::Pola2 ;
constexpr Control::Word::Digit Control::Pwen1 ;
constexpr Control::Word::Digit Control::Pwen2 ;
constexpr Control::Word::Digit Control::Rptl1 ;
constexpr Control::Word::Digit Control::Rptl2 ;
constexpr Control::Word::Digit Control::Sbit1 ;
constexpr Control::Word::Digit Control::Sbit2 ;
constexpr Control::Word::Digit Control::Usef1 ;
constexpr Control::Word::Digit Control::Usef2 ;

constexpr Status::Word::Digit Status::Berr ;
constexpr Status::Word::Digit Status::Empt ;
constexpr Status::Word::Digit Status::Full ;
constexpr Status::Word::Digit Status::Gap1 ;
constexpr Status::Word::Digit Status::Gap2 ;
constexpr Status::Word::Digit Status::Rerr ;
constexpr Status::Word::Digit Status::Sta1 ;
constexpr Status::Word::Digit Status::Sta2 ;
constexpr Status::Word::Digit Status::Werr ;

Rpi::Pwm::Control::Bank const& Rpi::Pwm::Control::Bank::select(Index i)
{
    static Bank array[2] = {
	{ Mode1,Msen1,Pola1,Pwen1,Rptl1,Sbit1,Usef1 },
	{ Mode2,Msen2,Pola2,Pwen2,Rptl2,Sbit2,Usef2 },
    } ;
    return array[i.value()] ;
}

Rpi::Pwm::Status::Bank const& Rpi::Pwm::Status::Bank::select(Index i)
{
    static Bank array[2] = {
	{ Gap1,Sta1 },
	{ Gap2,Sta2 },
    } ;
    return array[i.value()] ;
}

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
