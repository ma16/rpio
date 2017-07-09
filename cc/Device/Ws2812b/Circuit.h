// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef INCLUDE_Device_Ws2812b_Circuit_h
#define INCLUDE_Device_Ws2812b_Circuit_h

#include <math.h>
#include <ostream>

namespace Device { namespace Ws2812b {

namespace Circuit
{
    template<typename T> struct Timing
    {
	T t0h_min ; // for 0-bit; High-level, minimum duration
	T t0h_max ; // for 0-bit; High-level, maximum duration
	T t0l_min ; // for 0-bit;  Low-level, minimum duration
	T t0l_max ; // for 0-bit;  Low-level, maximum duration
    
	T t1h_min ; // for 1-bit; High-level, minimum duration
	T t1h_max ; // for 1-bit; High-level, maximum duration
	T t1l_min ; // for 1-bit;  Low-level, minimum duration
	T t1l_max ; // for 1-bit;  Low-level, maximum duration

	T res_min ; // duration to latch (reset) the data into WS2812Bs

	Timing(
	    T t0h_min,T t0h_max,T t0l_min,T t0l_max,
	    T t1h_min,T t1h_max,T t1l_min,T t1l_max,
	    T res_min)
	    :
	    t0h_min(t0h_min),t0h_max(t0h_max),t0l_min(t0l_min),t0l_max(t0l_max),
	    t1h_min(t1h_min),t1h_max(t1h_max),t1l_min(t1l_min),t1l_max(t1l_max),
	    res_min(res_min)
	    {
		if (t0h_min > t0h_max ||
		    t0l_min > t0l_max ||
		    t1h_min > t1h_max ||
		    t1l_min > t1l_max)
		    
		    throw std::runtime_error(
			"Device::Ws2812b::Circuit::Timing:invalid") ;
	    }
    } ;

    template<typename T> inline std::string toStr(Timing<T> const &t)
    {
	std::ostringstream os ;
	os << "0-bit:((" << t.t0h_min << ',' << t.t0h_max << "),("
	   <<               t.t0l_min << ',' << t.t0l_max << ")) " 
	   << "1-bit:((" << t.t1h_min << ',' << t.t1h_max << "),("
	   <<               t.t1l_min << ',' << t.t1l_max << ")) "
	   << "latch:"   << t.res_min ;
	return os.str() ;
    }
  
    using Seconds = Timing<float> ;
    // [todo] introduce class for non-negative floating point values

    static Seconds const strict // from data sheet
    ( 25e-8f,55e-8f,70e-8f,100e-8f,65e-8f,95e-8f,30e-8f,60e-8f,50e-6f ) ;

    using Ticks = Timing<uint32_t> ;

    // convert seconds-based-timing to ticks-based-timing
    inline Ticks asTicks(Seconds const &s,double freq)
    {
	if (freq == 0.0)
	    throw std::runtime_error(
		"Device::Ws2812b::Circuit::Timing:zero frequency") ;
	return Ticks(
	    static_cast<uint32_t>( ceil(s.t0h_min*freq)),
	    static_cast<uint32_t>(floor(s.t0h_max*freq)),
	    static_cast<uint32_t>( ceil(s.t0l_min*freq)),
	    static_cast<uint32_t>(floor(s.t0l_max*freq)),
	    static_cast<uint32_t>( ceil(s.t1h_min*freq)),
	    static_cast<uint32_t>(floor(s.t1h_max*freq)),
	    static_cast<uint32_t>( ceil(s.t1l_min*freq)),
	    static_cast<uint32_t>(floor(s.t1l_max*freq)),
	    static_cast<uint32_t>( ceil(s.res_min*freq))) ;
    }
  
} } } 

#endif // INCLUDE_Device_Ws2812b_Circuit_h
