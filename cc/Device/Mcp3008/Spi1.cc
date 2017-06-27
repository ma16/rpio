// BSD 2-Clause License, see github.com/ma16/rpio

#include "Spi1.h"
#include <vector>

using namespace Device ;
using namespace Device::Mcp3008 ;
using namespace Device::Mcp3008::Circuit ;

void Spi1::setup(Rpi::Spi1 *spi,Speed speed,Cs cs,bool full)
{
    spi->setControl0(Rpi::Spi1::Flush) ;
    uint32_t c0 = full
	? 26   // request MSB and LSB samples (19-bit)
	: 17 ; // request only MSB sample (10-bit)
    c0 |= Rpi::Spi1::TxMsb ;
    c0 |= Rpi::Spi1::Enable ;
    c0 |= cs   .value() << Neat::U32::AlignRight<Rpi::Spi1::   Cs>() ;
    c0 |= speed.value() << Neat::U32::AlignRight<Rpi::Spi1::Speed>() ;
    spi->setControl0(c0) ;
    uint32_t c1 = Rpi::Spi1::RxMsb ;
    spi->setControl1(c1) ;
}

uint32_t Spi1::makeTx(Source source)
{
    uint32_t tx = 0 ;
    // start bit
    tx |= 0x10u ;
    // channel number
    tx |= source.value() ;
    // 5 bits, MSB first 
    return (tx << (32-5)) ;
}

Sample Spi1::evalRx(uint32_t rx)
{
    return Sample::coset(rx) ; 
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

int Spi1::evalRx(uint32_t rx,Sample *sample)
{
    assert(sample != nullptr) ;
    // MSB sample at pos. 18..9
    (*sample) = Sample::coset(rx >> 9) ;
    // LSB sample at pos. 0..9
    if (sample->value() != (reverse(rx) >> 22))
	return Mismatch ;
    // Zero start bit at pos. 19
    if (0 != (rx & (1u << 19)))
	return WrongStart ;
    return Success ;
}
