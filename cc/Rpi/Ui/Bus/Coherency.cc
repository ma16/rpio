// BSD 2-Clause License, see github.com/ma16/rpio

#include "Coherency.h"

Rpi::Bus::Coherency Rpi::Ui::Bus::Coherency::get(::Ui::ArgL *argL)
{
    auto i = argL->pop_if({"--co-0","--co-4","--co-8","--co-c"}) ;
    if (!i)
	i = boost::make_optional(1u) ;
    switch (*i) {
    case 0: return Rpi::Bus::Coherency::x0() ;
    case 1: return Rpi::Bus::Coherency::x4() ;
    case 2: return Rpi::Bus::Coherency::x8() ;
    case 3: return Rpi::Bus::Coherency::xC() ;
    }
    abort() ;
}
    
