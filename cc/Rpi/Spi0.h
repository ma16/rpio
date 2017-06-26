// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Rpi_Spi0_h_
#define _Rpi_Spi0_h_

// --------------------------------------------------------------------
//
// Serial Peripheral Interface (SPI0)
//
// See BCM2835 ARM Peripherals: chapter 10
//
// Not all registers are covered here (yet).
//
// --------------------------------------------------------------------
//
// Errata: FIFO register (p.155)
// If TA=0 and DMAEN=0: The first 32-bit write to this register will
// set the CS register (bit:0-7) and the DLEN register (bit:16-31). --
// This is similar to the DMA mode (§10.6.3.l). Hoewever, the DLEN
// register only affects the SCLK suspension at the end of each byte
// transfer (see errata for DLEN register). -- Subsequent writes to
// the FIFO register are considered as single-byte-writes (not words).
//
// Errata: CLK Register (p.156)
// It says "The divisor must be a power of 2." It should be: "The
// divisor must be a multiple of 2."
// https://www.raspberrypi.org/forums/viewtopic.php?p=758507&sid=414c8961da811296abcda387c176e723#p758507
//
// Errata: DLEN register (p.156)
// It says "This field is only valid for DMA-mode (DMAEN set)".
// However, it also effects the non-DMA-mode: if DLEN=0 or DLEN=1,
// teh SCLK is suspended (i.e. stays low) for the duration of a
// single clock-cycle at the end of each byte-transfer. There is no
// such SCLK suspension if DLEN>1. 
// See https://www.raspberrypi.org/forums/viewtopic.php?f=44&t=181154
//
// * The base clock (APB CLK) seems to depend on the core clock which
//   varies on Pi-0 between 250 and 400 MHz. This distorts the SPI
//   clock! To prevent SPI clock changes, you have to fix the core-
//   clock to a certain speed, e.g. by core_freq=250 (/boot/config.txt).
//
//   https://raspberrypi.stackexchange.com/questions/3400/does-overclocking-affect-the-spi-apb-clock
//   https://www.raspberrypi.org/forums/viewtopic.php?f=44&t=185662
//
// --------------------------------------------------------------------
//
// DMA Setup (see § 10.6.3 + errata)
//
// Two DMA channels are required [*1]:
// * channel (A) to write the tx-fifo,
// * channel (B) to  read the rx-fifo.
// The SPI side of the DMA channel must operate with 32-bit transfers
// (see source/destination field in the DMA TI register).
//
// [*1] It might be possible to operate the SPI with a single DMA
//      channel. That would require alternating control blocks (CB)
//      to read and write the SPI.
//
// Operation:
//
// (1) Enable DMA operation by setting SPI0.DMAEN. This will also 
//     enable pacing through DREQ lines for writing (DREQ signal number
//     #6) and reading (#7). [see §4.2.1.3]
//
// (2) Write: Program a DMA control block (CB) for channel A. Besides
//     others:
//
//       CB.TI.DEST_INC = 0    # don't increment the destination address
//       CB.TI.DEST_WIDTH = 0  # use 32-bit word operations
//       CB.TI.DEST_DREQ = 1   # use paced write operations
//       CB.TI.PERMAP = 6      # pacing is done by DREQ signal # 6
//
//     The first 32-bit write sets SPI.CS register (bit:0-7),
//     SPI.CS:21-23 (bit:8-10) and SPI.DLEN register (bit:16-31). SPI.
//     DLEN should contain the number of bytes [not words] to transfer.
//
// (3) Read: Program a DMA control block for channel B. Besides others:
//
//       CB.TI.SRC_INC = 0     # don't increment the destination address
//       CB.TI.SRC_WIDTH = 0   # use 32-bit word operations
//       CB.TI.SRC_DREQ = 1    # use paced write operations
//       CB.TI.PERMAP = 7      # pacing is done by DREQ signal # 7
//
// (4) Choose two DMA channels, set CB, and enable ACTIVE.
//
// (5) Wait for the transfer end.
//
// --------------------------------------------------------------------
//
// DMA errata (new)
//
// * Cspol has no effect
//
// * The CE signal becomes shorter and shorter (from about 350ns at
//   about 4 MHz to finally 40ns at about div=500) and finally vanishes
//   * cs0 vanishes if not reversed, however keeps coming if is
//   * cs1 vanishes if reversed, however keep coming if not
//
// * If cs1 is activated, cs0 becomes also visible at low frequencies
//   with almost identical timings
//
// * Adcs has not the effect as described: it appears setting it does
//   make CE come about a clock-cycle earlier
//
// * Dmaen=1 appears to make read/writes from/to FIFO 32-bit wide
//   (wheter by DMA or CPU doesn't matter)
//
// * Writing to the FIFO if TA=0 appear to be always 32-bit writes. If
//   the word is immediately consumed (not enqueued) with the
//   characteristics described above. Thus, writes are consumed
//   immediately until one of them sets TA.
//
// * DMA operation with Dmaen=0 appears to be possible (with FIFO byte
//   access), however, reading the FIFO w/o pacing might be of very
//   little use.
//
// [todo] make a test suit and run it on several Pi's
//
// --------------------------------------------------------------------

#include "Bus/Address.h"
#include "Peripheral.h"

namespace Rpi {

struct Spi0 
{
  Spi0(Peripheral *p) : page(p->page(Peripheral::PNo::make<0x204>())) {}

  // control and status register @ 0x0
  
  constexpr static Bus::Address ctrl_addr() { return Rpi::Bus::Address(0x7e204000u) ; }
  
  enum : uint32_t
  {
    Cs      = (3u <<  0), // RW chip select
    Cpha    = (1u <<  2), // RW clock phase
    Cpol    = (1u <<  3), // RW clock polarity
    ClearTx = (1u <<  4), // WO clear tx fifo
    ClearRx = (1u <<  5), // WO clear rx fifo
    Cspol   = (1u <<  6), // RW chip select polarity
    Ta      = (1u <<  7), // RW transfer active
    Dmaen   = (1u <<  8), // RW DMA enable
    Intd    = (1u <<  9), // RW interrupt when Done=1
    Intr    = (1u << 10), // RW interrupt when Rxr=1
    Adcs    = (1u << 11), // RW automatically deassert chip select
    Ren     = (1u << 12), // RW read enable
    Len     = (1u << 13), // RW LoSSI enable
    Done    = (1u << 16), // RO transfer done
    Rxd     = (1u << 17), // RO rx fifo readable
    Txd     = (1u << 18), // RO tx fifo writable
    Rxr     = (1u << 19), // RO rx fifo full
    Rxf     = (1u << 20), // RO rx fifo needs reading
    Cspol0  = (1u << 21), // RW chip select #0 polarity
    Cspol1  = (1u << 22), // RW chip select #1 polarity
    Cspol2  = (1u << 23), // RW chip select #2 polarity
    DmaLen  = (1u << 24), // RW DMA LoSSI enable
    LenLong = (1u << 25), // RW DMA LoSSI long word
  } ;

  uint32_t getControl() const
  {
    return this->page->at<0x0/4>() ;
  }

  void setControl(uint32_t w)
  {
    this->page->at<0x0/4>() = w ;
  }

  // fifo register @ 0x4
  
  constexpr static Bus::Address fifo_addr() { return Rpi::Bus::Address(0x7e204004u) ; }
  
  uint8_t read()
  {
    return static_cast<uint8_t>(this->page->at<0x4/4>()) ;
  }

  void write(uint8_t byte)
  {
    this->page->at<0x4/4>() = byte ;
  }
  
  void write(uint8_t c,uint16_t nbytes)
  {
    // the first tx-queue entry needs to be a control word if TA==0 
    this->page->at<0x4/4>() = (static_cast<uint32_t>(nbytes)<<16) | c ;
  }

  // clock divide (CLK) register @ 0x8
  
  uint16_t getDivider() const ;

  void setDivider(uint16_t) ;

  // data length (DLEN) register @ 0x0c
  
  constexpr static Bus::Address dlen_addr() { return Rpi::Bus::Address(0x7e20400cu) ; }
  
  uint16_t getDlen() const
  {
    return static_cast<uint16_t>(this->page->at<0xc/4>()) ;
  }
  
  void setDlen(uint16_t i)
  {
    this->page->at<0xc/4>() = i ;
  }

  // DMA control (DC) register @ 0x14

  enum // thresholds (THR)
  {
    TxReq   = 0x000000ffu, // 0x20 DREQ  signal when TX_FIFO_Level <= THR
    TxPanic = 0x0000ff00u, // 0x10 Panic signal when TX_FIFO_Level <= THR
    RxReq   = 0x00ff0000u, // 0x20 DREQ  signal when RX_FIFO_Level  > THR [*1]
    RxPanic = 0xff000000u, // 0x30 Panic signal when RX_FIFO_level  > THR
  } ;
  // [*1] the datasheet says "the DREQ signal for the RxFifo is also
  // generated if the transfer has finished but the RxFifo is not empty"

  // errata: it appears the thresholds are byte-values (not words)
  // since rx and tx-fifo cover each only _16_ 32-bit words (0x40 bytes).

  uint32_t getDc() const
  {
    return this->page->at<0x14/4>() ;
  }
  
  void setDc(uint32_t i)
  {
    this->page->at<0x14/4>() = i ;
  }
  
  // helper: run communication by polling in a busy loop
    
  void transceive(size_t nbytes,uint8_t const tx[],uint8_t rx[]) ;
  
private:

  std::shared_ptr<Page> page ; 

} ;

}

#endif // _Rpi_Spi0_h_
