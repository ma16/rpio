// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Rpi_Dma_Channel_h_
#define _Rpi_Dma_Channel_h_

// --------------------------------------------------------------------
// DMA Channel (there are 16 channels alltogether)
//
// Note: A type-safe implementation is desirable but rather difficult
// to accomplish at this time
// --------------------------------------------------------------------
      
#include "Cs.h"
#include "Debug.h"
#include "Ti.h"
#include "../Bus/Address.h"
#include "../Page.h"

namespace Rpi { namespace Dma { struct Ctrl ; } }

namespace Rpi { namespace Dma { struct Channel
{
  // 0   0x0 CS        - Control and Status               (RW)
  // 1   0x4 CONBLK_AD - Control Block Address            (RW)
  // 2   0x8 TI        - CB Word 0 (Transfer Information) (RO)
  // 3   0xc SOURCE_AD - CB Word 1 (Source Address)       (RO)
  // 4  0x10 DEST_AD   - CB Word 2 (Destination Address)  (RO)
  // 5  0x14 TXFR_LEN  - CB Word 3 (Transfer Length)      (RO)
  // 6  0x18 STRIDE    - CB Word 4 (2D Stride)            (RO)
  // 7  0x1c NEXTCONBK - CB Word 5 (Next CB Address)      (RW)*
  // 8  0x20 DEBUG     - Debug                            (RW)
  //
  //  * p.41 says:"The value loaded into the NEXTCONBK register can be
  //    overwritten so that the linked list of Control Block data
  //    structures can be dynamically altered. However it is only safe
  //    to do this when the DMA is paused."
  //    However, this contradicts [elinux.org/BCM2835_registers#DMA0]
  //    which says the register is read-only.
  
  Cs                getCs() const { return              Cs(this->p[0]) ; }
  Bus::Address      getCb() const { return    Bus::Address(this->p[1]) ; }
  Ti::Word          getTi() const { return Ti::Word::coset(this->p[2]) ; }
  Bus::Address     getSrc() const { return    Bus::Address(this->p[3]) ; } 
  Bus::Address     getDst() const { return    Bus::Address(this->p[4]) ; } 
  uint32_t         getLen() const { return                 this->p[5]  ; }
  uint32_t      getStride() const { return                 this->p[6]  ; }
  Bus::Address    getNext() const { return    Bus::Address(this->p[7]) ; }
  Debug          getDebug() const { return           Debug(this->p[8]) ; }

  void    setCs(Cs             cs) { this->p[0] =   Cs::wmask & cs.u32 ; }
  void    setCb(Bus::Address addr) { this->p[1] =         addr.value() ; }
  void  setNext(Bus::Address addr) { this->p[7] =         addr.value() ; } 
  void setDebug(Debug           d) { this->p[8] = Debug::wmask & d.u32 ; }

  // [notes] ; [todo] make type-safe
  //
  // --the CONBLK_AD & NEXTCONBK address must be 32-byte aligned
  // --TXFR_LEN is 30 bit-wide
  // --TXFR_LEN is separated in YLENGTH and XLENGTH if stride-mode
  // --there a are many dependencies between TI/SRC/DST/Len/Stride
  // --channels 7..14 are Lite channels with limited functionality

  // DMA operation (see pp.40):
  //
  // A DMA transfer is started by writing the address of a CB structure
  // into the CONBLK_AD register and then setting the ACTIVE bit. The
  // DMA will fetch the CB from the address set in the SCB_ADDR field
  // of this register and it will load it into the read-only registers.
  // It will then begin the transfer.
  //
  // When it has completed the current DMA transfer (i.e. TXFR_LEN
  // drops to 0) the DMA will update the CONBLK_AD register with the
  // contents of the NEXTCONBK register, fetch a new CB from that
  // address, and start the whole procedure once again.
  //
  // The DMA will stop (and clear the ACTIVE bit) when it has completed
  // a DMA transfer and the NEXTCONBK register is 0. It will load this
  // value into the CONBLK_AD register and then stop.
  
  void setup(Bus::Address cb,Cs cs) ;

  void start() { auto cs = getCs() ; cs.active() = true ; setCs(cs) ; }

  void reset() { auto cs = getCs() ; cs.reset() = true ; setCs(cs) ; }
    
  void stop()
  {
    reset() ;
    while ((0 != getCs().active().bits()) || (0 != getCb().value()))
      ;
  }

  std::string toStr() const ;

  ~Channel()
  {
    this->stop() ;
    // [todo] there may be situations when we want to keep DMA running
  }
  
private:

  friend Ctrl ;

  Page::shared_ptr page ; uint32_t volatile *p ;

  Channel(Page::shared_ptr page,Page::Index wofs) : page(page),p(&page->at(wofs)) {} // constructed by Ctrl
    
} ; } } 

#endif // _Rpi_Dma_Channel_h_
