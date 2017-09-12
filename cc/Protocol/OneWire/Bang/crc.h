// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef INCLUDE_Protocol_OneWire_Bang_crc_h
#define INCLUDE_Protocol_OneWire_Bang_crc_h

#include <Neat/Bit/Crc.h>

namespace Protocol { namespace OneWire { namespace Bang { 

template<size_t N> static inline uint8_t crc(std::bitset<N> const &set)
{
    return Neat::Bit::Crc::x31(set) ;
}

} } }

#endif // INCLUDE_Protocol_OneWire_Bang_crc_h
