// BSD 2-Clause License, see github.com/ma16/rpio

#include "Error.h"
#include "Pwm.h"
#include <cassert>

using Control = Rpi::Pwm::Control ;
using Status  = Rpi::Pwm:: Status ;

Rpi::Pwm::Control::Bank const& Rpi::Pwm::Control::Bank::select(Index i)
{
    static Bank array[2] = {
      { Mode1::Digit,
	Msen1::Digit,
	Pola1::Digit,
	Pwen1::Digit,
	Rptl1::Digit,
	Sbit1::Digit,
	Usef1::Digit },
      { Mode2::Digit,
	Msen2::Digit,
	Pola2::Digit,
	Pwen2::Digit,
	Rptl2::Digit,
	Sbit2::Digit,
	Usef2::Digit },
    } ;
    return array[i.value()] ;
}

Rpi::Pwm::Status::Bank const& Rpi::Pwm::Status::Bank::select(Index i)
{
    static Bank array[2] = {
	{ Gap1::Digit,Sta1::Digit },
	{ Gap2::Digit,Sta2::Digit },
    } ;
    return array[i.value()] ;
}

constexpr Rpi::Bus::Address Rpi::Pwm::Control::Address ;

constexpr Rpi::Bus::Address Rpi::Pwm::Status::Address ;

constexpr Rpi::Bus::Address Rpi::Pwm::Fifo::Address ;
