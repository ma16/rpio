// BSD 2-Clause License, see github.com/ma16/rpio

#include "Spi0.h"
#include <vector>

static uint32_t msb32(uint8_t const *p)
{
    uint32_t i = p[0] ;
    i <<= 8 ; i |= p[1] ;
    i <<= 8 ; i |= p[2] ;
    i <<= 8 ; i |= p[3] ;
    return i ;
}

static uint32_t reverse(uint32_t i)
{
    i = (i & 0xffff0000) >> 16 | (i & 0x0000ffff) << 16 ;
    i = (i & 0xff00ff00) >>  8 | (i & 0x00ff00ff) <<  8 ;
    i = (i & 0xf0f0f0f0) >>  4 | (i & 0x0f0f0f0f) <<  4 ;
    i = (i & 0xcccccccc) >>  2 | (i & 0x33333333) <<  2 ;
    i = (i & 0xaaaaaaaa) >>  1 | (i & 0x55555555) <<  1 ;
    return i ;
}

using Sample = Console::Mcp3008::Circuit::Sample ;

boost::optional<Sample> Console::Mcp3008::Spi0::query(Circuit::Source source)
{
    if (!this->monitor)
	return this->queryShort(source) ;
    auto sample = this->queryLong(source) ;
    if (sample.first.value() == sample.second.value())
	return sample.first ;
    return boost::none ;
}

Sample Console::Mcp3008::Spi0::queryShort(Circuit::Source source)
{
    uint8_t tx[4] = {0} ;
    tx[0] = static_cast<uint8_t>(0x80 | (source.value() << 3)) ;
    uint8_t rx[4] ;
    this->spi.transceive(3,tx,rx) ;
    return Circuit::Sample::coset(msb32(rx) >> 15) ; // pos. 7-16
}

std::pair<Sample,Sample> Console::Mcp3008::Spi0::queryLong(Circuit::Source source)
{
    uint8_t tx[4] = {0} ;
    tx[0] = static_cast<uint8_t>(0x80 | (source.value() << 3)) ;
    uint8_t rx[4] ;
    this->spi.transceive(4,tx,rx) ;
    auto i = msb32(rx) ;
    auto msb = Circuit::Sample::coset(        i  >> 15) ; // pos.  7-16
    auto lsb = Circuit::Sample::coset(reverse(i) >> 16) ; // pos. 25-16
    return std::make_pair(msb,lsb) ;
}
