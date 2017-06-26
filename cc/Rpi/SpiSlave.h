// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Rpi_SpiSlave_h_
#define _Rpi_SpiSlave_h_

// --------------------------------------------------------------------
//
// SPI / BSC Slave
//
// See BCM2835 ARM Peripherals: chapter 11 (pp.160)
//
// This implementation regards only the SPI Slave (no I2C). There are
// also several interrupt registers which aren't supported either.
//
// --------------------------------------------------------------------
//
// Short Description (including extensive errata)
//
// The device handles special-purpose dialogues. The dialogues are
// Octet based (8-bit groups). Octets are transmitted MSB-first. The
// first MOSI octet defines the type of the dialogue:
//
// If the LSB is 0, then all subsequent MOSI octets are read and
//   deserialized into the rx-fifo. No data is put on MISO. It will
//   be high for the complete dialogue. The tx-fifo is not touched.
//
// If the LSB is 1, then all subsequent MOSI octets are ignored. The
//   rx-fifo is not touched. However, the data in the tx-fifo is
//   serialized and put on MOSI.
//
// The 7 most significant bits of the first MOSI octet appear to be
// ignored completely. The first MISO octet is always 0xff.
//
// Note that MISO and MOSI are the other way around than described in
// the datasheet. The BCM pins are MISO=18, SCLK=19, MOSI=20, CE=21.
//
// Both, the rx- and tx-fifo, may each hold a maximum of 16 octets.
//
// Control register fields:
//
// * The BRK field does neither clear the tx- nor the rx-fifo.
//
// * If the TESTFIFO field is set, then access to TDR (FIFO test data
//   register at address 0x2c) takes effect:
//   * Read: returns the head of the tx-fifo; it will not be dequeued;
//     note that if the device is enabled, the next octet to be sent 
//     has already left the tx-fifo and was placed into the serializer
//   * Write: enqueues a new octet at the rx-fifo's tail; incoming
//     data from MOSI appears to be ignored (0x00 is enqueued instead).
//
// Other than specified, the tx-fifo can neither be cleared by BRK nor
// by reading TDR. However, it is possible to dequeue a tx-fifo octet
// by enabling the device (which places an octet into the serializer).
// Thus a sequence of disabling (0) and enabling (EN+TXE) the device
// for (TXFLEVEL+1) times clears the tx-fifo including the byte in the
// serializer.
//
// --------------------------------------------------------------------

#include "Peripheral.h"

namespace Rpi {

struct SpiSlave
{
  SpiSlave(Peripheral *p) : page(p->page(Peripheral::PNo::make<0x214>())) {}

  // data register @ 0x4
  
  enum 
  {
    Data        = 0xffu <<  0, // RW read/write rx/tx-fifo
    DataRxOver  =    1u <<  8, // RO data has received while rx-fifo was full
    DataTxUnder =    1u <<  9, // RO data has sent while tx-fifo was empty
    DataTxBusy  =    1u << 16, // RO transmit in progress
    DataRxEmpty =    1u << 17, // RO rx-fifo empty
    DataTxFull  =    1u << 18, // RO tx-fifo full
    DataRxFull  =    1u << 19, // RO rx-fifo full
    DataTxEmpty =    1u << 20, // RO tx-fifo empty
    DataRxBusy  =    1u << 21, // RO receive in progress
    DataTxSize  = 0x1fu << 22, // RO current number of bytes in tx-fifo
    DataRxSize  = 0x1fu << 27, // RO current number of bytes in rx-fifo
  } ;

  uint32_t read() const 
  {
    return this->page->at<0x0/4>() ;
  }
  
  void write(uint32_t w) 
  {
    this->page->at<0x0/4>() = w ;
  }

  // operation-status & error-clear register @ 0x4
  
  enum 
  {
    RxOver  = 1u << 0, // rx-fifo overflow
    TxUnder = 1u << 1, // tx-fifo underrun
  } ;

  uint32_t status() const 
  {
    return this->page->at<0x4/4>() ;
  }

  void reset() 
  {
    this->page->at<0x4/4>() = 0 ;
  }

  // control register @ 0xc
  
  enum 
  {
    Enable    = 1u <<  0, // enable device
    Spi       = 1u <<  1, // spi mode
    I2c       = 1u <<  2, // i2c mode
    Cpha      = 1u <<  3, // clock phase
    Cpol      = 1u <<  4, // clock polarity
    EnStat    = 1u <<  5, // (i2c) enable 8-bit  status register
    EnCtrl    = 1u <<  6, // (i2c) enable 8-bit control register [todo] says read-only ??
    Brk       = 1u <<  7, // stop operation and clear fifos
    EnTx      = 1u <<  8, // transmit mode enabled
    EnRx      = 1u <<  9, // receive mode enabled
    InvRxFull = 1u << 10, // ?? (does not affect RxFull)
    EnTest    = 1u << 11, // enable test-fifo
    EnHost    = 1u << 12, // enable host-control
    InvTxFull = 1u << 13, // ?? (does not affect TxFull)
  } ;

  uint32_t getControl() const 
  {
    return this->page->at<0xc/4>() ;
  }
  
  void setControl(uint32_t w) 
  {
    this->page->at<0xc/4>() = w ;
  }

  // flag register @ 0x10
  
  enum 
  {
    TxBusy  =    1u <<  0, // transmit in progress
    RxEmpty =    1u <<  1, // rx-fifo empty
    TxFull  =    1u <<  2, // tx-fifo full
    RxFull  =    1u <<  3, // rx-fifo full 
    TxEmpty =    1u <<  4, // tx-fifo empty
    RxBusy  =    1u <<  5, // receive in progress
    TxLevel = 0x1fu <<  6, // tx-fifo level
    RxLevel = 0x1fu << 11, // rx-fifo level (0..16)
  } ;

  uint32_t flags() const
  {
    return this->page->at<0x10/4>() ;
  }
  
  // fifo-test register @ 0x2c
  
  uint32_t readTx() const 
  {
    return this->page->at<0x2c/4>() ;
  }
  
  void writeRx(uint32_t w) 
  {
    this->page->at<0x2c/4>() = w ;
  }

private:

  std::shared_ptr<Page> page ;
  
} ;

}

#endif // _Rpi_SpiSlave_h_
