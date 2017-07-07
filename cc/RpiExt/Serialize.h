// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef INCLUDE_RpiExt_Serialize_h
#define INCLUDE_RpiExt_Serialize_h

#include <Rpi/Counter.h>
#include <Rpi/Gpio.h>
#include <vector>

namespace RpiExt { 

struct Serialize
{
    static double frequency(Rpi::Counter counter) ;
    
    struct Edge
    {
	Rpi::Gpio::Output output ;
	uint32_t pins ;
	uint32_t t_min ;
	uint32_t t_max ;
	Edge(
	    Rpi::Gpio::Output output,
	    uint32_t pins,
	    uint32_t t_min,
	    uint32_t t_max)
	: output(output),pins(pins),t_min(t_min),t_max(t_max) {}
    } ;

    // ...all tick values must also be <= int32_t:max (?!)
    
    Serialize(Rpi::Gpio gpio,Rpi::Counter counter) : gpio(gpio),counter(counter) {}

    bool send(std::vector<Edge> const &v) ;
    // ...the counter's prescaler must match the timing
  
private:

    Rpi::Gpio gpio ; Rpi::Counter counter ;

    bool send(uint32_t *t0,Edge const &edge) ;
    
} ; } 

#endif // INCLUDE_RpiExt_Serialize_h
