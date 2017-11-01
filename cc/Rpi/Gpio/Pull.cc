// BSD 2-Clause License, see github.com/ma16/rpio

#include "Pull.h"

// See BCM2835 ARM Peripherals §6 pp.100,101

void Rpi::Gpio::Pull::set(Base gpio,uint32_t set,Mode mode)
{
    auto control = gpio.at<Register::Gpio::Pull::Control>().value() ;

    auto bank0 = gpio.at<Register::Gpio::Pull::Bank0>().value() ;
    auto bank1 = gpio.at<Register::Gpio::Pull::Bank1>().value() ;
    
    // 1. Write to GPPUD to set the required control signal (i.e. Pull-
    //    up or Pull-Down or neither to remove the current Pull-up/down)
    (*control) = Neat::as_base(mode) ;
    
    // 2. Wait 150 cycles – this provides the required set-up time for 
    //    the control signal
    for (unsigned i=0 ; i<150 ; ++i)
    {
	auto x = (*control) ; (void)x ;
    }
    // ...but what is a "cycle"? [todo]

    // 3. Write to GPPUDCLK0/1 to clock the control signal into the GPIO 
    //    pads you wish to modify – NOTE only the pads which receive a 
    //    clock will be modified, all others will retain their previous 
    //    state.
    (*bank0) = set ; (*bank1) = 0 ;

    // 4. Wait 150 cycles – this provides the required hold time for the 
    //    control signal
    for (unsigned i=0 ; i<150 ; ++i)
    {
	auto x = (*control) ; (void)x ;
    }
    // ...again: what is a "cycle"?  [todo]

    // 5. Write to GPPUD to remove the control signal
    (*control) = 0x0 ;
    // ...write what? I guess it doesn't matter. [todo]

    // 6. Write to GPPUDCLK0/1 to remove the clock
    (*bank0) = 0 ; (*bank1) = 0 ;
    // ...write what? I guess zero is ok to "close" the gate. [todo]
}
