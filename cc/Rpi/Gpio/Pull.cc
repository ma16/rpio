// BSD 2-Clause License, see github.com/ma16/rpio

#include "Pull.h"

constexpr Rpi::Peripheral::PNo Rpi::Gpio::Pull::PNo ;

void Rpi::Gpio::Pull::set(uint32_t set,Mode mode)
{
    // 1. Write to GPPUD to set the required control signal (i.e. Pull-
    //    up or Pull-Down or neither to remove the current Pull-up/down)
    this->control().write(Control::Word::coset(ModeEnum(mode).n())) ;
    // ...[todo] that is an awful lot
    
    // 2. Wait 150 cycles – this provides the required set-up time for 
    //    the control signal
    for (unsigned i=0 ; i<150 ; ++i)
    {
	auto x = this->control().read() ; (void)x ;
    }
    // ...but what is a "cycle"? [todo]

    // 3. Write to GPPUDCLK0/1 to clock the control signal into the GPIO 
    //    pads you wish to modify – NOTE only the pads which receive a 
    //    clock will be modified, all others will retain their previous 
    //    state.
    this->bank0().write(set) ;

    // 4. Wait 150 cycles – this provides the required hold time for the 
    //    control signal
    for (unsigned i=0 ; i<150 ; ++i)
    {
	auto x = this->control().read() ; (void)x ;
    }
    // ...again: what is a "cycle"?  [todo]

    // 5. Write to GPPUD to remove the control signal
    this->control().write(Control::Word()) ;
    // ...write what? I guess it doesn't matter. [todo]

    // 6. Write to GPPUDCLK0/1 to remove the clock
    this->bank0().write(0) ;
    // ...write what? I guess zero is ok to "close" the gate. [todo]
}
