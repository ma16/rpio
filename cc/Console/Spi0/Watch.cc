// BSD 2-Clause License, see github.com/ma16/rpio

// --------------------------------------------------------------------
// Todo:
// * we might need memory barriers
// * we might need to copy volatile data word by word
// * see also: http://stackoverflow.com/questions/34884313/need-help-mapping-pre-reserved-cacheable-dma-buffer-on-xilinx-arm-soc-zynq    
// --------------------------------------------------------------------

#include "Watch.h"

#include <algorithm> // std::copy
#include <cassert>

Console::Spi0::Watch::Watch(void const *front,size_t nrecords,size_t rsize,size_t i)
    : front(reinterpret_cast<uint8_t const*>(front)),
      nrecords(nrecords),rsize(rsize)
{
    assert(nrecords > 0) ;
    this->reset(i) ;
}

void Console::Spi0::Watch::reset(size_t i)
{
    assert(i < this->nrecords) ;
    this->i0 = i ;
    auto p0 = this->front + this->i0 * this->rsize ;
    this->t0 = *reinterpret_cast<volatile uint32_t const*>(p0) ;
}

bool Console::Spi0::Watch::overrun() const
{
    auto p0 = this->front + this->i0 * this->rsize ;
    auto t0_now = *reinterpret_cast<volatile uint32_t const*>(p0) ;
    return this->t0 != t0_now ;
}
    
size_t Console::Spi0::Watch::avail(size_t i) const
{
    assert(i < this->nrecords) ;
    auto i1 = (i==0) ? (this->nrecords-1) : (i-1) ;
    // ...the current record may not be complete (yet)
    auto n = (i1 >= this->i0) ? (i1 - this->i0) : (this->nrecords - this->i0 + i1) ;
    // ...total number of fresh records
    return n ;
    // might be overrun though
}
    
bool Console::Spi0::Watch::fetch(size_t n,void *buffer)
{
    assert(n <= this->nrecords) ;
    auto i1 = (this->i0 < nrecords-n)
      ? (this->i0+n)
      : (this->i0+n-this->nrecords) ;
    
    auto p0 = this->front + this->i0 * this->rsize ;
    auto p1 = this->front +       i1 * this->rsize ;
    
    auto t0_now = *reinterpret_cast<volatile uint32_t const*>(p0) ;
    auto t1_now = *reinterpret_cast<volatile uint32_t const*>(p1) ;

    if (i1 >= i0)
    {
	std::copy(p0 + this->rsize,p1 + this->rsize,static_cast<uint8_t*>(buffer)) ;
    }
    else
    {
	auto halfway = (this->nrecords - 1 - this->i0) * this->rsize ;
	
	std::copy(p0 + this->rsize,p0 + this->rsize + halfway,static_cast<uint8_t*>(buffer)) ;
	
	std::copy(this->front,p1 + this->rsize,static_cast<uint8_t*>(buffer) + halfway) ;
    }
    
    t0_now = *reinterpret_cast<volatile uint32_t const*>(p0) ;
    if (this->t0 != t0_now)
	return false ;
    
    this->i0 = i1 ;
    this->t0 = t1_now ;
    return true ;
}
