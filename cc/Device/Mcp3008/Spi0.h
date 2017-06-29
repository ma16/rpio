// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef INCLUDE_Device_Mcp3008_Spi0_h
#define INCLUDE_Device_Mcp3008_Spi0_h

#include "Circuit.h"
#include <Neat/uint.h>
#include <Rpi/Spi0.h>

namespace Device { namespace Mcp3008 {

struct Spi0
{
    struct Error 
    {
        Error() : head(0),mismatch(0),tail(0) {} 
	
	uint8_t head      : 1 ; // invalid DOUT start sequence
	uint8_t mismatch  : 1 ; // MSB vs LSB mismatch
	uint8_t tail      : 1 ; // invalid DOUT remainder

	Neat::uint<unsigned,3> code() const
	{
	    auto code = (head << 0) | (mismatch << 1) | (tail << 2) ;
	    return Neat::uint<unsigned,3>::coset((unsigned)code) ;
	}

	bool ok() const { return head == 0 && mismatch == 0 && tail == 0 ; }
    } ;
    
    struct Sample24
    {
	uint32_t i ;
	Sample24() : i(0) {}
	Sample24(uint32_t i) : i(i) {}
	Error verify() const ;
	Circuit::Sample fetch() const ;
    } ;
    Sample24 query24(Circuit::Source) ;

    struct Sample32
    {
	uint32_t i ;
	Sample32() : i(0) {}
	Sample32(uint32_t i) : i(i) {}
	Error verify() const ;
	Circuit::Sample fetch() const ;
    } ;
    Sample32 query32(Circuit::Source) ;

    Spi0(Rpi::Peripheral *rpi) : spi(rpi) {}

private:
  
    Rpi::Spi0 spi ;

} ; } } 

#endif // INCLUDE_Device_Mcp3008_Spi0_h
