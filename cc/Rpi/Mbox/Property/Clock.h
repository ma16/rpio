// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Rpi_Mbox_Property_Clock_h_
#define _Rpi_Mbox_Property_Clock_h_

// see github.com/raspberrypi/firmware/wiki/Mailbox-property-interface

#include "../Interface.h"
#include <Neat/Enum.h>
#include <vector>

namespace Rpi { namespace Mbox { namespace Property
{
  struct Clock
  {
    using State = Neat::Enum<uint32_t,3> ;
    
    static std::vector<Clock> all(Interface*) ; 

    static std::string name(uint32_t id) ;
    
    static State state(Interface*,uint32_t id) ; 

    static State setState(Interface*,uint32_t id,State) ;
    
    static uint32_t setRate(Interface *iface,uint32_t id,uint32_t rate,uint32_t skip) ;
    
    static uint32_t rate(Interface*,uint32_t id) ; 
  
    static uint32_t min(Interface*,uint32_t id) ; 

    static uint32_t max(Interface*,uint32_t id) ;
    
    uint32_t parent,id ;

    Clock(uint32_t parent,uint32_t id) : parent(parent),id(id) {}
  } ;

} /* Property */ } /* Mbox */ } /* Rpi */

#endif // _Rpi_Mbox_Property_Clock_h_
