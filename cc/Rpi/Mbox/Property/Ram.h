// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Rpi_Mbox_Property_Ram_h_
#define _Rpi_Mbox_Property_Ram_h_

// see github.com/raspberrypi/firmware/wiki/Mailbox-property-interface

#include "../Interface.h"
#include <vector>

namespace Rpi { namespace Mbox { namespace Property
{
  struct Ram
  { 
    static std::vector<Ram> on_arm(Interface*) ;
    
    static std::vector<Ram> on_videoCore(Interface*) ;
    
    uint32_t base,size ;

    Ram(uint32_t base,uint32_t size) : base(base),size(size) {}
  } ;

} /* Property */ } /* Mbox */ } /* Rpi */

#endif // _Rpi_Mbox_Property_Clock_h_
