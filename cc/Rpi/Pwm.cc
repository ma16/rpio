// BSD 2-Clause License, see github.com/ma16/rpio

#include "Error.h"
#include "Pwm.h"
#include <cassert>

constexpr Rpi::Pwm::Control::Word Rpi::Pwm::Control::Flags:: Clrf ;
constexpr Rpi::Pwm::Control::Word Rpi::Pwm::Control::Flags::Mode1 ;
constexpr Rpi::Pwm::Control::Word Rpi::Pwm::Control::Flags::Mode2 ;
constexpr Rpi::Pwm::Control::Word Rpi::Pwm::Control::Flags::Msen1 ;
constexpr Rpi::Pwm::Control::Word Rpi::Pwm::Control::Flags::Msen2 ;
constexpr Rpi::Pwm::Control::Word Rpi::Pwm::Control::Flags::Pola1 ;
constexpr Rpi::Pwm::Control::Word Rpi::Pwm::Control::Flags::Pola2 ;
constexpr Rpi::Pwm::Control::Word Rpi::Pwm::Control::Flags::Pwen1 ;
constexpr Rpi::Pwm::Control::Word Rpi::Pwm::Control::Flags::Pwen2 ;
constexpr Rpi::Pwm::Control::Word Rpi::Pwm::Control::Flags::Rptl1 ;
constexpr Rpi::Pwm::Control::Word Rpi::Pwm::Control::Flags::Rptl2 ;
constexpr Rpi::Pwm::Control::Word Rpi::Pwm::Control::Flags::Sbit1 ;
constexpr Rpi::Pwm::Control::Word Rpi::Pwm::Control::Flags::Sbit2 ;
constexpr Rpi::Pwm::Control::Word Rpi::Pwm::Control::Flags::Usef1 ;
constexpr Rpi::Pwm::Control::Word Rpi::Pwm::Control::Flags::Usef2 ;

Rpi::Pwm::Control::Bank const& Rpi::Pwm::Control::Bank::select(Index i)
{
    static Bank array[2] =
	{ {
		Flags::Mode1,
		Flags::Msen1,
		Flags::Pola1,
		Flags::Pwen1,
		Flags::Rptl1,
		Flags::Sbit1,
		Flags::Usef1,
	    },{
		Flags::Mode2,
		Flags::Msen2,
		Flags::Pola2,
		Flags::Pwen2,
		Flags::Rptl2,
		Flags::Sbit2,
		Flags::Usef2
	} } ;
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
