// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Rpi_Bus_Address_h_
#define _Rpi_Bus_Address_h_

#include "Coherency.h"

namespace Rpi { namespace Bus {

struct Address
{
  // This doesn't provide type safety since every client can create
  // an Address simply by constructing it. However, there is at least
  // some kind of reminder that we deal here with bus addresses.
  
  constexpr uint32_t value() { return u32 ; }

  void set(Coherency co) { u32 &= ~(0x3<<30) ; u32 |= co.prefix() ; }

  constexpr explicit Address(uint32_t u32) : u32(u32) {}
  // ...prevents implicit conversion so that caller must notice!

  static constexpr uint32_t Base = 0x7e000000 ;
 
private:
  
  uint32_t u32 ; 

} ;

constexpr static auto null_addr = Address(0) ;
    
} }

#endif // _Rpi_Bus_Address_h_
