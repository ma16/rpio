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

    // returns true it at least one device is present
    bool init() ;
    // ...subsequent functions will throw if no device is present

    // start temperature measurement on all attached devices
    void convert() ;
    // can be followed by isBusy() to wait for completion

    // returns true as long as the current operation has not finished
    bool isBusy() ;
    // only usable for convert() and as long as no new operation begins

    using Pad = std::bitset<72> ; // the scratch-pad
    
    // read scratch-pad (single drop bus only)
    Pad readPad() ;

    using Address = std::bitset<64> ; // the "ROM code"

    // get device address (single drop bus only)
    boost::optional<Address> address() ;

    // get address of first device
    boost::optional<Address> first() ;

    // get address of next device
    boost::optional<Address> next(Address const&) ;

    // ....
    
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
	, io                      (rpi)
	, busPin               (busPin)
	, pinMask(1u << busPin.value())
	, timing               (timing) {}

private:

    bool  read () ;
    void  read (size_t nbits,bool *bitA) ;
    void write (bool bit) ;
    void write (uint8_t byte) ;

    void  complete(Address      *,size_t offset) ;
    Address branch(Address const&,size_t offset) ;
    unsigned  scan(Address const&              ) ;
    void     track(Address const&,size_t  nbits) ;
    // [todo] make offset range 0..63
    
    Rpi::Peripheral *rpi ;

    RpiExt::BangIo io ;

    Rpi::Pin busPin ; uint32_t pinMask ;

    Timing<uint32_t> timing ;

} ; } } 

#endif // INCLUDE_Device_Ds18x20_Bang_h
