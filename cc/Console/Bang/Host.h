// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Console_Bang_Host_h_
#define _Console_Bang_Host_h_

#include <Rpi/Counter.h>
#include <Rpi/Gpio.h>
#include <vector>

namespace Console { namespace Bang {

struct Host
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
    
    Host(Rpi::Gpio gpio,Rpi::Counter counter) : gpio(gpio),counter(counter) {}

    bool send(std::vector<Edge> const &v) ;
    // ...the counter's prescaler must match the timing
  
private:

    Rpi::Gpio gpio ; Rpi::Counter counter ;

    bool send(uint32_t *t0,Edge const &edge) ;
    
} ; } } 

#endif // _Console_Bang_Host_h_
