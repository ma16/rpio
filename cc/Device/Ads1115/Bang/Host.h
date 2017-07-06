// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef INCLUDE_Device_Ads1115_Bang_Host_h
#define INCLUDE_Device_Ads1115_Bang_Host_h

#include "Config.h"
#include "Record.h"
#include <Rpi/Peripheral.h>

namespace Device { namespace Ads1115 { namespace Bang {

struct Host
{
    Record::Reset reset() ;
    Record::Read readConfig() ;
    Record::Read readSample() ;
    Record::Write writeConfig(uint16_t word) ;
    
    Host(Rpi::Peripheral *rpi,Config const &config) : rpi(rpi),config(config) {}

private:

    Rpi::Peripheral *rpi ; Config config ;
    
} ;

} } }

#endif // INCLUDE_Device_Ads1115_Bang_Host_h
