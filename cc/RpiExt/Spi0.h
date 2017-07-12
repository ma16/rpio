// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef INCLUDE_RpiExt_Spi0_h
#define INCLUDE_RpiExt_Spi0_h

#include <Rpi/Timer.h>
#include <Rpi/Spi0.h>
#include <vector>

namespace RpiExt {

struct Spi0
{
    struct Error : Neat::Error
    {
      Error(std::string const &s) : Neat::Error("RpiExt:Spi0:" + s) {}
    } ;

    // [todo] function to guess (?) frequency 
    
    std::vector<uint8_t> xfer(std::vector<uint8_t> const&) ;
	
    Spi0(Rpi::Peripheral *rpi) : spi(rpi) {}
    
private:
  
    Rpi::Spi0 spi ;
    
} ; }

#endif // INCLUDE_RpiExt_Spi0_h
