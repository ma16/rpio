// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Rpi_Dma_Cs_h_
#define _Rpi_Dma_Cs_h_

#include <Neat/BitPtr.h>
#include <cstdint> // uint32_t

namespace Rpi { namespace Dma { struct Channel ; } }

namespace Rpi { namespace Dma { struct Cs // Control and Status register (pp.47)
{
  using Active   = Neat::BitPtr< 0,1,true, true, uint32_t> ; Active     active() { return   Active(&u32) ; } 
  using End      = Neat::BitPtr< 1,1,false,true, uint32_t> ; End           end() { return      End(&u32) ; }
  using Intr     = Neat::BitPtr< 2,1,true, true, uint32_t> ; Intr         intr() { return     Intr(&u32) ; }
  using Dreq     = Neat::BitPtr< 3,1,true, false,uint32_t> ; Dreq         dreq() { return     Dreq(&u32) ; }
  using Paused   = Neat::BitPtr< 4,1,true, false,uint32_t> ; Paused     paused() { return   Paused(&u32) ; }
  using Stopped  = Neat::BitPtr< 5,1,true, false,uint32_t> ; Stopped   stopped() { return  Stopped(&u32) ; }
  using Waiting  = Neat::BitPtr< 6,1,true, false,uint32_t> ; Waiting   waiting() { return  Waiting(&u32) ; }
  using Error    = Neat::BitPtr< 8,1,true, false,uint32_t> ; Error       error() { return    Error(&u32) ; }
  using Priority = Neat::BitPtr<16,4,true, true, uint32_t> ; Priority priority() { return Priority(&u32) ; }
  using Panic    = Neat::BitPtr<20,4,true, true, uint32_t> ; Panic       panic() { return    Panic(&u32) ; }
  using Wait     = Neat::BitPtr<28,1,true, true, uint32_t> ; Wait         wait() { return     Wait(&u32) ; }
  using Disdebug = Neat::BitPtr<29,1,true, true, uint32_t> ; Disdebug disdebug() { return Disdebug(&u32) ; }
  using Abort    = Neat::BitPtr<30,1,false,true, uint32_t> ; Abort       abort() { return    Abort(&u32) ; }
  using Reset    = Neat::BitPtr<31,1,false,true, uint32_t> ; Reset       reset() { return    Reset(&u32) ; }

  Cs() : u32(0) {}
    
  uint32_t value() const { return u32 ; }
    
  static uint32_t const wmask = 0xf0ff0007u ; // write mask
    
private:

  friend Channel ;
    
  uint32_t u32 ; Cs(uint32_t u32) : u32(u32) {} // constructed by Channel
  
} ; } }

#endif // _Rpi_Dma_Cs_h_
