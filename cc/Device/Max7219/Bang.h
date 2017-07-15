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

    template<typename T> struct Timing
    {
	T   ch ; // CLK high-level pulse-width
	T   cl ; // CLK low-level pulse-width
	T  csw ; // LOAD high-level pulse-width
	T   ds ; // DIN setup to CLK rising-edge
	T ldck ; // LOAD rising-edge to next CLK rising-edge

	Timing(T ch,T cl,T csw,T ds,T ldck)
	    : ch(ch),cl(cl),csw(csw),ds(ds),ldck(ldck) {}
    } ;

    using Seconds = Timing<float> ;
    
    static Seconds strict() // from datasheet
    { return Seconds( 50e-9f,50e-9f,50e-9f,25e-9f,25e-9f ) ; }
    
    using Ticks = Timing<uint32_t> ;

    static Ticks asTicks(Seconds const &s,double freq) ;

    void send(RpiExt::Bang::Enqueue *q,uint16_t data) ;
    
    void load(RpiExt::Bang::Enqueue *q) ;
    
    Bang(
	Rpi::Peripheral *rpi,
	Rpi::Pin dinPin,Rpi::Pin loadPin,Rpi::Pin clkPin,
	Ticks const &ticks)
	: rpi(rpi),pins(dinPin,loadPin,clkPin),ticks(ticks) {}

private:
  
    Rpi::Peripheral *rpi ; Pins pins ; Ticks ticks ;
    
} ; } } 

#endif // INCLUDE_Device_Max7219_Bang_h
