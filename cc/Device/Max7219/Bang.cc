// BSD 2-Clause License, see github.com/ma16/rpio

#include "Bang.h"
#include <cmath>

Device::Max7219::Bang::Ticks
Device::Max7219::Bang::asTicks(Seconds const &s,double freq)
{
    auto f = [freq](double s) {
	return static_cast<uint32_t>(s*freq+1.5) ; } ;
    return Ticks(f(s.ch),f(s.cl),f(s.csw),f(s.ds),f(s.ldck)) ;
}

void Device::Max7219::Bang::send(RpiExt::Bang::Enqueue *q,uint16_t data)
{
    for (unsigned mask=(1u<<15) ; mask!=0 ; mask>>=1)
    {
	// DIN is shifted-in with the LH-edge on CLK
	if (0 == (data & mask))
	    q->reset(this->pins.din) ;
	else  q->set(this->pins.din) ;
	q->sleep(this->ticks.ds) ;
	
	q->sleep(this->ticks.cl) ; // [todo] since last HL
	q->set(this->pins.clk) ; 
	q->sleep(this->ticks.ch) ; 
	q->reset(this->pins.clk) ; 
    }
}

void Device::Max7219::Bang::load(RpiExt::Bang::Enqueue *q)
{
    // all shift-registers are loaded with a LHL clock pulse on LOAD
    q->set(this->pins.load) ;
    q->sleep(this->ticks.csw) ;
    q->reset(this->pins.load) ; 
    q->sleep(this->ticks.ldck) ; // [todo] before CLK
}
