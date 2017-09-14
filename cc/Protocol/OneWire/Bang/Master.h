// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef INCLUDE_Protocol_OneWire_Bang_Master_h
#define INCLUDE_Protocol_OneWire_Bang_Master_h

#include "Timing.h"
#include <bitset>
#include <Rpi/Intr.h>
#include <RpiExt/BangIo.h>

namespace Protocol { namespace OneWire { namespace Bang { 

struct Master
{
    static constexpr auto AlarmSearch = std::bitset<8>(0xec) ;
    static constexpr auto MatchRom    = std::bitset<8>(0x55) ; 
    static constexpr auto ReadRom     = std::bitset<8>(0x33) ; 
    static constexpr auto SearchRom   = std::bitset<8>(0xf0) ; 
    static constexpr auto SkipRom     = std::bitset<8>(0xcc) ; 
    
    Master(
	Rpi::Peripheral *rpi,
	Rpi::Pin pin,
	Timing::Template<uint32_t> const& timing)

	: intr              (rpi)
	, io                (rpi)
	, pin               (pin)
	, mask(1u << pin.value())
	, timing         (timing) {}

private:

    friend class Signaling ;

    Rpi::Intr intr ;

    RpiExt::BangIo io ;

    Rpi::Pin pin ; uint32_t mask ;

    Timing::Template<uint32_t> timing ;

} ; } } }

#endif // INCLUDE_Protocol_OneWire_Bang_Master_h
