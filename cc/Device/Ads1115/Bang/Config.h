// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef INCLUDE_Device_Ads1115_Bang_Config_h
#define INCLUDE_Device_Ads1115_Bang_Config_h

#include "../Circuit.h"
#include <Rpi/Pin.h>

namespace Device { namespace Ads1115 { namespace Bang {

struct Config
{
    Rpi::Pin sclPin,sdaPin ;

    Circuit::Addr addr ;

    Circuit::Timing<uint32_t> timing ;
    
    Config(
	Rpi::Pin sclPin,
	Rpi::Pin sdaPin,
	Circuit::Addr addr,
	Circuit::Timing<uint32_t> const &timing) 
	:
	sclPin(sclPin),sdaPin(sdaPin),addr(addr),timing(timing) {}
} ;

} } }

#endif // INCLUDE_Device_Ads1115_Bang_Config_h
