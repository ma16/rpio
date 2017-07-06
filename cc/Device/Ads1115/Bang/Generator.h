// BSD 2-Clause License, see github.com/ma16/rpio

// --------------------------------------------------------------------
// The ADS1115 is an I2C driven 15-bit (+sign) ADC with 2x2 or 4x1
// multiplexed channels and a maximum sampling rate of somewhat below
// 1k/s.
//
// This implementation does only support the "single-shot" mode with
// an up to 400khz bus clock (bit-banged).
//
// This implementation does not support:
// --continuous mode
// --comparator configuration
// --high speed I2C bus clock
//
// Please refer to the ADS1115 datasheet for more details.
// --------------------------------------------------------------------

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
	
//private:

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
