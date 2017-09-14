// BSD 2-Clause License, see github.com/ma16/rpio

#include "Timing.h"

using namespace Protocol::OneWire::Bang ;

constexpr Timing::Template<double> Timing::spec ;

Timing::Template<uint32_t>
Timing::xlat(double f,Template<double> const &seconds)
{
    Template<uint32_t> ticks ;
    auto c = [f](double s) { return static_cast<uint32_t>(s * f + 0.5) ; } ;
    
    ticks.slot.min = c(seconds.slot.min) ;
    ticks.slot.max = c(seconds.slot.max) ;

    ticks.rec = c(seconds.rec) ;
    
    ticks.low0.min = c(seconds.low0.min) ;
    ticks.low0.max = c(seconds.low0.max) ;

    ticks.low1.min = c(seconds.low1.min) ;
    ticks.low1.max = c(seconds.low1.max) ;

    ticks.rinit = c(seconds.rinit) ;
    ticks.rrc   = c(seconds.  rrc) ;
    ticks.rdv   = c(seconds.  rdv) ;
    ticks.rsth  = c(seconds. rsth) ;
    ticks.rstl  = c(seconds. rstl) ;
    
    ticks.pdhigh.min = c(seconds.pdhigh.min) ;
    ticks.pdhigh.max = c(seconds.pdhigh.max) ;

    ticks.pdlow.min = c(seconds.pdlow.min) ;
    ticks.pdlow.max = c(seconds.pdlow.max) ;

    return ticks ;
}
