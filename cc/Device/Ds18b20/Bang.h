// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef INCLUDE_Device_Ds18b20_Bang_h
#define INCLUDE_Device_Ds18b20_Bang_h

// An implementation
// * for the 1-Wire Bus
// * the DS18B20 chip
//
// This is a bit-banged implementation. Since we're in userland, the
// execution may be interrupted or suspended at any time and for any
// duration. Luckily, the 1-Wire Bus is quite tolerant: A transaction
// can be suspended for an infinite time in-between the transmission
// of bits as long as the bus is idle (high). So we "only" need to
// care about thread suspension when the bus is pulled Low and when we
// wait for a response. If a suspension was detected (i.e. a given
// maximum timing was exceeded) then an exception is thrown and the
// client may retry the dialogue from the beginning (i.e. with the
// initialization sequence).
//
// This implementation can be used on any pin. The pin must be in
// Input mode. We toggle the mode between Input mode and Output mode
// to send LH and HL edges, though, the output level is never changed.
// The client has to make sure that the output level is set to Low.
//
// This implementation requires a clock to issue pulses, to detect the
// bus voltage level and to verify timings. The ARM Counter is used as
// clock. The client needs to set up the ARM counter.

#include <Protocol/OneWire/Bang/Address.h>
#include <Protocol/OneWire/Bang/Signaling.h>
#include <boost/optional.hpp>

namespace Device { namespace Ds18b20 { 

struct Bang
{
    using Address = Protocol::OneWire::Bang::Address ;
  
    using Pad = std::bitset<72> ; // the scratch-pad

    using Mod = std::bitset<24> ; // writable part of the scratch-pad
    
    // returns true as long as the current operation has not finished
    // and as long as no new operation begins
    bool isBusy() ;
    // for operations as convert and copy- and recall scratch-pad

    // start temperature measurement (after a skip or match ROM command)
    void convert(boost::optional<Address> const&) ;
    // can be followed by isBusy() to wait for completion

    bool isPowered(boost::optional<Address> const&) ;
    // returns true if Vcc is connect; false if in parasite mode

    // read scratch-pad
    Pad readPad(boost::optional<Address> const&) ;

    // restore scratch-pad's thresholds
    void restoreThresholds(boost::optional<Address> const&) ;

    // save scratch-pad's thresholds
    void saveThresholds(boost::optional<Address> const&) ;

    // write scratch-pad's thresholds
    void write(Mod mod,boost::optional<Address> const&) ;

    using Master = Protocol::OneWire::Bang::Master ;
    
    Bang(Master *master) : signaling(master) {}

private:

    using Signaling = Protocol::OneWire::Bang::Signaling ;
  
    Signaling signaling ;

} ; } }

#endif // INCLUDE_Device_Ds18b20_Bang_h
