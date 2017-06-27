// BSD 2-Clause License, see github.com/ma16/rpio

// --------------------------------------------------------------------
// The MCP3008 is a 10-bit ADC with 8x1 (4x2) channels. The maximum
// sampling rate varies between 75k/s (2.7V) and 200k/s (5V). An SPI-
// like protocol is used to communicate with the device. Bit-banging
// can also be applied since the timing requirements are very modest.
// 
// Please refer to the MCP3008 data sheet for more details.
// --------------------------------------------------------------------

#ifndef INCLUDE_Device_Mcp3008_Circuit_h
#define INCLUDE_Device_Mcp3008_Circuit_h

#include <Neat/uint.h>

namespace Device { namespace Mcp3008 {

namespace Circuit
{
    // the 10-bit digitized sample value
    using Sample = Neat::uint<unsigned,10> ;

    // the (4-bit) signal source
    using Source = Neat::uint<unsigned,4> ;
    // 0 0 0 0 differential CH0 = IN+ CH1 = IN-
    // 0 0 0 1 differential CH0 = IN- CH1 = IN+
    // 0 0 1 0 differential CH2 = IN+ CH3 = IN-
    // 0 0 1 1 differential CH2 = IN- CH3 = IN+
    // 0 1 0 0 differential CH4 = IN+ CH5 = IN-
    // 0 1 0 1 differential CH4 = IN- CH5 = IN+
    // 0 1 1 0 differential CH6 = IN+ CH7 = IN-
    // 0 1 1 1 differential CH6 = IN- CH7 = IN+
    // 1 0 0 0 single-ended CH0
    // 1 0 0 1 single-ended CH1
    // 1 0 1 0 single-ended CH2
    // 1 0 1 1 single-ended CH3
    // 1 1 0 0 single-ended CH4
    // 1 1 0 1 single-ended CH5
    // 1 1 1 0 single-ended CH6
    // 1 1 1 1 single-ended CH7

    template<typename T> struct Timing
    {
	T csh  ; // CS disable time
	T sucs ; // CS fall to first rising CLK edge
	T hd   ; // DIN hold time
	T su   ; // DIN setup time
	T en   ; // CLK fall to output enable
	T lo   ; // CLK low time
	T hi   ; // CLK high time
	T dov  ; // CLK fall to output data valid
	T dis  ; // CS rise to output disable
	T bled ; // max time between sample and data clocked out
	
	// "It should be noted that during the entire conversion cycle,
	//  the A/D converter does not require a constant clock speed
	//  or duty cycle, as long as all timing specifications are met."
    } ;

    constexpr static Timing<float> timing_5v =
    {
	// for 5 volts (all values in seconds)
	270e-9f,100e-9f, 50e-9f, 50e-9f,125e-9f, 
	120e-9f,120e-9f,125e-9f,100e-9f,1.2e-3f, 
    } ;

    constexpr static float frequency_5v = 3.6e+6 ;
  
    constexpr static Timing<float> timing_3v =
    {
	// for 2.7 volts (all values in seconds)
	270e-9f,100e-9f, 50e-9f, 50e-9f,200e-9f, 
	120e-9f,120e-9f,200e-9f,100e-9f,1.2e-3f, 
    } ;
  
    constexpr static float frequency_3v = 1.35e+6 ;

} } } 

#endif // INCLUDE_Device_Mcp3008_Circuit_h
