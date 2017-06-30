// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Rpi_Spi1_h_
#define _Rpi_Spi1_h_

// --------------------------------------------------------------------
//
// Universal Serial Peripheral Interface (SPI) Master
//
// See BCM2835 ARM Peripherals: chapter 2.3
//
// See also the errate at
// http://elinux.org/BCM2835_datasheet_errata
//
// --------------------------------------------------------------------
//
// Short Description
//
// The device employs a 32-bit (de)serializer together with a 4-word
// FIFO each to queue MOSI requests and MISO responses.
//
//  Client      | -> FIFO ->   serializer -> MOSI -> | Slave
//  Application | <- FIFO <- deserializer <- MISO <- | Device
//
// Words that read from/written to FIFO are always 32-bit wide. The
// actual width to be transferred can be configured. Either statically
// (32-bit or less) or dynamically within the word (24-bit or less).
//
// There is always a gap of two clock-cycles between the transmission
// of two subsequent words (whatever size they have). SCLK remains low
// in this period.
//
// --------------------------------------------------------------------
//
// Notes
//
// TxMsb=0: Transmit LSB-First
//   The serializer takes Bit:0 and does a right-shift (31..1 -> 30..0)
//
// TxMsb=1: Transmit MSB-First
//   The serializer takes Bit:31 and does a left-shift (31..1 <- 30..0)
//
// RxMsb=0: Receive LSB-First
//   The serializer does a right-shift (31..1 -> 30..0) and sets Bit:31
//
// RxMsb=1: Receive MSB-First
//   The serializer does a left-shift (31..1 <- 30..0) and sets Bit:0
//
// --------------------------------------------------------------------
//
// Errata: (figured out by tests, no reference yet)
//
// p.25: Status Register:
//   Bit:7      is set if rx-fifo is empty
//   Bit:8      is set if rx-fifo is full
//   Bit:9      is set if tx-fifo is empty
//   Bit:10     is set if tx-fifo is full
//   Bit:20-22  number of words in the rx-fifo (0..4)
//   Bit:28-30  number of words in the tx-fifo (0..4)
//
//   Writes to the register have no effect.
//
// pp.26: Data words are always 32-bit for FIFO read/write (not 16-bit).
//
// --------------------------------------------------------------------
//
// [todo] the device needs to be enabled before it can be used
//
// --------------------------------------------------------------------

#include "Peripheral.h"
#include <Neat/Enum.h>
#include <deque>
#include <vector>

namespace Rpi { struct Spi1 
{
  Spi1(Peripheral *p) : page(p->page(Peripheral::PNo::make<0x215>())) {}

  // ------------------------------------------------------------------
  
  enum : uint32_t
  {
    Len     = ( 0x3fu <<  0), // number of bits/word to transfer (see TxLen)
    TxMsb   = (  0x1u <<  6), // tx: send serializer's word with MSB first
    //          ...starting with bit:23 if TxLen; else starting with bit:31
    IdleClk = (  0x1u <<  7), // SCLK level when idle
    TxLh    = (  0x1u <<  8), // transmit MOSI data on rising (LH) SCLK edge
    //           ...in case the slave samples with the falling (HL) SCLK edge
    Flush   = (  0x1u <<  9), // flush rx- & tx-fifo
    RxLh    = (  0x1u << 10), // receive MISO data on rising (LH) SCLK edge
    Enable  = (  0x1u << 11), // enable transfer
    TxHold  = (  0x3u << 12), // extra MOSI (hold) cycles
    TxLen   = (  0x1u << 14), // use tx-word (bit:24-28) instead of Len
    TxCs    = (  0x1u << 15), // use tx-word (bit:29-31) instead of Cs
    RxPost  = (  0x1u << 16), // deserialize in post-input mode (delayed by 1 cycle)
    Cs      = (  0x7u << 17), // pin level for all three chip-select lines (see CsLen)
    Cs0     = (  0x1u << 17), // pin level to activate chip-select (see CsLen)
    Cs1     = (  0x1u << 18), // pin level to activate chip-select (see CsLen)
    Cs2     = (  0x1u << 19), // pin level to activate chip-select (see CsLen)
    Speed   = (0xfffu << 20), // SCLK speed
  } ;
  // [todo] static_assert(Mask == (Set< Len<Mask>() >() << Ofs<Mask>()),"bit-mask not contiguous") ;
  // [todo] make sure not overlapping
  // [todo] make sure only valid for get/set control #0 register
  
  uint32_t getControl0() const
  {
    return this->page->at<0x80/4>() ;
  }

  void setControl0(uint32_t i)
  {
    this->page->at<0x80/4>() = i ;
  }
  
  // ------------------------------------------------------------------
  
  enum : uint32_t
  {
    RxKeep  = (0x1u << 0), // don't reset (de)serializer register
    RxMsb   = (0x1u << 1), // rx-fifo: MSB first from (de)serializer
    IrqIdle = (0x1u << 6), // request interrupt if idle
    IrqFifo = (0x1u << 7), // request interrupt if tx-fifo is empty
    CsHold  = (0x7u << 8), // extra number of cycles to extend CS-high
  } ;

  uint32_t getControl1() const
  {
    return this->page->at<0x84/4>() ; // [todo] set mask
  }

  void setControl1(uint32_t i)
  {
    this->page->at<0x84/4>() = i ; // [todo] set mask
  }
  
  // ------------------------------------------------------------------
  
  enum : uint32_t
  {
    Left    = (0x3fu <<  0), // left bits in serializer (0..32)
    Busy    = ( 0x1u <<  6), // serializer busy or !TxEmpty
    RxEmpty = ( 0x1u <<  7), // rx: fifo is empty
    RxFull  = ( 0x1u <<  8), // rx: fifo is full
    TxEmpty = ( 0x1u <<  9), // tx: fifo is empty
    TxFull  = ( 0x1u << 10), // tx: fifo is full
    RxSize  = ( 0x7u << 20), // rx: fifo's number of words (0..4)
    TxSize  = ( 0x7u << 28), // tx: fifo's number of words (0..4)
  } ;

  uint32_t status() const
  {
    return this->page->at<0x88/4>() ; // [todo] set mask
  }

  // ------------------------------------------------------------------

  uint32_t peek() const
  {
    return static_cast<uint32_t>(this->page->at<0x8c/4>()) ;
  }

  uint32_t read() const
  {
    return static_cast<uint32_t>(this->page->at<0xa0/4>()) ;
    // note: reading from 0xa4, 0xa8, 0xac, 0xb0, 0xb4, 0xb8 or 0xbc
    //       has the same effect as reading from 0xa0
  }

  void writeEnd(uint32_t word)
  {
    this->page->at<0xa0/4>() = word ; 
    // note: writing to 0xa4, 0xa8 or 0xac has the same effect
  }
  
  void writeNext(uint32_t word)
  {
    this->page->at<0xb0/4>() = word ;
    // note: writing to 0xb4, 0xb8 or 0xbc has the same effect
  }
  
  // ------------------------------------------------------------------

  uint32_t xfer(uint32_t tx) ;
  
  void xfer(std::deque<uint32_t> const &tx,std::vector<uint32_t> *rx,bool continuous) ;
  
private:

  std::shared_ptr<Page> page ; 

} ; }

#endif // _Rpi_Spi1_h_
