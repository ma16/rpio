#include "Crc.h"

uint8_t Neat::Bit::Crc::x31(std::vector<bool> const &v)
{
    unsigned reg = 0 ;
    for (auto i=0u ; i<v.size() ; ++i)
    {
	auto lsb = 0 != (reg & 0x01) ;
	reg >>= 1 ;
	if (v[i] != lsb)
	    reg ^= 0x8c ;
    }
    return static_cast<uint8_t>(reg) ;
}
