// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Rpi_Mbox_Property_base_h_
#define _Rpi_Mbox_Property_base_h_

// see github.com/raspberrypi/firmware/wiki/Mailbox-property-interface
// see www.raspberrypi.org/forums/viewtopic.php?f=72&t=184879

#include "../Interface.h"
#include <array>

namespace Rpi { namespace Mbox { namespace Property
{
  uint32_t firmware_revision(Interface*) ; 
  
  uint32_t board_model(Interface*) ; 
  
  uint32_t board_revision(Interface*) ; 
  
  std::array<uint8_t,6> board_mac_address(Interface*) ; 
  
  uint64_t board_serial(Interface*) ; 

  std::string command_line(Interface*) ; 

  uint32_t dma_channels(Interface*) ;

} /* Property */ } /* Mbox */ } /* Rpi */

#endif // _Rpi_Mbox_Property_base_h_
