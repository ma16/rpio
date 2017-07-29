// BSD 2-Clause License, see github.com/ma16/rpio

// --------------------------------------------------------------------
// See Pulse Width Modulator (§9) 
// https://www.raspberrypi.org/app/uploads/2012/02/BCM2835-ARM-Peripherals.pdf
//
// Errata:
//
// p.138: "read data from a FIFO storage block, which can store up to
//   eight 32-bit words."
// The FIFO holds 16 32-bit words. So, if only one channel is used, all
// 16 words make up a "block".
//
// p.138: "Both modes clocked by clk_pwm which is nominally 100MHz"
// The "nominal" clock seems to be zero since it is not set-up.
//
// p.143: CLRF1 is marked as RO (read-only).
// It is WO (write-only).
//
// p.143: RPTL "0:Transmission interrupts when FIFO is empty"
// In non-DMA mode: when FIFO gets empty, the last word is repeated
// regardless whether this bit is set or not. Even if the FIFO is
// cleared (CLRF). If the serializer starts with a cleared FIFO there
// is nothing to repeat.
//
// p.143: SBIT "Defines the state of the output when no transmission
//   takes place "
// That is only true:
//   * if mode=0 & msen=1 & sbit=1
//   * if mode=1 & range>32 (for bits 32..)
//
// p.143: USEF
// [defect] channel #2 seems not always to work in FIFO-mode. Sometimes
// no transmission occurs (in serialization mode) even if the FIFO is
// full and the channel is busy. The FIFO remains full. [open issue]
// 
// 
// --------------------------------------------------------------------

#ifndef INCLUDE_Rpi_Pwm_h
#define INCLUDE_Rpi_Pwm_h

#include "Bus/Address.h"
#include "Dma/Ti.h"
#include <Neat/Bit.h>
#include "Peripheral.h"

namespace Rpi {

struct Pwm 
{
    enum : uint32_t
    {
	Pwen1 = (1u <<  0), // 1=enable transmission (0=disable)
	Pwen2 = (1u <<  8),
	Mode1 = (1u <<  1), // 1=as serializer (0:as PWM)
	Mode2 = (1u <<  9),
	Rptl1 = (1u <<  2), // 1=repeat last word
	Rptl2 = (1u << 10), // (only effective if Usef=1)
	Sbit1 = (1u <<  3), // 1=silence bit is High (0=Low)
	Sbit2 = (1u << 11), 
	Pola1 = (1u <<  4), // 1=inverse output polarity (0=don't)
	Pola2 = (1u << 12), 
	Usef1 = (1u <<  5), // 1=use FIFO (0=use Data register)
	Usef2 = (1u << 13), 
	Clrf  = (1u <<  6), // 1=clear FIFO (single shot)
	Msen1 = (1u <<  7), // 1=mark-space PWM (0=coherent)
	Msen2 = (1u << 15), // (has only effect if Mode=PWM)
    } ;
	    
    // static uint32_t const wmask = 0x0bfffu ; // write mask
    
    using Index = Neat::Enum<unsigned,1> ;

    struct Control
    {
	Control(uint32_t w) ;

	uint32_t value() const ;

	struct Channel
	{
	    uint8_t pwen : 1 ;
	    uint8_t mode : 1 ;
	    uint8_t rptl : 1 ;
	    uint8_t sbit : 1 ;
	    uint8_t pola : 1 ;
	    uint8_t usef : 1 ;
	    uint8_t msen : 1 ;
	    Channel() : pwen(0),mode(0),rptl(0),sbit(0),pola(0),usef(0),msen(0) {}
	} ;

	Channel channel[2] ; // [todo] channel(Index) <- make array class
	bool clear ;
	
	Control getControl() const ; void set(Control) ;

    private:

	uint32_t w ;
    } ;

    Control getControl() const
    {
	return Control(page->at<0/4>()) ;
    } 

    void setControl(Control c)
    {
	page->at<0/4>() = c.value() ;
    }
  
    struct Status // 32-bit status word
    {
	// FULL:  FIFO is full
	using Full  = Neat::Bit< 0,uint32_t> ;
	Full   full() { return  Full(&u32) ; }
	uint32_t  cfull() const { return  Full::mask & u32 ; }
	
	// EMPT:  FIFO is empty
	using Empt  = Neat::Bit< 1,uint32_t> ;
	Empt   empt() { return  Empt(&u32) ; }
	uint32_t  cempt() const { return  Empt::mask & u32 ; }
	
	// WERR:  write FIFO -- but FIFO full
	using Werr  = Neat::Bit< 2,uint32_t> ;
	Werr   werr() { return  Werr(&u32) ; }
	uint32_t  cwerr() const { return  Werr::mask & u32 ; }
	
	// RERR:  read FIFO -- but FIFO empty
	using Rerr  = Neat::Bit< 3,uint32_t> ;
	Rerr   rerr() { return  Rerr(&u32) ; }
	uint32_t  crerr() const { return  Rerr::mask & u32 ; }
	
	// GAPO1: channel 1 (index 1) gap-occurred-flag
	using Gapo1 = Neat::Bit< 4,uint32_t> ;
	Gapo1 gapo1() { return Gapo1(&u32) ; }
	uint32_t cgapo1() const { return Gapo1::mask & u32 ; }
	
	// GAPO2: channel 2 (index 1) gap-occurred-flag
	using Gapo2 = Neat::Bit< 5,uint32_t> ;
	Gapo2 gapo2() { return Gapo2(&u32) ; }
	uint32_t cgapo2() const { return Gapo2::mask & u32 ; }
	
	// BERR:  bus error has occurred while writing to registers via APB
	using Berr  = Neat::Bit< 8,uint32_t> ;
	Berr   berr() { return  Berr(&u32) ; }
	uint32_t  cberr() const { return  Berr::mask & u32 ; }
	
	// STA1:  channel (index 0) is transmitting
	using Sta1  = Neat::Bit< 9,uint32_t> ;
	Sta1   sta1() { return  Sta1(&u32) ; }
	uint32_t  csta1() const { return  Sta1::mask & u32 ; }
	// [defect] the channel may continue transmission even
	// if PWEN was reset (observed when BERR=1). Reset will
	// only reset BERR, but the channel continues transmission
	// (STA=1). A write with PWEN=0 will stop transmission.
	// [info] when PWM is enabled (FIFO mode) and FIFO is empty
	// then STA remains unset until data is put into the FIFO
	
	// [defect] if USEF=1 and PWEN after FIFO clear and status reset,
	// STA remains unset until data is put into the FIFO 
	
	// STA2:  channel (index 1) is transmitting
	using Sta2  = Neat::Bit<10,uint32_t> ;
	Sta2   sta2() { return  Sta2(&u32) ; }
	uint32_t  csta2() const { return  Sta2::mask & u32 ; }

	// [observed] Full,Empt,Sta1,Sta2 are writable (resetable) according
	// to the peripheral spec. which however makes not much sense, does it?
    
	Status() : u32(0) {}

	// [todo] how to reset a certain flag, e.g. werr??

	uint32_t value() const { return u32 ; }

	static uint32_t const wmask = 0x73fu ; // write mask
    
    private:

	friend Pwm ;
    
	uint32_t u32 ; Status(uint32_t u32) : u32(u32) {}
    } ;
  
    Status getStatus() const { return page->at<0x4/4>() ; }
  
    void resetStatus(Status s) { page->at<0x4/4>() = s.value() & s.wmask ; }

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
    // The base-address is not listed in the datasheet
    // see instead: http://elinux.org/BCM2835_datasheet_errata#p141
  
private:
  
    std::shared_ptr<Page> page ; 

} ; }

#endif // INCLUDE_Rpi_Pwm_h
