// BSD 2-Clause License, see github.com/ma16/rpio

// This is a simple serializer. That is, a vector of (Level,Min,Low)-
// tuples is serialized.
//
// * A level can be high or low.
// * The signal level is hold at least for a minimum amount of time.
// * A flag is set if the signal level lasts for more then a maximum
//   amount of time.
//
// All timing values are considered to match ARM counter ticks.
//
//   __   ________________   __
//     \ /                \ /
//      x                  x
//   __/ \________________/ \__
//
//   t0   t1             t2  t3
//
// * The time t0 is taken immediately before the new level is set.
// * The signal level is set (causing a rising of falling edge).
// * The time t1 is taken immediately after the signal was set.
// * The time t2 is taken (repeatedly) until (t2-t1) >= min.
// * The signal level is set (causing an inverse edge).
// * The time t3 is taken immediately after the signal was set.
// * An error flag is raised if (t3 - t0) > max
//
// For the next tuple, the timer values are re-used: t0=t2,t1=t3.
//
// A bit-banged operation may fail any time due to process suspension.
// Besides, cache faults and interrupts may occur. The caller may try
// again from the beginning, if a maximum timing was exceeded. Anyway,
// with this implementation, signal levels of less than 1us appear to
// be diffcult to achieve.

#ifndef INCLUDE_RpiExt_Serialize_h
#define INCLUDE_RpiExt_Serialize_h

#include <Rpi/ArmTimer.h>
#include <Rpi/Gpio/Output.h>
#include <vector>

namespace RpiExt { 

struct Serialize
{
    struct Edge
    {
	Rpi::Gpio::Output::Level level ;
	uint32_t pins ;
	uint32_t t_min ;
	uint32_t t_max ;
	Edge(uint32_t pins,
	     uint32_t t_min,uint32_t t_max,
	     Rpi::Gpio::Output::Level level)
	: level(level),pins(pins),t_min(t_min),t_max(t_max) {}
    } ;

    Serialize(Rpi::Gpio::Output output,Rpi::ArmTimer timer)
    : output(output),timer(timer) {}

    bool send(std::vector<Edge> const &v) ;
  
private:

    Rpi::Gpio::Output output ; Rpi::ArmTimer timer ;

    bool send(uint32_t *t0,Edge const &edge) ;
    
} ; } 

#endif // INCLUDE_RpiExt_Serialize_h
