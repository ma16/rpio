// BSD 2-Clause License, see github.com/ma16/rpio

// see Pwm.md for details

#ifndef INCLUDE_Rpi_Pwm_h
#define INCLUDE_Rpi_Pwm_h

#include "Bus/Address.h"
#include "Dma/Ti.h" // Permap for DMA pacing
#include "Peripheral.h"
#include <Neat/Bit/Word.h>
#include <Neat/Enum.h>

namespace Rpi {

struct Pwm 
{
    using Index = Neat::Enum<unsigned,1> ; // channel index

    struct Control
    {
	static constexpr uint32_t Mask = 0xbfffu ;

	using Word = Neat::Bit::Word<uint32_t,Mask> ; 
	
	static constexpr auto Pwen1 = Word::Digit::make< 0>() ; 
	static constexpr auto Mode1 = Word::Digit::make< 1>() ; 
	static constexpr auto Rptl1 = Word::Digit::make< 2>() ; 
	static constexpr auto Sbit1 = Word::Digit::make< 3>() ; 
	static constexpr auto Pola1 = Word::Digit::make< 4>() ; 
	static constexpr auto Usef1 = Word::Digit::make< 5>() ; 
	static constexpr auto Clrf  = Word::Digit::make< 6>() ; 
	static constexpr auto Msen1 = Word::Digit::make< 7>() ; 
	static constexpr auto Pwen2 = Word::Digit::make< 8>() ; 
	static constexpr auto Mode2 = Word::Digit::make< 9>() ; 
	static constexpr auto Rptl2 = Word::Digit::make<10>() ; 
	static constexpr auto Sbit2 = Word::Digit::make<11>() ; 	
	static constexpr auto Pola2 = Word::Digit::make<12>() ; 
	static constexpr auto Usef2 = Word::Digit::make<13>() ; 
	static constexpr auto Msen2 = Word::Digit::make<15>() ;
	
	struct Bank
	{
	    Word::Digit mode ;
	    Word::Digit msen ;
	    Word::Digit pola ;
	    Word::Digit pwen ;
	    Word::Digit rptl ;
	    Word::Digit sbit ;
	    Word::Digit usef ;
	    static Bank const& select(Index) ;
	} ;
	
	Word read() const { return Word::coset(*p) ; }
	
	void write(Word w) { (*p) = w.value() ; }

    private:
	
	friend Pwm ; uint32_t volatile *p ;
	
	Control(uint32_t volatile *p) : p(p) {}
    } ;
	    
    Control control() { return & page->at<0x0/4>() ; }
    
    struct Status 
    {
	static constexpr auto Mask = 0x73f ;
	
	using Word = Neat::Bit::Word<uint32_t,Mask> ;

	static constexpr auto Full = Word::Digit::make< 0>() ; 
	static constexpr auto Empt = Word::Digit::make< 1>() ; 
	static constexpr auto Werr = Word::Digit::make< 2>() ; 
	static constexpr auto Rerr = Word::Digit::make< 3>() ; 
	static constexpr auto Gap1 = Word::Digit::make< 4>() ; 
	static constexpr auto Gap2 = Word::Digit::make< 5>() ; 
	static constexpr auto Berr = Word::Digit::make< 8>() ; 
	static constexpr auto Sta1 = Word::Digit::make< 9>() ; 
	static constexpr auto Sta2 = Word::Digit::make<10>() ; 

	struct Bank
	{
	    Word::Digit gap ;
	    Word::Digit sta ;
	    static Bank const& select(Index) ;
	} ;
	
	Word read() const { return Word::coset(*p) ; }

	void clear(Word w) { (*p) = w.value() ; }

    private:
	
	friend Pwm ; uint32_t volatile *p ;

	Status(uint32_t volatile *p) : p(p) {}
    } ;

    Status status() { return & page->at<0x4/4>() ; }

    // Write word to 16-word-deep FIFO; you should make sure:
    // --either beforhand that not full
    // --or afterwards that no write error has occurred
    //
    // When both channels are enabled for FIFO usage, then:
    // (1) the data is shared between these channels in turn. For example
    //     with the word series A B C D E F G H, first channel will use
    //     A C E G and second channel will use B D F H.
    // (2) The range register must have the same value for both channels.
    // (3) The spec says that RPTLi is not meaningful as there is no
    //     defined channel to own the last data in the FIFO. Therefore
    //     both RPTL must be set to zero. (Observation: doesn't need to)
    // (4) If the configuration has changed in any of the two channels,
    //     FIFO should be cleared before writing new data.
    void write(uint32_t d)
    {
	// fifo with 16 x 32-bit words
	// check status.full1 before write...
	this->page->at<0x18/4>() = d ;
	// ...or check status.werr1 to see whether succeeded
    }

    uint32_t read() 
    {
	// only to check if it has an impact on the RERR flag (it has not)
	return this->page->at<0x18/4>() ;
	// seems to return always 0x70776d30 which spells "pwm0"
    }

    static constexpr auto fifoAddr = Bus::Address(0x7e20c018) ;
    // ...FIFO address for DMA transfers
    static constexpr Dma::Ti::Permap::Uint permap()
    { return Dma::Ti::Permap::Uint::make<5>() ; }
    // ...peripheral pacing mapping for DMA transfers
    // note that gcc struggles with...
    // static constexpr auto permap = Dma::Ti::Permap::Uint::make<5>() ;
  
    // define the number of clock-pulses that define a period;
    // if in serialization mode (mode=1):
    //   if range<32: only the most significant data bits are put out
    //   if range>32: (range-32) additional padding bits are put out
    // [defect] if USEF=1 range=1: still two bits are transferred (LSB twice?)
    // [defect] if USEF=1 range=0: strange effects
    //    * first single write to FIFO enables STA, but FIFO remains empty
    //    * more than two writes in a row start to fill FIFO
    uint32_t getRange(Index i) const ; void setRange(Index i,uint32_t r) ; 
  
    // pwm mode (mode=0): the number of pulses within a period (range)
    //   coherent (msen=0) : evenly distributed
    //   none     (msen=1) : sequentially in one row
    // serialization mode (mode=1) : the actual bits to put out
    //   period<32: clipped, only most significant bits
    //   period>32: expanded by (period-32) sbit values
    uint32_t getData(Index i) const ; void setData(Index i ,uint32_t d) ; 

    struct Dmac // DMA control word
    {
	bool  enable  ; // enable control of DMA signals
	uint8_t panic ; // set panic bits if beyond this threshold (default 7)
	uint8_t dreq  ; // activate dreq signal if beyond this threshold (default 7)
	Dmac(bool enable,uint8_t panic,uint8_t dreq)
	    : enable(enable),panic(panic),dreq(dreq) {}
    } ;
    Dmac getDmac() const ; void setDmac(Dmac) ; 
    // the dreq field lacks some documentation, from observation (32 words):
    // dreq= 9: 0..15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
    // dreq=10: 0..15, * 17,18,19,20,21,22,23, * 25,26,27,28,29,30,31
    // dreq=11: 0..15, *  * 18,19,20,21,22,23, *  * 26,27,28,29,30,31
    // dreq=12: 0..15, *  * 18,19,20,21,22, *  *  * 26,27,28,29,30, *
    // dreq=13: 0..15, *  * 18,19,20,21, *  *  *  * 26,27,28,29, *  *
    // dreq=14: 0..15, *  * 18,19,20  *  *  *  *  * 26,27,28, *  *  *
    // dreq=15: 0..15, *  * 18,19, *  *  *  *  *  * 26,27, *  *  *  *
    // (*) missing values because of writing into a full FIFO queue; this
    // appears to be reproducible at different frequencies
    // usage of --ti-wait-resp prevents the gaps (at the expense of speed)
  
    Pwm(Peripheral *p) : page(p->page(Peripheral::PNo::make<0x20c>())) {}
  
private:
  
    std::shared_ptr<Page> page ; 

} ; }

#endif // INCLUDE_Rpi_Pwm_h
