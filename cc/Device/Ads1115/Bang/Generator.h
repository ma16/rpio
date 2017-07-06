// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef INCLUDE_Device_Ads1115_Bang_Generator_h
#define INCLUDE_Device_Ads1115_Bang_Generator_h

#include "Config.h"
#include "Record.h"
#include "../Circuit.h"
#include <RpiExt/Bang.h>

namespace Device { namespace Ads1115 { namespace Bang {

struct Generator
{
    Generator(Config const &config) : config(config) {}

    using Script = std::vector<RpiExt::Bang::Command> ;
	
    Script reset      (Record::Reset*) ;
    Script readConfig (Record::Read*) ;
    Script readSample (Record::Read*) ;
    Script writeConfig(Record::Write*,uint16_t word) ;
	
private:

    Config config ;

    using Draft = RpiExt::Bang::Enqueue ;

    enum class Line
    {
	Low, // pin connected to ground
	Off, // pin not connected (tri-state/open-drain)
    } ;
  
    void start(Draft*) ;
    void stop (Draft*,Line sda,uint32_t *t0) ;
    
    void recvBit(Draft*,Line sda,uint32_t *t0,uint32_t *levels) ;
    void sendBit(Draft*,Line from,Line to,uint32_t *t0) ;

    void recvByte(Draft*,Line sda,uint32_t *t0,Record::Read::Byte*) ;
    void sendByte(Draft*,Line sda,uint8_t byte,uint32_t *t0,uint32_t *ack) ;
    
    void read(Draft*,Record::Read*,uint8_t reg) ;
	
    void reset      (Draft*,Record::Reset*) ;
    void readConfig (Draft*,Record::Read*) ;
    void readSample (Draft*,Record::Read*) ;
    void writeConfig(Draft*,Record::Write*,uint16_t word) ;
} ;

} } }

#endif // INCLUDE_Device_Ads1115_Bang_Generator_h
