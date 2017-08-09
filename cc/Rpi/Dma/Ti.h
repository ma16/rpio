// BSD 2-Clause License, see github.com/ma16/rpio

// Transfer Information register (datasheet pp.50 & pp.55)

#ifndef INCLUDE_Rpi_Dma_Ti_h
#define INCLUDE_Rpi_Dma_Ti_h

#include <Neat/Bit/Word.h>
#include <Neat/BitPtr.h>
#include <cstdint> // uint32_t

namespace Rpi { namespace Dma {

namespace Ti
{
    using Word = Neat::Bit::Word<uint32_t,0x07fffffb> ;
    
    using Inten        = Word::Set< 0,1> ;
    using Tdmode       = Word::Set< 1,1> ;
    using WaitResp     = Word::Set< 3,1> ;
    using DestInc      = Word::Set< 4,1> ;
    using DestWidth    = Word::Set< 5,1> ;
    using DestDreq     = Word::Set< 6,1> ;
    using DestIgnore   = Word::Set< 7,1> ;
    using SrcInc       = Word::Set< 8,1> ;
    using SrcWidth     = Word::Set< 9,1> ;
    using SrcDreq      = Word::Set<10,1> ;
    using SrcIgnore    = Word::Set<11,1> ;
    using BurstLength  = Word::Set<12,4> ;
    using Permap       = Word::Set<16,5> ;
    using Waits        = Word::Set<21,5> ;
    using NoWideBursts = Word::Set<26,1> ;

    static inline Word make(Permap permap)
    {
	Word w ;
	w = Inten       ::make<0>() ; // we can't use interrupts in userland
	w = Tdmode      ::make<0>() ; // no 2d transfer
	w = WaitResp    ::make<0>() ; // +++ can be overwritten by client +++
	w = DestInc     ::make<0>() ; // destination is peripheral address
	w = DestWidth   ::make<0>() ; // destination is peripheral address
	w = DestDreq    ::make<1>() ; // paced by destination peripheral
	w = DestIgnore  ::make<0>() ; // destination is peripheral address
	w = SrcInc      ::make<1>() ; // source is memory block
	w = SrcWidth    ::make<0>() ; // +++ can be overwritten by client +++
	w = SrcDreq     ::make<0>() ; // source is memory block
	w = SrcIgnore   ::make<0>() ; // source is memory block
	w = BurstLength ::make<0>() ; // +++ can be overwritten by client +++
	w = permap ; // e.g. 5 for PWM
	w = Waits       ::make<0>() ; // +++ can be overwritten by client +++
	w = NoWideBursts::make<0>() ; // +++ can be overwritten by client +++
	return w ;
    }
} } }

#endif // INCLUDE_Rpi_Dma_Ti_h
