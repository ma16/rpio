// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef INCLUDE_Device_Ads1115_Circuit_h
#define INCLUDE_Device_Ads1115_Circuit_h

#include <Neat/Enum.h>

namespace Device { namespace Ads1115 {

namespace Circuit
{
    using Source = Neat::Enum<unsigned,0x7> ;
    //     |  Pos |  Neg
    // ----+------+------------
    // 000 | IN-0 | IN-1 (default)
    // 001 | IN-0 | IN-3
    // 010 | IN-1 | IN-3
    // 011 | IN-2 | IN-3
    // 100 | IN-0 |  GND
    // 101 | IN-1 |  GND
    // 110 | IN-2 |  GND
    // 111 | IN-3 |  GND

    using Addr = Neat::Enum<unsigned,0x7f> ;

    template<typename T> struct Timing 
    {
	T buf   ; // (min) bus free time between START and STOP condition
	T hdsta ; // (min) hold time after repeated START condition.
	T susto ; // (min) stop condition setup time
	T sudat ; // (min) data setup time
	T hddat ; // (min) data hold time
	T low   ; // (min) SCL clock low period
	T high  ; // (min) SCL clock high period

	/* notes:
	   - susta (repeated START condition setup time) is the same
	     as buf time
	   - there are also maximum fall and raise time constraints for
	     the SCL edges (300ns) which we could verify, however, not 
	     at this moment.
	   - the implementation doesn't verify the clock cycle, which 
	     should be: 1/400 kHz <= (high + low) <= 1/10 kHz
	*/
	
	Timing(T buf,T hdsta,T susto,T sudat,T hddat,T low,T high) :
	    buf    (buf), hdsta(hdsta), susto(susto), sudat(sudat),
	    hddat(hddat), low    (low), high  (high) {}
    } ;

    inline Timing<float> const& fast_timing()
    {
	// values from datasheet (in seconds):
	static Timing<float> t =
	{ 6e-7f,6e-7f,6e-7f,1e-7f,0,13e-7f,6e-7f } ;
	return t ;
    }
} 

} }

#endif // INCLUDE_Device_Ads1115_Circuit_h
