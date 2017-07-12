// BSD 2-Clause License, see github.com/ma16/rpio

#include "BitStream.h"
#include <cassert>
#include <sstream>

template<typename T> static std::string
toStr(Device::Ws2812b::BitStream::Timing<T> const &t)
{
    std::ostringstream os ;
    os.setf(std::ios::scientific) ;
    os.precision(2) ;
    os << "0-bit:(" << t.bit_0.hi << ',' << t.bit_0.lo << ") "
       << "1-bit:(" << t.bit_1.hi << ',' << t.bit_1.lo << ") "
       << "reset:" << t.reset ;
    return os.str() ;
}

namespace Device { namespace Ws2812b {

template<> std::string BitStream::Seconds::toStr() const { return ::toStr(*this) ; }
template<> std::string BitStream::Ticks  ::toStr() const { return ::toStr(*this) ; }

} }

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
	    this->pushBit(1,this->ticks.bit_0.hi) ;
	    this->pushBit(0,this->ticks.bit_0.lo) ;
	}
	else
	{
	    this->pushBit(1,this->ticks.bit_1.hi) ;
	    this->pushBit(0,this->ticks.bit_1.lo) ;
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
	// last word padded with zeros (extends ticks.reset)
	bits <<= (32-i) ;
	v.push_back(bits) ;
    }
    // [defect] PWM needs two additional words (extends ticks.reset)
    v.push_back(0) ;
    v.push_back(0) ;
    return v ;
}

std::vector<uint32_t> Device::Ws2812b::BitStream::
make(Ticks const &ticks,uint32_t grb,size_t n)
{
    BitStream bs(ticks) ;
    bs.pushBit(0,ticks.reset) ;
    // ...the datasheet doesn't really say how to start a dialogue.
    // By observation: the data-line has to be low for a little while
    // before the transmission can start (about 2 pixel-times).
    // Anyway, we use the reset/latch time here (which should be a
    // safe bet).
    bs.push(grb,n) ;
    bs.pushBit(0,ticks.reset) ;
    return bs.copy() ;
}
