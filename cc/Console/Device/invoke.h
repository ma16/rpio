// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef INCLUDE_Console_Device_invoke_h
#define INCLUDE_Console_Device_invoke_h

#include <Rpi/Peripheral.h>
#include <Ui/ArgL.h>

namespace Console { namespace Device
{
    namespace Ads1115 { void invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL) ; }
    namespace Ds18s20 { void invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL) ; }
    namespace Max7219 { void invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL) ; }
    namespace Mcp3008 { void invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL) ; }
    namespace Ws2812b { void invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL) ; }
} }

#endif /* INCLUDE_Console_Device_invoke_h */
