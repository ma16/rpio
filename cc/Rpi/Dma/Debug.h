// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Rpi_Dma_Debug_h_
#define _Rpi_Dma_Debug_h_

#include <Neat/BitPtr.h>
#include <cstdint> // uint32_t

namespace Rpi { namespace Dma { struct Channel ; } }

namespace Rpi { namespace Dma { struct Debug // Debug register (pp.54 + pp.57)
{
  using Lerror  = Neat::BitPtr< 0,1,true,true, uint32_t> ; Lerror   lerror() { return  Lerror(&u32) ; }
  using Ferror  = Neat::BitPtr< 1,1,true,true, uint32_t> ; Ferror   ferror() { return  Ferror(&u32) ; }
  using Rerror  = Neat::BitPtr< 2,1,true,true, uint32_t> ; Rerror   rerror() { return  Rerror(&u32) ; }
  using Writes  = Neat::BitPtr< 4,4,true,false,uint32_t> ; Writes   writes() { return  Writes(&u32) ; }
  using Id      = Neat::BitPtr< 8,8,true,false,uint32_t> ; Id           id() { return      Id(&u32) ; }
  using State   = Neat::BitPtr<16,9,true,false,uint32_t> ; State     state() { return   State(&u32) ; }
  using Version = Neat::BitPtr<25,3,true,false,uint32_t> ; Version version() { return Version(&u32) ; }
  using Lite    = Neat::BitPtr<28,1,true,false,uint32_t> ; Lite       lite() { return    Lite(&u32) ; }

  Debug() : u32(0) {}

  uint32_t value() const { return u32 ; }
    
  static uint32_t const wmask = 0x7 ;
    
private:
    
  friend Channel ;
    
  uint32_t u32 ; Debug(uint32_t u32) : u32(u32) {} // constructed by Channel
  
} ; } }

#endif // _Rpi_Dma_Debug_h_
