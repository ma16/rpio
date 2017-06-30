// BSD 2-Clause License, see github.com/ma16/rpio

#include "Spi1.h"
#include <vector>

static uint32_t reverse(uint32_t i)
{
    i = (i & 0xffff0000) >> 16 | (i & 0x0000ffff) << 16 ;
    i = (i & 0xff00ff00) >>  8 | (i & 0x00ff00ff) <<  8 ;
    i = (i & 0xf0f0f0f0) >>  4 | (i & 0x0f0f0f0f) <<  4 ;
    i = (i & 0xcccccccc) >>  2 | (i & 0x33333333) <<  2 ;
    i = (i & 0xaaaaaaaa) >>  1 | (i & 0x55555555) <<  1 ;
    return i ;
}

Device::Mcp3008::Spi1::Sample17
Device::Mcp3008::Spi1::query17(Circuit::Source source)
{
    // 17-bit dialog
    auto c0 = this->spi.getControl0() ;
    c0 &= ~Rpi::Spi1::Len ;
    c0 |= 17 ; 
    this->spi.setControl0(c0) ;
    uint32_t tx = 0 ;
    // channel number bit:0-3
    tx |= source.value() ;
    // start bit:4
    tx |= 0x10u ;
    // 5 bits, MSB first 
    tx <<= (32-5) ;
    auto rx = this->spi.xfer(tx) ;
    return Sample17(rx) ; 
}

Device::Mcp3008::Circuit::Sample 
Device::Mcp3008::Spi1::Sample17::fetch() const
{
    return Circuit::Sample::coset(this->i) ;
}

Device::Mcp3008::Spi1::Error
Device::Mcp3008::Spi1::Sample17::verify() const
{
    Error error ;
    // 7-bit head
    error.head = 0 != ((0x7f<<10) & (this->i ^ (0x7e<<10))) ;
    return error ;
}

Device::Mcp3008::Spi1::Sample26
Device::Mcp3008::Spi1::query26(Circuit::Source source)
{
    // 26-bit dialog
    auto c0 = this->spi.getControl0() ;
    c0 &= ~Rpi::Spi1::Len ;
    c0 |= 26 ; 
    this->spi.setControl0(c0) ;
    uint32_t tx = 0 ;
    // channel number bit:0-3
    tx |= source.value() ;
    // start bit:4
    tx |= 0x10u ;
    // 5 bits, MSB first 
    tx <<= (32-5) ;
    auto rx = this->spi.xfer(tx) ;
    return Sample26(rx) ; 
}

Device::Mcp3008::Circuit::Sample 
Device::Mcp3008::Spi1::Sample26::fetch() const
{
    // pos. 9..18
    return Circuit::Sample::coset(this->i >> 9) ;
}

Device::Mcp3008::Spi1::Error
Device::Mcp3008::Spi1::Sample26::verify() const
{
    Error error ;
    // 7-bit head
    error.head = 0 != ((0x7f<<19) & (this->i ^ (0x7e<<19))) ;
    // 9 + 1 + 9 bit sample
    auto msb =         this->i  >>  9 ; // pos. 9..18
    auto lsb = reverse(this->i) >> 22 ; // pos. 9..0
    error.mismatch = 0 != (0x3ff & (msb ^ lsb)) ;
    return error ;
}
