// BSD 2-Clause License, see github.com/ma16/rpio

#include "BitStream.h"
#include <cassert>

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
	    this->pushBit(1, this->ticks.t0h) ;
	    this->pushBit(0,this->ticks.t0l) ;
	}
	else
	{
	    this->pushBit(1, this->ticks.t1h) ;
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
    assert(0 == (this->q.size() % 32)) ;
    auto n = (this->q.size()+31) / 32 ;
    std::vector<uint32_t> v ;
    v.reserve(n) ;
    unsigned i = 0 ;
    uint32_t bits = 0 ; // compiler may complain if not init.
    for (auto bit : this->q)
    {
	bits <<= 1 ;
	bits |= bit ;
	if (++i < 32)
	    continue ;
	v.push_back(bits) ;
	i = 0 ;
    }
    return v ;
}

std::vector<uint32_t> Device::Ws2812b::BitStream::
make(Ticks const &ticks,uint32_t grb,size_t n)
{
    BitStream bs(ticks) ;
    bs.pushBit(0,ticks.res) ; // [todo] do we need this?
    bs.push(grb,n) ;
    bs.pushBit(0,ticks.res) ;
    bs.pushBit(0,32) ; // for RPTL [todo] why again?
    auto size = bs.q.size() ;
    auto rup = ~0x1fu & (size + 0x1fu) ;
    bs.pushBit(0,rup-size) ;
    return bs.copy() ;
}
