// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Rpi_Pwm_h_
#define _Rpi_Pwm_h_

#include "Bus/Address.h"
#include "Dma/Ti.h"
#include <Neat/Bit.h>
#include "Peripheral.h"

namespace Rpi { struct Pwm 
{
  using Index = Neat::Enum<unsigned,1> ;

  struct Control
  {
    // PWEN: 1:enable channel (0:disable)
    // MODE: 1:serializer mode (0:pwm)
    // RPTL: spec. says: "1:repeat last data when FIFO is empty"
    //       [defect]: last data is always repeated
    //       when FIFO is empty, regardless whether set or not
    // SBIT: spec. says: silence bit is put out
    //       [observed]: that's true...
    //       --if mode=0 & msen=1 & sbit=1
    //       --or mode=1 & range>32 for the (range-32) tail bits
    // POLA: 1:inverse output polarity (0:don't)
    // USEF: 1:use Fifo (0:use data register)
    //       [defect]: using channel 2 on FIFO doesn't work always:
    //       i.e. in serialization mode, even though the FIFO is full
    //       and the channel is busy the FIFO remains full and no
    //       data is put out [todo] create a defect report
    // MSEN: 1:coherent pwm (0:not) ; only for mode=0
    
    using Pwen1 = Neat::Bit< 0,uint32_t> ; Pwen1 pwen1() { return Pwen1(&u32) ; } uint32_t cpwen1() const { return Pwen1::mask & u32 ; }
    using Mode1 = Neat::Bit< 1,uint32_t> ; Mode1 mode1() { return Mode1(&u32) ; } uint32_t cmode1() const { return Mode1::mask & u32 ; }
    using Rptl1 = Neat::Bit< 2,uint32_t> ; Rptl1 rptl1() { return Rptl1(&u32) ; } uint32_t crptl1() const { return Rptl1::mask & u32 ; }
    using Sbit1 = Neat::Bit< 3,uint32_t> ; Sbit1 sbit1() { return Sbit1(&u32) ; } uint32_t csbit1() const { return Sbit1::mask & u32 ; }
    using Pola1 = Neat::Bit< 4,uint32_t> ; Pola1 pola1() { return Pola1(&u32) ; } uint32_t cpola1() const { return Pola1::mask & u32 ; }
    using Usef1 = Neat::Bit< 5,uint32_t> ; Usef1 usef1() { return Usef1(&u32) ; } uint32_t cusef1() const { return Usef1::mask & u32 ; }
    using Clrf1 = Neat::Bit< 6,uint32_t> ; Clrf1 clrf1() { return Clrf1(&u32) ; } uint32_t cclrf1() const { return Clrf1::mask & u32 ; }
    using Msen1 = Neat::Bit< 7,uint32_t> ; Msen1 msen1() { return Msen1(&u32) ; } uint32_t cmsen1() const { return Msen1::mask & u32 ; }
    using Pwen2 = Neat::Bit< 8,uint32_t> ; Pwen2 pwen2() { return Pwen2(&u32) ; } uint32_t cpwen2() const { return Pwen2::mask & u32 ; }
    using Mode2 = Neat::Bit< 9,uint32_t> ; Mode2 mode2() { return Mode2(&u32) ; } uint32_t cmode2() const { return Mode2::mask & u32 ; }
    using Rptl2 = Neat::Bit<10,uint32_t> ; Rptl2 rptl2() { return Rptl2(&u32) ; } uint32_t crptl2() const { return Rptl2::mask & u32 ; }
    using Sbit2 = Neat::Bit<11,uint32_t> ; Sbit2 sbit2() { return Sbit2(&u32) ; } uint32_t csbit2() const { return Sbit2::mask & u32 ; }
    using Pola2 = Neat::Bit<12,uint32_t> ; Pola2 pola2() { return Pola2(&u32) ; } uint32_t cpola2() const { return Pola2::mask & u32 ; }
    using Usef2 = Neat::Bit<13,uint32_t> ; Usef2 usef2() { return Usef2(&u32) ; } uint32_t cusef2() const { return Usef2::mask & u32 ; }
    using Msen2 = Neat::Bit<15,uint32_t> ; Msen2 msen2() { return Msen2(&u32) ; } uint32_t cmsen2() const { return Msen2::mask & u32 ; }
    
    Control() : u32(0) {}

    uint32_t value() const { return u32 ; }
    
    static uint32_t const wmask = 0x0bfffu ; // write mask
    
    struct Channel
    {
      uint32_t pwen : 1 ;
      uint32_t mode : 1 ;
      uint32_t rptl : 1 ;
      uint32_t sbit : 1 ;
      uint32_t pola : 1 ;
      uint32_t usef : 1 ;
      uint32_t msen : 1 ;
      Channel() : pwen(0),mode(0),rptl(0),sbit(0),pola(0),usef(0),msen(0) {}
    } ;

    Channel get(Index) const ; void set(Index,Channel) ;
    
  private:

    friend Pwm ;
    
    uint32_t u32 ; Control(uint32_t u32) : u32(u32) {} 
  
  } ; 
  
  Control getControl() const { return page->at<0/4>() ; } ;

  void setControl(Control c) { page->at<0/4>() = c.value() & c.wmask ; }

  struct Status
  {
    // FULL:  FIFO is full
    // EMPT:  FIFO is empty
    // WERR:  write FIFO -- but FIFO full
    // RERR:  read FIFO -- but FIFO empty
    // GAPO1: channel 1 (index 1) gap-occurred-flag
    // GAPO2: channel 2 (index 1) gap-occurred-flag
    // BERR:  bus error has occurred while writing to registers via APB
    // STA1:  channel (index 0) is transmitting
    // STA2:  channel (index 1) is transmitting

    using Full  = Neat::Bit< 0,uint32_t> ; Full   full() { return  Full(&u32) ; } uint32_t  cfull() const { return  Full::mask & u32 ; }
    using Empt  = Neat::Bit< 1,uint32_t> ; Empt   empt() { return  Empt(&u32) ; } uint32_t  cempt() const { return  Empt::mask & u32 ; }
    using Werr  = Neat::Bit< 2,uint32_t> ; Werr   werr() { return  Werr(&u32) ; } uint32_t  cwerr() const { return  Werr::mask & u32 ; }
    using Rerr  = Neat::Bit< 3,uint32_t> ; Rerr   rerr() { return  Rerr(&u32) ; } uint32_t  crerr() const { return  Rerr::mask & u32 ; }
    using Gapo1 = Neat::Bit< 4,uint32_t> ; Gapo1 gapo1() { return Gapo1(&u32) ; } uint32_t cgapo1() const { return Gapo1::mask & u32 ; }
    using Gapo2 = Neat::Bit< 5,uint32_t> ; Gapo2 gapo2() { return Gapo2(&u32) ; } uint32_t cgapo2() const { return Gapo2::mask & u32 ; }
    using Berr  = Neat::Bit< 8,uint32_t> ; Berr   berr() { return  Berr(&u32) ; } uint32_t  cberr() const { return  Berr::mask & u32 ; }
    using Sta1  = Neat::Bit< 9,uint32_t> ; Sta1   sta1() { return  Sta1(&u32) ; } uint32_t  csta1() const { return  Sta1::mask & u32 ; }
    using Sta2  = Neat::Bit<10,uint32_t> ; Sta2   sta2() { return  Sta2(&u32) ; } uint32_t  csta2() const { return  Sta2::mask & u32 ; }

    // [observed] Full,Empt,Sta1,Sta2 are writable (resetable) according
    // to the peripheral spec. which however makes not much sense, does it?
    
    Status() : u32(0) {}

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
  uint32_t getRange(Index i) const ; void setRange(Index i,uint32_t r) ; 
  
  // pwm mode (mode=0): the number of pulses within a period (range)
  //   coherent (msen=0) : evenly distributed
  //   none     (msen=1) : sequentially in one row
  // serialization mode (mode=1) : the actual bits to put out
  //   period<32: clipped, only most significant bits
  //   period>32: expanded by (period-32) sbit values
  uint32_t getData(Index i) const ; void setData(Index i ,uint32_t d) ; 

  struct Dmac // DMA Control
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

#endif // _Rpi_Pwm_h_
