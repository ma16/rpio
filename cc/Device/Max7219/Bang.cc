// BSD 2-Clause License, see github.com/ma16/rpio

#include "Bang.h"
#include <chrono>
#include <iostream>
#include <Neat/cast.h>

void Device::Max7219::Bang::send(RpiExt::Bang::Enqueue *q,uint16_t data)
{
    for (uint_fast16_t mask=(1u<<15) ; mask!=0 ; mask>>=1)
    {
	// DIN is shifted-in with the LH-edge on CLK
	if (0 == (data & mask))
	    q->reset(this->pins.din) ;
	else  q->set(this->pins.din) ;
	q->sleep(50/4) ; 
	q->set(this->pins.clk) ; 
	q->sleep(50/4) ; 
	q->reset(this->pins.clk) ; 
    }
}

void Device::Max7219::Bang::load(RpiExt::Bang::Enqueue *q)
{
    // all shift-registers are loaded with a LHL clock pulse on LOAD
    q->reset(this->pins.load) ; 
    q->sleep(50/4) ; 
    q->set(this->pins.load) ;
}
