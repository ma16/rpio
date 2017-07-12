// BSD 2-Clause License, see github.com/ma16/rpio

#include "BitStream.h"
#include <cassert>
#include <iostream> // debugging

void Device::Ws2812b::BitStream::pushBit(bool bit,size_t n)
{
    for (decltype(n) i=0 ; i<n ; ++i)
	this->q.push_back(bit) ;
}

void Device::Ws2812b::BitStream::pushWord(uint32_t grb)
{
    for (uint32_t mask=(1u<<23) ; mask!=0 ; mask>>=1)
    {
	if (0 == (grb & mask))
	{
	    this->pushBit(1,this->ticks.t0h) ;
	    this->pushBit(0,this->ticks.t0l) ;
	}
	else
	{
	    this->pushBit(1,this->ticks.t1h) ;
	    this->pushBit(0,this->ticks.t1l) ;
	}
    }
}
  
void Device::Ws2812b::BitStream::push(uint32_t grb,size_t n)
{
    for (decltype(n) i=0 ; i<n ; ++i)
	this->pushWord(grb) ;
}

std::vector<uint32_t> Device::Ws2812b::BitStream::copy() const
{
    auto n = (this->q.size()+31) / 32 ;
    std::vector<uint32_t> v ;
    v.reserve(n+2) ;
    unsigned i = 0 ;
    uint32_t bits = 0 ;
    for (auto bit : this->q)
    {
	bits <<= 1 ;
	bits |= bit ;
	if (++i < 32)
	    continue ;
	v.push_back(bits) ;
	i = 0 ;
    }
    if (i != 0)
    {
	// last word padded with zeros (extends ticks.res)
	bits <<= (32-i) ;
	v.push_back(bits) ;
    }
    // [defect] PWM needs two additional words (extends ticks.res)
    v.push_back(0) ;
    v.push_back(0) ;
    return v ;
}

std::vector<uint32_t> Device::Ws2812b::BitStream::
make(Ticks const &ticks,uint32_t grb,size_t n)
{
    BitStream bs(ticks) ;
    bs.pushBit(0,ticks.res) ;
    // ...the datasheet doesn't really say how to start a dialogue.
    // The data line has to be low for a little time t before the
    // transmission can start: 2 pixel <= t <= t.res (by observation).
    // Anyway, with t.res we have to be on the safe side.
    bs.push(grb,n) ;
    bs.pushBit(0,ticks.res) ;
    return bs.copy() ;
}
