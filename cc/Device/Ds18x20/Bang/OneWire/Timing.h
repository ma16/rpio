// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef INCLUDE_Device_Ds18x20_Bang_OneWire_Timing_h
#define INCLUDE_Device_Ds18x20_Bang_OneWire_Timing_h

#include <cstdint>

namespace Device { namespace Ds18x20 { namespace Bang { namespace OneWire {

struct Timing
{
    template<typename T> struct Template
    {
	struct Range
	{
	    T min ;
	    T max ;
	} ;

	Range slot ;   // Time Slot
	T rec ;        // Minimum Recovery Time
	Range low0 ;   // Write 0 Low Time
	Range low1 ;   // Write 1 Low Time
	T rinit ;      // minimum low time to initiate Read Time Slot
	T rrc ;        // RC time after releasing the bus to return to High
	T rdv ;        // Maximum Read Data Valid
	T rsth ;       // Minimum Reset Time High (Rx/Open Collector)
	T rstl ;       // Minimum Reset Time Low (Actual Low Time)
	Range pdhigh ; // Presence-Detect High (period for HL edge)
	Range pdlow ;  // Presence-Detect Low (period for LH edge)
    } ;
    
    static constexpr Template<double> spec =
    {
	{ 60e-6,120e-6 },
	1e-6,
	{ 60e-6,120e-6 },
	{  1e-6, 15e-6 },
	1e-6,
	1e-6,
	15e-6,
	480e-6,
	480e-6,
	{  15e-6, 60e-6 },
	{  60e-6,240e-6 },
    } ;

    static Template<uint32_t> xlat(Template<double> const &seconds,double tps) ;
    
} ; } } } }

#endif // INCLUDE_Device_Ds18x20_Bang_OneWire_Timing_h
