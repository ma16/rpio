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

Device::Mcp3008::Spi0::Sample24
Device::Mcp3008::Spi0::query24(Circuit::Source source)
{
    // 17-bit dialog
    uint8_t tx[4] = {0} ; // MOSI: 1 + SOURCE:4 + ANY:12 (+ANY:7)
    tx[0] = static_cast<uint8_t>(0x80 | (source.value() << 3)) ;
    uint8_t rx[4] ; // MISO: Hi:6 + Lo:1 + MSB:10 (+Lo:7)
    this->spi.transceive(3,tx,rx) ;
    // SPI0 transfers are a multiple of 8-bit. Thus, 17 bits become
    // 24 bits and so we're going to receive also 7 LSB bits of the
    // sample. We use 32-bit buffers (instead of 24-bit) because of
    // the msb32() function.
    return Sample24(msb32(rx)) ;
}

Device::Mcp3008::Circuit::Sample 
Device::Mcp3008::Spi0::Sample24::fetch() const
{
    // pos. 7-16
    return Circuit::Sample::coset(this->i >> 15) ;
}

Device::Mcp3008::Spi0::Error
Device::Mcp3008::Spi0::Sample24::verify() const
{
    Error error ;
    // 7-bit head
    error.head = 0 != (0xfe000000 & (this->i ^ 0xfc000000)) ;
    auto msb =         this->i  >> 15 ; // pos.  7-16
    auto lsb = reverse(this->i) >> 16 ; // pos. 25-16
    // 9 + 1 + 7 bit sample
    error.mismatch = 0 != (0xff & (msb ^ lsb)) ;
    error.tail = 0 ;
    return error ;
}


Device::Mcp3008::Spi0::Sample32
Device::Mcp3008::Spi0::query32(Circuit::Source source)
{
    // 26-bit dialog
    uint8_t tx[4] = {0} ; // MOSI: 1 + SOURCE:4 + ANY:21 (+ANY:6)
    tx[0] = static_cast<uint8_t>(0x80 | (source.value() << 3)) ;
    uint8_t rx[4] ; // MISO: Hi:6 + Lo:1 + MSB:10 + LSB:9 (+Lo:6)
    this->spi.transceive(4,tx,rx) ;
    // SPI0 transfers are a multiple of 8-bit. Thus, 26 bits become
    // 32 bits and so we're going to receive 6 additional Lo bits.
    return Sample32(msb32(rx)) ;
}

Device::Mcp3008::Circuit::Sample 
Device::Mcp3008::Spi0::Sample32::fetch() const
{
    // pos. 7-16
    return Circuit::Sample::coset(this->i >> 15) ;
}

Device::Mcp3008::Spi0::Error
Device::Mcp3008::Spi0::Sample32::verify() const
{
    Error error ;
    // 7-bit head
    error.head = 0 != (0xfe000000 & (this->i ^ 0xfc000000)) ;
    // 9 + 1 + 9 bit sample
    auto msb =         this->i  >> 15 ; // pos.  7-16
    auto lsb = reverse(this->i) >> 16 ; // pos. 25-16
    error.mismatch = 0 != (0x3ff & (msb ^ lsb)) ;
    // 6-bit tail
    error.tail = 0 != (0x3f & this->i) ;
    return error ;
}

