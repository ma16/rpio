// BSD 2-Clause License, see github.com/ma16/rpio

#include "Timing.h"

using namespace Device::Ds18x20::Bang ;

constexpr OneWire::Timing::Template<double> OneWire::Timing::spec ;

OneWire::Timing::Template<uint32_t>
OneWire::Timing::xlat(Template<double> const &seconds,double tps)
{
    Template<uint32_t> ticks ;
    auto f = [tps](double s) { return static_cast<uint32_t>(s * tps + 0.5) ; } ;
    
    ticks.slot.min = f(seconds.slot.min) ;
    ticks.slot.max = f(seconds.slot.max) ;

    ticks.rec = f(seconds.rec) ;
    
    ticks.low0.min = f(seconds.low0.min) ;
    ticks.low0.max = f(seconds.low0.max) ;

    ticks.low1.min = f(seconds.low1.min) ;
    ticks.low1.max = f(seconds.low1.max) ;

    ticks.rinit = f(seconds.rinit) ;
    ticks.rrc   = f(seconds.  rrc) ;
    ticks.rdv   = f(seconds.  rdv) ;
    ticks.rsth  = f(seconds. rsth) ;
    ticks.rstl  = f(seconds. rstl) ;
    
    ticks.pdhigh.min = f(seconds.pdhigh.min) ;
    ticks.pdhigh.max = f(seconds.pdhigh.max) ;

    ticks.pdlow.min = f(seconds.pdlow.min) ;
    ticks.pdlow.max = f(seconds.pdlow.max) ;

    return ticks ;
}
