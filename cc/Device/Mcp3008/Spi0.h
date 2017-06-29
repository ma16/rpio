// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef INCLUDE_Device_Mcp3008_Spi0_h
#define INCLUDE_Device_Mcp3008_Spi0_h

#include "Circuit.h"
#include <Rpi/Spi0.h>
#include <boost/optional.hpp>

namespace Device { namespace Mcp3008 {

struct Spi0
{
    boost::optional<Circuit::Sample> query(Circuit::Source) ;
  
    Circuit::Sample queryShort(Circuit::Source) ;
  
    std::pair<Circuit::Sample,Circuit::Sample> queryLong(Circuit::Source) ;
    
    Spi0(Rpi::Peripheral *rpi,bool monitor=true) : spi(rpi),monitor(monitor) {}

private:
  
    Rpi::Spi0 spi ; bool monitor ;

} ; } } 

#endif // INCLUDE_Device_Mcp3008_Spi0_h
