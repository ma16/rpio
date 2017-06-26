// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Rpi_Dma_Ti_h_
#define _Rpi_Dma_Ti_h_

#include <Neat/BitPtr.h>
#include <cstdint> // uint32_t

namespace Rpi { namespace Dma { struct Channel ; } }

namespace Rpi { namespace Dma { struct Ti // Transfer Information register (pp.50 + pp.55 for Lite)
{
  using Inten        = Neat::BitPtr< 0,1,true, true, uint32_t> ; Inten               inten() { return        Inten(&u32) ; } 
  using Tdmode       = Neat::BitPtr< 1,1,true, true, uint32_t> ; Tdmode             tdmode() { return       Tdmode(&u32) ; } 
  using WaitResp     = Neat::BitPtr< 3,1,true, true, uint32_t> ; WaitResp         waitResp() { return     WaitResp(&u32) ; } 
  using DestInc      = Neat::BitPtr< 4,1,true, true, uint32_t> ; DestInc           destInc() { return      DestInc(&u32) ; } 
  using DestWidth    = Neat::BitPtr< 5,1,true, true, uint32_t> ; DestWidth       destWidth() { return    DestWidth(&u32) ; } 
  using DestDreq     = Neat::BitPtr< 6,1,true, true, uint32_t> ; DestDreq         destDreq() { return     DestDreq(&u32) ; } 
  using DestIgnore   = Neat::BitPtr< 7,1,true, true, uint32_t> ; DestIgnore     destIgnore() { return   DestIgnore(&u32) ; } 
  using SrcInc       = Neat::BitPtr< 8,1,true, true, uint32_t> ; SrcInc             srcInc() { return       SrcInc(&u32) ; } 
  using SrcWidth     = Neat::BitPtr< 9,1,true, true, uint32_t> ; SrcWidth         srcWidth() { return     SrcWidth(&u32) ; } 
  using SrcDreq      = Neat::BitPtr<10,1,true, true, uint32_t> ; SrcDreq           srcDreq() { return      SrcDreq(&u32) ; } 
  using SrcIgnore    = Neat::BitPtr<11,1,true, true, uint32_t> ; SrcIgnore       srcIgnore() { return    SrcIgnore(&u32) ; } 
  using BurstLength  = Neat::BitPtr<12,4,true, true, uint32_t> ; BurstLength   burstLength() { return  BurstLength(&u32) ; } 
  using Permap       = Neat::BitPtr<16,5,true, true, uint32_t> ; Permap             permap() { return       Permap(&u32) ; } 
  using Waits        = Neat::BitPtr<21,5,true, true, uint32_t> ; Waits               waits() { return        Waits(&u32) ; } 
  using NoWideBursts = Neat::BitPtr<26,1,true, true, uint32_t> ; NoWideBursts noWideBursts() { return NoWideBursts(&u32) ; } 

  Ti() : u32(0) {}

  static Ti send(Permap::Uint permap)
  {
    Ti ti ;
    ti.       inten()   =  false ; // we can't use interrupts in userland
    ti.      tdmode()   =  false ; // no 2d transfer
    ti.    waitResp()   =  false ; // +++ can be overwritten by client +++
    ti.     destInc()   =  false ; // destination is peripheral address
    ti.   destWidth()   =  false ; // destination is peripheral address
    ti.    destDreq()   =   true ; // paced by destination peripheral
    ti.  destIgnore()   =  false ; // destination is peripheral address
    ti.      srcInc()   =   true ; // source is memory block
    ti.    srcWidth()   =  false ; // +++ can be overwritten by client +++
    ti.     srcDreq()   =  false ; // paced by destination peripheral
    ti.   srcIgnore()   =  false ; // source is memory block
    ti. burstLength()   = BurstLength::Uint::make<0>() ;
                                   // +++ can be overwritten by client +++
    ti.      permap()   = permap ; // e.g. 5 for PWM
    ti.       waits()   = Waits::Uint::make<0>() ;
                                   // +++ can be overwritten by client +++
    ti.noWideBursts() =    false ; // +++ can be overwritten by client +++
    return ti ;
  }
  

  uint32_t value() const { return u32 ; }
    
  static uint32_t const wmask = 0x07fffffbu ; // write mask
    
private:

  friend Channel ;
    
  uint32_t u32 ; Ti(uint32_t u32) : u32(u32) {} // constructed by Channel
  
} ; } }

#endif // _Rpi_Dma_Ti_h_
