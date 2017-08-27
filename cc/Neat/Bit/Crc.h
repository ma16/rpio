// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef INCLUDE_Neat_Bit_Crc_h
#define INCLUDE_Neat_Bit_Crc_h

#include <vector>
#include <cinttypes>

namespace Neat { namespace Bit {

namespace Crc	
{
    
    uint8_t x31(std::vector<bool> const &v) ;
    // v's bit:0 is MSB; polynom and result are reversed
    
} } }

#endif // INCLUDE_Neat_Bit_Crc_h
