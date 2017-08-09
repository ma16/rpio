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

constexpr Rpi::Bus::Address Rpi::Pwm::Control::Address ;

constexpr Rpi::Bus::Address Rpi::Pwm::Status::Address ;

constexpr Rpi::Bus::Address Rpi::Pwm::Fifo::Address ;

Rpi::Pwm::DmaC::Word Rpi::Pwm::DmaC::Word::make(uint32_t r)
{
    bool enable = 0 != (r & (1u<<31)) ;
    auto panic = static_cast<uint8_t>(r >> 8) ;
    auto dreq  = static_cast<uint8_t>(r     ) ;
    return Word(enable,panic,dreq) ;
}

uint32_t Rpi::Pwm::DmaC::Word::value() const
{
    uint32_t r = 0 ;
    r |= static_cast<uint32_t>(this->enable) << 31 ;
    r |= static_cast<uint32_t>(this->panic)  <<  8 ;
    r |= static_cast<uint32_t>(this-> dreq)        ;
    return r ;
}
