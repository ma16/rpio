// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef INCLUDE_Device_Ds18x20_Bang_h
#define INCLUDE_Device_Ds18x20_Bang_h

#include <Neat/Bit/Crc.h>
#include <RpiExt/BangIo.h>

namespace Device { namespace Ds18x20 {

struct Bang
{
    struct Error : Neat::Error
    {
	Error(std::string const &m) : Neat::Error("Device:Ds18x20:Bang:" + m) {}
    } ;
    
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
	Range rinit ; // minimum low time to initiate Read Time Slot
	T rinitgap ; // max gap fpr rinit HL edge
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
	{  1e-6, 15e-6 },
	{  1e-6,  5e-6 },
	3e-6,
	15e-6,
	480e-6,
	480e-6,
	{  15e-6, 60e-6 },
	{  60e-6,240e-6 },
    } ;

    static Timing<uint32_t> ticks(Timing<double> const &seconds,double tps) ;
    
    // assumes
    // * busPin.mode=Input (changed between input and output)
    // * busPin.outputLevel = Low (not changed)

    void convert  (RpiExt::BangIo*) const ;
    void readPad  (RpiExt::BangIo*,bool(*)[72]) const ;
    void readRom  (RpiExt::BangIo*,bool(*rx)[64]) const ;
    void firstRom (RpiExt::BangIo*,bool(*rx)[64]) const ;
    bool nextRom  (RpiExt::BangIo*,bool const(*prev)[64],bool(*next)[64]) const ;
    bool isBusy   (RpiExt::BangIo*) const ;
    
    static uint8_t crc(std::vector<bool> const &v)
    {
	return Neat::Bit::Crc::x31(v) ;
    }
    
    Bang(
	Rpi::Peripheral *rpi,
	Rpi::Pin busPin,
	Timing<uint32_t> const& timing = ticks(spec,250e+6))

	: rpi                     (rpi)
	, busPin               (busPin)
	, pinMask(1u << busPin.value())
	, timing               (timing) {}

private:

    void  init(RpiExt::BangIo*) const ;
    bool  read(RpiExt::BangIo*) const ;
    void  read(RpiExt::BangIo*,size_t nbits,bool *bitA) const ;
    void  scan(RpiExt::BangIo*,size_t offset,bool(*rx)[64]) const ;
    void write(RpiExt::BangIo*,bool bit) const ;
    void write(RpiExt::BangIo*,uint8_t byte) const ;
    
    Rpi::Peripheral *rpi ;

    Rpi::Pin busPin ; uint32_t pinMask ;

    Timing<uint32_t> timing ;

} ; } } 

#endif // INCLUDE_Device_Ds18x20_Bang_h
