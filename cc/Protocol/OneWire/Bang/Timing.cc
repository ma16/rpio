// BSD 2-Clause License, see github.com/ma16/rpio

#include "Timing.h"

using namespace Protocol::OneWire::Bang ;

Timing::Template<double> Timing::specified() 
{
    Template<double> t ;
	
    t.resetPulse_min    = 480e-6 ;
    t.presenceIdle_min  =  15e-6 ;
    t.presenceIdle_max  =  60e-6 ;
    t.presencePulse_min =  60e-6 ;
    t.presencePulse_max = 240e-6 ;
    t.presenceFrame_max = 480e-6 ;

    t.slot_min =  60e-6 ;
    t.slot_max = 120e-6 ;
    t.init_min =   1e-6 ;
    t.init_max = 160e-6 ; // a guess (1/2 reset pulse didn't work)
    t.rc_max   =   1e-6 ; // a guess
    t.rdv_min  =  15e-6 ;
    t.rec_min  =   1e-6 ;

    t.write_0_min =  60e-6 ;
    t.write_0_max = 120e-6 ;
    t.write_1_min =   1e-6 ;
    t.write_1_max =  15e-6 ;

    return t ;
} 

Timing::Template<uint32_t>
Timing::xlat(double f,Template<double> const &seconds)
{
    Template<uint32_t> ticks ;
    auto c = [f](double s) { return static_cast<uint32_t>(s * f + 0.5) ; } ;
    
    ticks.resetPulse_min = c(seconds.resetPulse_min) ;
    ticks.presenceIdle_min = c(seconds.presenceIdle_min) ;
    ticks.presenceIdle_max = c(seconds.presenceIdle_max) ;
    ticks.presencePulse_min = c(seconds.presencePulse_min) ;
    ticks.presencePulse_max = c(seconds.presencePulse_max) ;
    ticks.presenceFrame_max = c(seconds.presenceFrame_max) ;
    
    ticks.slot_min = c(seconds.slot_min) ;
    ticks.slot_max = c(seconds.slot_max) ;
    ticks.init_min = c(seconds.init_min) ;
    ticks.init_max = c(seconds.init_max) ;
    ticks.rc_max = c(seconds.rc_max) ;
    ticks.rdv_min = c(seconds.rdv_min) ;
    ticks.rec_min = c(seconds.rec_min) ;
    
    ticks.write_0_min = c(seconds.write_0_min) ;
    ticks.write_0_max = c(seconds.write_0_max) ;
    ticks.write_1_min = c(seconds.write_1_min) ;
    ticks.write_1_max = c(seconds.write_1_max) ;

    return ticks ;
}
