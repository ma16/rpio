// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef INCLUDE_Device_Ds18x20_Bang_OneWire_Master_h
#define INCLUDE_Device_Ds18x20_Bang_OneWire_Master_h

#include "Timing.h"
#include <bitset>
#include <RpiExt/BangIo.h>
#include <Rpi/Intr.h>

namespace Device { namespace Ds18x20 { namespace Bang { namespace OneWire {

struct Master
{
    // reset the bus and return the presence pulse (if any)
    bool init() ;

    // read time-slot
    bool read() ;

    // write time-slot
    void write(bool bit) ;
    
    Master(
	Rpi::Peripheral *rpi,
	Rpi::Pin busPin,
	Timing::Template<uint32_t> const& timing =
	Timing::xlat(Timing::spec,250e+6)) // [todo]

	: intr                    (rpi)
	, io                      (rpi)
	, busPin               (busPin)
	, pinMask(1u << busPin.value())
	, timing               (timing) {}

    template<size_t N> std::bitset<N> read()
    {
	std::bitset<N> set(0) ;
	for (size_t i=0 ; i<N ; ++i)
	    set[i] = this->read() ;
	return set ;
    }
    
    template<size_t N> void write(std::bitset<N> const &set) 
    {
	for (size_t i=0 ; i<N ; ++i)
	    this->write(set[i]) ;
    }
    
    static constexpr auto AlarmSearch = std::bitset<8>(0xec) ;
    static constexpr auto MatchRom    = std::bitset<8>(0x55) ; 
    static constexpr auto ReadRom     = std::bitset<8>(0x33) ; 
    static constexpr auto SearchRom   = std::bitset<8>(0xf0) ; 
    static constexpr auto SkipRom     = std::bitset<8>(0xcc) ; 
    
private:

    Rpi::Intr intr ;

    RpiExt::BangIo io ;

    Rpi::Pin busPin ; uint32_t pinMask ;

    Timing::Template<uint32_t> timing ;

} ; } } } }

#endif // INCLUDE_Device_Ds18x20_Bang_OneWire_Master_h
