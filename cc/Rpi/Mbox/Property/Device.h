// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Rpi_Mbox_Property_Device_h_
#define _Rpi_Mbox_Property_Device_h_

// see github.com/raspberrypi/firmware/wiki/Mailbox-property-interface

#include "../Interface.h"

namespace Rpi { namespace Mbox { namespace Property
{
  struct Device 
  {
    enum class Id : uint32_t { SDCard=0,UART0=1,UART1=2,USBHCD=3,I2C0=4,I2C1=5,I2C2=6,SPI=7,CCP2TX=8 } ;

    static char const* name(Id) ;
    
    static bool get_power_state(Interface*,Id) ;
    // true if powered on

    static bool set_power_state(Interface*,Id,bool on,bool wait) ;
    // ...[note] appears to have no effect
  
    static uint32_t timing(Interface*,Id) ; 
    // ...[note] documentation says: "returns 0 wait time if the device does not exist."

  } ;
  
} /* Property */ } /* Mbox */ } /* Rpi */

#endif // _Rpi_Mbox_Property_Device_h_
