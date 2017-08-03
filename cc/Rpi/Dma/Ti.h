// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Rpi_Dma_Ti_h_
#define _Rpi_Dma_Ti_h_

#include <Neat/BitPtr.h>
#include <cstdint> // uint32_t

namespace Rpi { namespace Dma { struct Channel ; } }

namespace Rpi { namespace Dma {

struct Ti // Transfer Information register (pp.50 + pp.55 for Lite)
{
    using Inten        = Neat::BitPtr< 0,1,true, true, uint32_t> ;
    using Tdmode       = Neat::BitPtr< 1,1,true, true, uint32_t> ;
    using WaitResp     = Neat::BitPtr< 3,1,true, true, uint32_t> ;
    using DestInc      = Neat::BitPtr< 4,1,true, true, uint32_t> ;
    using DestWidth    = Neat::BitPtr< 5,1,true, true, uint32_t> ;
    using DestDreq     = Neat::BitPtr< 6,1,true, true, uint32_t> ;
    using DestIgnore   = Neat::BitPtr< 7,1,true, true, uint32_t> ;
    using SrcInc       = Neat::BitPtr< 8,1,true, true, uint32_t> ;
    using SrcWidth     = Neat::BitPtr< 9,1,true, true, uint32_t> ;
    using SrcDreq      = Neat::BitPtr<10,1,true, true, uint32_t> ;
    using SrcIgnore    = Neat::BitPtr<11,1,true, true, uint32_t> ;
    using BurstLength  = Neat::BitPtr<12,4,true, true, uint32_t> ;
    using Permap       = Neat::BitPtr<16,5,true, true, uint32_t> ;
    using Waits        = Neat::BitPtr<21,5,true, true, uint32_t> ;
    using NoWideBursts = Neat::BitPtr<26,1,true, true, uint32_t> ;
    
    Inten               inten() { return        Inten(&u32) ; } 
    Tdmode             tdmode() { return       Tdmode(&u32) ; } 
    WaitResp         waitResp() { return     WaitResp(&u32) ; } 
    DestInc           destInc() { return      DestInc(&u32) ; } 
    DestWidth       destWidth() { return    DestWidth(&u32) ; } 
    DestDreq         destDreq() { return     DestDreq(&u32) ; } 
    DestIgnore     destIgnore() { return   DestIgnore(&u32) ; } 
    SrcInc             srcInc() { return       SrcInc(&u32) ; } 
    SrcWidth         srcWidth() { return     SrcWidth(&u32) ; } 
    SrcDreq           srcDreq() { return      SrcDreq(&u32) ; } 
    SrcIgnore       srcIgnore() { return    SrcIgnore(&u32) ; } 
    BurstLength   burstLength() { return  BurstLength(&u32) ; } 
    Permap             permap() { return       Permap(&u32) ; } 
    Waits               waits() { return        Waits(&u32) ; } 
    NoWideBursts noWideBursts() { return NoWideBursts(&u32) ; } 

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
