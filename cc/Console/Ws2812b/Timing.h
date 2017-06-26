// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Console_Ws2812b_Timing_h_
#define _Console_Ws2812b_Timing_h_

// --------------------------------------------------------------------
// WS2812B timing requirements
// e.g. at cdn-shop.adafruit.com/datasheets/WS2812.pdf
// --------------------------------------------------------------------

#include <Rpi/Peripheral.h>
#include <Ui/ArgL.h>
#include <Ui/strto.h>
#include <math.h>

namespace Console { namespace Ws2812b { namespace Timing
{
  template<typename T> struct Template
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
  } ;

  template<typename T> inline std::string toStr(Template<T> const &t)
  {
    std::ostringstream os ;
    os << "0-bit:((" << t.t0h_min << ',' << t.t0h_max << "),("  << t.t0l_min << ',' << t.t0l_max << ")) " 
       << "1-bit:((" << t.t1h_min << ',' << t.t1h_max << "),("  << t.t1l_min << ',' << t.t1l_max << ")) "
       << "latch:"   << t.res_min ;
    return os.str() ;
  }
  
  using Seconds = Template<double> ;
  // ...[todo] make sure no negative values

  constexpr static Seconds strict =
    // from data sheet
    { 250e-9,550e-9,700e-9,1000e-9,650e-9,950e-9,300e-9,600e-9,50e-6 } ;

  constexpr static Seconds relaxed =
    // arbitrary values by toying with
    { 250e-9,600e-9,700e-9,1200e-9,700e-9,1200e-9,250e-9,600e-9,50e-6 } ;    

  inline Seconds get(Ui::ArgL *argL)
  {
    auto s = strict ;
    if (argL->pop_if("--relaxed")) {
      s = relaxed ;
    }
    else if (argL->pop_if("--timing")) {
      s.t0h_min = Ui::strto<double>(argL->pop()) ;
      s.t0h_max = Ui::strto<double>(argL->pop()) ;
      s.t0l_min = Ui::strto<double>(argL->pop()) ;
      s.t0l_max = Ui::strto<double>(argL->pop()) ;
      s.t1h_min = Ui::strto<double>(argL->pop()) ;
      s.t1h_max = Ui::strto<double>(argL->pop()) ;
      s.t1l_min = Ui::strto<double>(argL->pop()) ;
      s.t1l_max = Ui::strto<double>(argL->pop()) ;
      s.res_min = Ui::strto<double>(argL->pop()) ;
    }
    return s ;
  }
  
  using Ticks = Template<uint32_t> ;

  inline Ticks compute(Seconds const &s,double f)
  {
    Ticks t = {
      static_cast<uint32_t>( ceil(s.t0h_min*f)),
      static_cast<uint32_t>(floor(s.t0h_max*f)),
      static_cast<uint32_t>( ceil(s.t0l_min*f)),
      static_cast<uint32_t>(floor(s.t0l_max*f)),
      static_cast<uint32_t>( ceil(s.t1h_min*f)),
      static_cast<uint32_t>(floor(s.t1h_max*f)),
      static_cast<uint32_t>( ceil(s.t1l_min*f)),
      static_cast<uint32_t>(floor(s.t1l_max*f)),
      static_cast<uint32_t>( ceil(s.res_min*f)),
    } ;
    // ...[todo] make sure no overflows occur
    if ((t.t0h_min > t.t0h_max) ||
	(t.t0l_min > t.t0l_max) ||
	(t.t1h_min > t.t1h_max) ||
	(t.t1l_min > t.t1l_max))
      throw std::runtime_error("Ticks:cannot compute") ;
    return t ;
  }

  struct Nticks
  {
    // number of ticks (exactly)
    uint32_t t0h ; // for 0-bit; High-level
    uint32_t t0l ; // for 0-bit;  Low-level
    uint32_t t1h ; // for 1-bit; High-level
    uint32_t t1l ; // for 1-bit;  Low-level
    uint32_t res ; // duration to latch (reset) the data into WS2812Bs
  } ;

  inline std::string toStr(Nticks const &n)
  {
    std::ostringstream os ;
    os << "0-bit:(" << n.t0h << ',' << n.t0l << ") 1-bit:(" << n.t1h << ',' << n.t1l << ") latch:" << n.res ;
    return os.str() ;
  }
  
  inline Nticks computeNticks(Seconds const &s,double f)
  {
    Nticks n = {
      static_cast<uint32_t>((s.t0h_min+s.t0h_max)/2*f+0.5),
      static_cast<uint32_t>((s.t0l_min+s.t0l_max)/2*f+0.5),
      static_cast<uint32_t>((s.t1h_min+s.t1h_max)/2*f+0.5),
      static_cast<uint32_t>((s.t1l_min+s.t1l_max)/2*f+0.5),
      static_cast<uint32_t>(ceil(s.res_min*f)),
    } ;
#if 1   
    // ...[todo] make sure no overflows occur
    if (((n.t0h/f < s.t0h_min) || (s.t0h_max < n.t0h/f)) ||
	((n.t0l/f < s.t0l_min) || (s.t0l_max < n.t0l/f)) ||
	((n.t1h/f < s.t1h_min) || (s.t1h_max < n.t1h/f)) ||
	((n.t1l/f < s.t1l_min) || (s.t1l_max < n.t1l/f)))
      throw std::runtime_error("Nticks:cannot compute") ;
#endif
    return n ;
  }
  
} } } 

#endif // _Console_Ws2812b_Timing_h_
