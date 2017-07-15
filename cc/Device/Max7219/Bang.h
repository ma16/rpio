// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef INCLUDE_Device_Max7219_Bang_h
#define INCLUDE_Device_Max7219_Bang_h

#include <vector>
#include <RpiExt/Bang.h>

namespace Device { namespace Max7219 {

struct Bang
{
    struct Pins
    {
	uint32_t din,load, clk ;
	Pins(Rpi::Pin  dinPin, Rpi::Pin loadPin, Rpi::Pin  clkPin)
	    :   din(1u<<  dinPin.value()),
	       load(1u<< loadPin.value()),
	        clk(1u<<  clkPin.value())
	    {}
    } ;

    // [todo] timing

    // [todo] record timing/data for monitoring

    // [todo] fetch error from recorded values
    
    using Script = std::vector<RpiExt::Bang::Command> ;
    
    void send(RpiExt::Bang::Enqueue *q,uint16_t data) ;
    
    void load(RpiExt::Bang::Enqueue *q) ;
    
    Bang(Rpi::Peripheral *rpi,Rpi::Pin dinPin,Rpi::Pin loadPin,Rpi::Pin clkPin)
	: rpi(rpi),pins(dinPin,loadPin,clkPin) {}

private:
  
    Rpi::Peripheral *rpi ; Pins pins ;
    
} ; } } 

#endif // INCLUDE_Device_Max7219_Bang_h
