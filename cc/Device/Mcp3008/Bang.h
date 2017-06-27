// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef INCLUDE_Device_Mcp3008_Bang_h
#define INCLUDE_Device_Mcp3008_Bang_h

#include "Circuit.h"
#include <Rpi/Counter.h>
#include <Rpi/Gpio.h>
#include <RpiExt/Bang.h>

namespace Device { namespace Mcp3008 {

struct Bang
{
    using Timing = Circuit::Timing<uint32_t> ;

    Bang(
	Rpi::Peripheral *rpi,
	Rpi::Pin       csPin,
	Rpi::Pin      clkPin,
	Rpi::Pin      dinPin,
	Rpi::Pin     doutPin,
	Timing const &timing) ;

    struct Pins
    {
	uint32_t   cs ;
	uint32_t  clk ;
	uint32_t  din ;
	uint32_t dout ;
	Pins(
	    Rpi::Pin   csPin,
	    Rpi::Pin  clkPin,
	    Rpi::Pin  dinPin,
	    Rpi::Pin doutPin)
	    :    cs(1u<<  csPin.value()),
	        clk(1u<< clkPin.value()),
	        din(1u<< dinPin.value()),
	       dout(1u<<doutPin.value())
	    {}
    } ;

    struct Record
    {
	// saved pin values
	uint32_t resetLevel ; // cs/clk/din/dout = 1/0/0/1 (monitor)
	uint32_t startLevel ; // dout = 0 (monitor)
	uint32_t sample[19] ; // sampled data (dout) ([10..18] monitor)
	uint32_t   endLevel ; // dout = 1 (monitor)
	// check if capacitor bled out (monitor)
	uint32_t t_start ; 
	uint32_t   t_end ; 
	// temporary timer values (monitor)
	uint32_t t0 ;
	uint32_t t1 ;
    } ;

    using Script = std::vector<RpiExt::Bang::Command> ;
    
    Script makeScript(Circuit::Source source,bool monitor,Record *record) ;
    
    Record query(Circuit::Source,bool monitor) ;

    // record.sample[0..9]
    Circuit::Sample msb(Record const &record) const ;
    
    // record.sample[9..18] (monitor)
    Circuit::Sample lsb(Record const &record) const ;

    // verify (monitor)
    Circuit::Error error(Record const &record) const ;

private:
  
    Rpi::Peripheral *rpi ;

    Pins pins ;

    Timing timing ;
    
} ; } } 

#endif // INCLUDE_Device_Mcp3008_Bang_h
