// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef INCLUDE_Device_Ds18x20_Bang_h
#define INCLUDE_Device_Ds18x20_Bang_h

#include <Neat/Bit/Crc.h>
#include <RpiExt/Bang.h>

namespace Device { namespace Ds18x20 {

struct Bang
{
    using Script = std::vector<RpiExt::Bang::Command> ;

    template<typename T> struct Timing
    {
	struct Range
	{
	    T min ;
	    T max ;
	} ;

	Range slot ; // Time Slot
	T rec ; // Minimum Recovery Time
	Range low0 ; // Write 0 Low Time
	Range low1 ; // Write 1 Low Time
	T rinit ; // minimum low time to initiate Read Time Slot
	T rrc ; // maximum recovery time after read initiation
	T rdv ; // Maximum Read Data Valid
	T rsth ; // Minimum Reset Time High (Rx/Open Collector)
	T rstl ; // Minimum Reset Time Low (Actual Low Time)
	Range pdhigh ; // Presence-Detect High (period for HL edge)
	Range pdlow ; // Presence-Detect Low (period for LH edge)
    } ;
    
    // If for any reason a transaction needs to be suspended, the bus
    // must be left in the idle state if the transaction is to resume.
    // Infinite recovery time can occur between bits so long as the bus
    // is idle during the recovery period.
    
    static constexpr Timing<double> spec =
    {
	{ 60e-6,120e-6 },
	1e-6,
	{ 60e-6,120e-6 },
	{ 1e-6, 15e-6 },
	1e-6,
	5e-6, // a guess?!
	15e-6,
	480e-6,
	480e-6,
	{ 15e-6,60e-6 },
	{ 60e-6,240e-6 },
    } ;

    static Timing<uint32_t> ticks(Timing<double> const &seconds,double tps) ;
    
    // assumes
    // * busPin.mode=Input (changed between input and output)
    // * busPin.outputLevel = Low (not changed)

    using Stack = RpiExt::Bang::Stack ;
    
    Script convert(Stack*) const ;
    Script readPad(Stack*,uint32_t(*)[72]) const ;
    Script readRom(Stack*,uint32_t(*)[64]) const ;
    
    static void pack(uint32_t const from[],size_t nwords,uint32_t mask,char to[]) ;
    
    static uint8_t crc(std::vector<bool> const &v)
    {
	return Neat::Bit::Crc::x31(v) ;
    }
    
    Bang(
	Rpi::Peripheral *rpi,
	Rpi::Pin busPin,
	Timing<uint32_t> const& timing = ticks(spec,250e+6))
	: rpi(rpi),busPin(busPin),timing(timing) {}

private:

    using Enqueue = RpiExt::Bang::Enqueue ;
    
    void  init(Enqueue*,Stack*) const ;
    void  read(Enqueue*,Stack*,uint32_t *levels) const ;
    void  read(Enqueue*,Stack*,size_t nwords,uint32_t *levels) const ;
    void write(Enqueue*,Stack*,bool bit) const ;
    void write(Enqueue*,Stack*,uint8_t byte) const ;
    
    Rpi::Peripheral *rpi ; Rpi::Pin busPin ; Timing<uint32_t> timing ;

} ; } } 

#endif // INCLUDE_Device_Ds18x20_Bang_h
 
