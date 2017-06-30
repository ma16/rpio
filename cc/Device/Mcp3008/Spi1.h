// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef INCLUDE_Device_Mcp3008_Spi1_h
#define INCLUDE_Device_Mcp3008_Spi1_h

#include "Circuit.h"
#include <Neat/uint.h>
#include <Rpi/Spi1.h>

namespace Device { namespace Mcp3008 {

struct Spi1
{
    struct Error 
    {
        Error() : head(0),mismatch(0) {} 
	
	uint8_t head      : 1 ; // invalid DOUT start sequence
	uint8_t mismatch  : 1 ; // MSB vs LSB mismatch

	Neat::uint<unsigned,2> code() const
	{
	    auto code = (head << 0) | (mismatch << 1) ;
	    return Neat::uint<unsigned,2>::coset((unsigned)code) ;
	}

	bool ok() const { return head == 0 && mismatch == 0 ; }
    } ;
    
    struct Sample17
    {
	uint32_t i ;
	Sample17() : i(0) {}
	Sample17(uint32_t i) : i(i) {}
	Error verify() const ;
	Circuit::Sample fetch() const ;
    } ;
    Sample17 query17(Circuit::Source) ;
  
    struct Sample26
    {
	uint32_t i ;
	Sample26() : i(0) {}
	Sample26(uint32_t i) : i(i) {}
	Error verify() const ;
	Circuit::Sample fetch() const ;
    } ;
    Sample26 query26(Circuit::Source) ;
  
    Spi1(Rpi::Peripheral *rpi) : spi(Rpi::Spi1(rpi)) {}
    
private:

    Rpi::Spi1 spi ;
} ;

} } 

#endif // INCLUDE_Device_Mcp3008_Spi1_h
