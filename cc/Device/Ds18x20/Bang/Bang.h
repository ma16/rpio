// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef INCLUDE_Device_Ds18x20_Bang_h
#define INCLUDE_Device_Ds18x20_Bang_h

// An implementation
// * for the 1-Wire Bus
// * the DS18B20 chip
//
// This is a bit-banged implementation. Since we're in userland, the
// execution may be interrupted or suspended at any time and for any
// duration. Luckily, the 1-Wire Bus is quite tolerant: A transaction
// can be suspended for an infinite time in between the transmission
// of bits as long as the bus is idle (high). So we "only" need to
// care about thread suspension when we pull the bus Low and when we
// wait for a response. If a suspension was detected (i.e. a given
// maximum timing was exceeded) then an exception is thrown and the
// client may retry the complete dialogue.
//
// This implementation can be used on any pin. The pin must be in
// Input mode. We toggle the mode between Input mode and Output mode
// to send LH and HL edges, though, the output level is never changed.
// The client has to make sure that the output level is set to Low.
//
// This implementation requires a clock to issue pulses, to detect the
// bus voltage level and to verify timings. The ARM Counter is used as
// clock. The client needs to set up the ARM counter.

#include <Neat/Bit/Crc.h>
#include <Protocol/OneWire/Bang/Master.h>

namespace Device { namespace Ds18x20 { namespace Bang {

struct Bang
{
    using Pad = std::bitset<72> ; // the scratch-pad
    
    // returns true as long as the current operation has not finished
    // and as long as no new operation begins
    bool isBusy() ;
    // for operations as convert and copy- and recall scratch-pad

    // start temperature measurement (after a skip or match ROM command)
    void convert() ;
    // can be followed by isBusy() to wait for completion

    bool isPowered() ;
    // returns true if Vcc is connect; false if in parasite mode

    // read scratch-pad (single drop bus only)
    Pad readPad() ;

    using Master = Protocol::OneWire::Bang::Master ;
  
    Bang(Master *master) : master(master) {}

    // ---- ....
    
    static uint8_t crc(std::vector<bool> const &v)
    {
	return Neat::Bit::Crc::x31(v) ;
    }

    template<size_t N> static uint8_t crc(std::bitset<N> const &set)
    {
	return Neat::Bit::Crc::x31(set) ;
    }
    
private:

    Master *master ;

} ; } } }

#endif // INCLUDE_Device_Ds18x20_Bang_h
