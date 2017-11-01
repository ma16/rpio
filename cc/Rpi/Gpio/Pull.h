// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef INCLUDE_Rpi_Gpio_Pull_h
#define INCLUDE_Rpi_Gpio_Pull_h

// --------------------------------------------------------------------
// BCM2835 ARM Peripherals: Chapter 6: General Purpose I/O (GPIO)
// --------------------------------------------------------------------

#include "../Peripheral.h"
#include "../Register.h"

namespace Rpi { namespace Gpio { namespace Pull
{
    enum class Mode : unsigned { Off=0,Down=1,Up=2 } ;

    using Base = Register::Base<Register::Gpio::PageNo> ;
    
    void set(Base gpio,uint32_t set,Mode mode) ;
} } }

#endif // INCLUDE_Rpi_Gpio_Pull_h
