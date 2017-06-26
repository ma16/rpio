// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Rpi_Dma_Ctrl_h_
#define _Rpi_Dma_Ctrl_h_

// --------------------------------------------------------------------
// DMA controller -- used as "venue" for DMA access
// --------------------------------------------------------------------

#include "Channel.h"
#include "../Peripheral.h"
#include <Neat/uint.h>

namespace Rpi { namespace Dma { struct Ctrl 
{
  // dis/enable DMA channels (note that channel #15 is always on)
  
  using uint15_t = Neat::uint<uint32_t,15> ;
  
  uint15_t enabled() const { return uint15_t::coset(page_007->at<0xff0u/4u>()) ; }
  
  void enable(uint15_t set) { page_007->at<0xff0u/4u>() = set.value() ; }

  // get/reset interrupt status for all channels
  
  uint16_t status() const { return 0xffffu & page_007->at<0xfe0u/4u>() ; }
  
  void reset(uint16_t set) { page_007->at<0xfe0u/4u>() = static_cast<uint32_t>(set) ; }

  // all other DMA activities are controlled by the Channel class

  using Index = Neat::Enum<uint32_t,15> ; // 0..15
  
  Channel channel(Index i)
  {
    if (i.value() == 15) {
      static auto const wofs = Page::Index::make<0>() ;
      return Channel(page_e05,wofs) ;
      // [note] channel 15 appears to be not working
      // --www.raspberrypi.org/forums/viewtopic.php?f=71&t=56235
      // --github.com/raspberrypi/linux/issues/921

    }
    else {
      auto wofs = Page::Index::make(i.value()*0x100u/4u) ;
      return Channel(page_007,wofs) ;
    }
  }

  Ctrl(Peripheral *p) :
    page_007(p->page(Peripheral::PNo::make<0x007>())),
    page_e05(p->page(Peripheral::PNo::make<0xe05>()))
  {}
  
private:

  std::shared_ptr<Page> page_007,page_e05 ;

} ; } }

#endif // _Rpi_Dma_Ctrl_h_
