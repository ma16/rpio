// BSD 2-Clause License, see github.com/ma16/rpio

// --------------------------------------------------------------------
// See Pulse Width Modulator (§9) 
// https://www.raspberrypi.org/app/uploads/2012/02/BCM2835-ARM-Peripherals.pdf
//
// Errata: (see also http://elinux.org/BCM2835_datasheet_errata)
//
// p.138: "read data from a FIFO storage block, which can store up to
//   eight 32-bit words."
// The FIFO holds 16 32-bit words. So, if only one channel is used, all
// 16 words make up a "block".
//
// p.138: "Both modes clocked by clk_pwm which is nominally 100MHz"
// The "nominal" clock seems to be zero since it is not set-up.
//
// p.141
// The base-address (0x7e20:c000) for the register-block is missing.
//
// p.143: CLRF1 is marked as RO (read-only).
// It is WO (write-only / reads-zero).
//
// p.143: RPTL=0: "Transmission interrupts when FIFO is empty"
// For none-DMA mode: when FIFO gets empty, the last word is repeated
// regardless whether this bit is set or not. Even if the FIFO is
// cleared (CLRF). However, there is nothing to repeat if the
// serializer starts with a cleared FIFO .
//
// p.143: SBIT "Defines the state of the output when no transmission
//   takes place "
// That is only true:
//   * if mode=0 & msen=1 & sbit=1
//   * if mode=1 & range>32 (for the 33rd "bit" and following)
//
// p.143: USEF
// [defect] channel #2 seems not always to work properly in FIFO-mode.
// "Sometimes" no transmission occurs (in serialization mode) even if
// the FIFO is full and STA=1. The FIFO simply stays full. [open issue]
//
// p.144: "BERR sets to high when an error has occurred while writing
//   to registers via APB. This may happen if the bus tries to write
//   successively to same set of registers faster than the synchroniser
//   block can cope with. Multiple switching may occur and contaminate 
//   the data during synchronisation.
// This kind of problem can be observed when writing twice in a row to
// the Control register. Since the effects are unpredictable,
// application developers should check for BERR after each write, and
// abort if set. (so try to prevent BERR; e.g. with a read-cyle.)
// 
// p.144: STA "1 means channel is transmitting data."
// For USEF=1 & RPT=1: if PWEN is enabled on an empty FIFO then STA
// is set immediately.
// For USEF=1 & RPT=0: if PWEN is enabled on an empty FIFO then STA
// remains cleared until a word is written to the FIFO (or RPT is set).
// [defect] STA may remain set; even if PWEN was cleared. This can be
// observed for operations that cause BERR=1. In order to clear STA,
// BERR needs to be cleared first, thereafter PWEN.
//
// p.144: "RERR1 bit sets to high when a read when empty error occurs."
// There is no explanation under which circumstances this may happen.
// Reading the FIFO by application (CPU) simply returns "pwm0" regard-
// less of the FIFO contents; RERR1 is not set when reading from an
// empty FIFO
//
// p.145: EMPT1,FULL1 are marked as RW (read-write)
// Since writing has no effect, ist should be RO (read-only)
//
// p.145: EMPT1
// The serializer may still be busy with the transfer even if the FIFO
// is empty.
// --------------------------------------------------------------------

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
    using Index = Neat::Enum<unsigned,1> ;

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
	
	// Clrf  : 1=clear FIFO (single shot)
	// Mode# : 1=as serializer (0=as PWM)
	// Msen# : 1=mark-space-mode (0=coherent); only effective if Mode=PWM
	// Pola# : 1=inverse output polarity (0=don't)
	// Pwen# : 1=enable transmission (0=disable)
	// Rptl# : 1=repeat last word; only effective if Usef=1
	// Sbit# : 1=silence bit is High (0=Low)
	// Usef# : 1=use FIFO (0=use Data register)

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

	static constexpr auto Full = Word::Digit::make< 0>() ; // FIFO
	static constexpr auto Empt = Word::Digit::make< 1>() ; // FIFO
	static constexpr auto Werr = Word::Digit::make< 2>() ; // FIFO
	static constexpr auto Rerr = Word::Digit::make< 3>() ; // FIFO
	static constexpr auto Gap1 = Word::Digit::make< 4>() ; // Gap
	static constexpr auto Gap2 = Word::Digit::make< 5>() ; // Gap
	static constexpr auto Berr = Word::Digit::make< 8>() ; // Bus Error
	static constexpr auto Sta1 = Word::Digit::make< 9>() ; // Status
	static constexpr auto Sta2 = Word::Digit::make<10>() ; // Status

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
