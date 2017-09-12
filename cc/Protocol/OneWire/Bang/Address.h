// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef INCLUDE_Protocol_OneWire_Bang_Address_h
#define INCLUDE_Protocol_OneWire_Bang_Address_h

#include <bitset>

namespace Protocol { namespace OneWire { namespace Bang { 

using Address = std::bitset<64> ; // the "ROM code"

} } }

#endif // INCLUDE_Protocol_OneWire_Bang_Address_h
