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

    struct Error 
    {
	// verify pin values at start after reset:
	uint8_t reset_cs   : 1 ; // CS=Hi (read-back)
	uint8_t reset_clk  : 1 ; // CLK=Lo (read-back)
	uint8_t reset_din  : 1 ; // DIN=Lo (read-back)
	uint8_t reset_dout : 1 ; // DOUT=Hi 

	// verify pin value when receiving data 
	uint8_t recv_dout : 1 ; // start-bit DOUT=Lo

	// verify MSB sample with LSB sample value
	uint8_t recv_mismatch : 1 ;

	// verify pin value after transfer
	uint8_t end_dout : 1 ; // DOUT=Hi (after CS has been disabled)

	// verify whether capacitance began to bleed out
	uint8_t bled_off : 1 ; 

	uint8_t value() const
	{
	    return static_cast<uint8_t>(
		(reset_cs      << 0) |
		(reset_clk     << 1) |
		(reset_din     << 2) |
		(reset_dout    << 3) |
		(recv_dout     << 4) |
		(recv_mismatch << 5) |
		(end_dout      << 6) |
		(bled_off      << 7)) ;
	}
    } ;
    
    // verify (monitor)
    Error error(Record const &record) const ;

private:
  
    Rpi::Peripheral *rpi ;

    Pins pins ;

    Timing timing ;
    
} ; } } 

#endif // INCLUDE_Device_Mcp3008_Bang_h
