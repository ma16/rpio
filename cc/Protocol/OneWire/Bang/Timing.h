// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef INCLUDE_Protocol_OneWire_Bang_Timing_h
#define INCLUDE_Protocol_OneWire_Bang_Timing_h

#include <cstdint>

namespace Protocol { namespace OneWire { namespace Bang { 

struct Timing
{
    template<typename T> struct Template
    {
	// (1) Initialization-Sequence
	
	// reset-pulse period
	T resetPulse_min ;

	// gap between LH-edge of reset-pulse and HL-edge of presence-pulse
	T presenceIdle_min ; 
	T presenceIdle_max ;

	// presence-pulse period
	T presencePulse_min ;
	T presencePulse_max ;

	// presence-frame period
	T presenceFrame_max ; 

	// (2) Read-Time-Slot
	
	// time-slot period
	T slot_min ; T slot_max ;
	
        // read-time-slots's Low signal to trigger Slave
	T init_min ;
	// ...the Slave must pull the bus Low within this time

	// the maximum Low time that is not considered as Reset-Pulse
	T init_max ;
	
	// RC recovery-time to return to High after releasing the bus
	T rc_max ; 

	// read-data valid-period (starting at Master's Low trigger)
	T rdv_min ;
	// ...the Slave must pull the bus Low for this period to send a 1-bit

	// idle bus period (recover) after time-slot's LH edge
	T rec_min ;
	
	// (3) Write-Time-Slot
	
	// write-0-pulse's Low period
	T write_0_min ; T write_0_max ;

	// write-1-pulse's Low period
	T write_1_min ; T write_1_max ;
    } ;
    
    static Template<double> specified() ;

    static Template<uint32_t> xlat(
	double f,Template<double> const &seconds = specified()) ;
    
} ; } } }

#endif // INCLUDE_Protocol_OneWire_Bang_Timing_h
