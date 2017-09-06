// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef INCLUDE_Device_Ds18x20_Bang_h
#define INCLUDE_Device_Ds18x20_Bang_h

#include <Neat/Bit/Crc.h>
#include <RpiExt/BangIo.h>
#include <bitset>
#include <boost/optional.hpp>

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

    
    void convert (RpiExt::BangIo*) const ;
    void readPad (RpiExt::BangIo*,bool(*)[72]) const ;
    bool  isBusy (RpiExt::BangIo*) const ;

    using Address = std::bitset<64> ; // the "ROM code"

    boost::optional<Address> address(RpiExt::BangIo*) const ;
    
    boost::optional<Address> first(RpiExt::BangIo*) const ;
    boost::optional<Address> next(RpiExt::BangIo*,Address const &prev) const ;
    
    static uint8_t crc(std::vector<bool> const &v)
    {
	return Neat::Bit::Crc::x31(v) ;
    }

    template<size_t N> static uint8_t crc(std::bitset<N> const &set)
    {
	return Neat::Bit::Crc::x31(set) ;
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

    bool  init(RpiExt::BangIo*) const ;
    bool  read(RpiExt::BangIo*) const ;
    void  read(RpiExt::BangIo*,size_t nbits,bool *bitA) const ;
    void write(RpiExt::BangIo*,bool bit) const ;
    void write(RpiExt::BangIo*,uint8_t byte) const ;

    void complete(RpiExt::BangIo*,size_t offset,Address*) const ;

    unsigned /* 0..64 */ scan(RpiExt::BangIo*,Address const&) const ;
    
    
    Rpi::Peripheral *rpi ;

    Rpi::Pin busPin ; uint32_t pinMask ;

    Timing<uint32_t> timing ;

} ; } } 

#endif // INCLUDE_Device_Ds18x20_Bang_h
