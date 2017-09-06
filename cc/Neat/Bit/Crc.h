// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef INCLUDE_Neat_Bit_Crc_h
#define INCLUDE_Neat_Bit_Crc_h

#include <bitset>
#include <vector>
#include <cinttypes>

namespace Neat { namespace Bit {

namespace Crc	
{
    template<size_t N> inline uint8_t x31(std::bitset<N> const &set)
    {
	unsigned reg = 0 ;
	for (auto i=0u ; i<set.size() ; ++i)
	{
	    auto lsb = 0 != (reg & 0x01) ;
	    reg >>= 1 ;
	    if (set[i] != lsb)
		reg ^= 0x8c ;
	}
	return static_cast<uint8_t>(reg) ;
    }
    
    uint8_t x31(std::vector<bool> const &v) ;
    // v's bit:0 is MSB; polynom and result are reversed
    
} } }

#endif // INCLUDE_Neat_Bit_Crc_h
