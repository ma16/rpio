// BSD 2-Clause License, see github.com/ma16/rpio

// --------------------------------------------------------------------
// PWM is used in Poll mode (no DMA) which might even work if the
// process is de-scheduled for just a brief moment in time.
// --------------------------------------------------------------------

#ifndef INCLUDE_RpiExt_Pwm_h
#define INCLUDE_RpiExt_Pwm_h

#include <Rpi/Timer.h>
#include <Rpi/Pwm.h>
#include <vector>

namespace RpiExt {

struct Pwm
{
    void start() ;
  
    void wait() ;
  
    void send(std::vector<uint32_t> const&) ; // [todo] some kind of bitset?

    Pwm(Rpi::Peripheral *rpi,Rpi::Pwm::Index index)
	: timer(rpi),pwm(rpi),index(index) {}
    
private:
  
    void fill_fifo(uint32_t const*p,unsigned n) ;
  
    void send_fifo(uint32_t const *grb,unsigned n) ;
  
    Rpi::Timer timer ; Rpi::Pwm pwm ; Rpi::Pwm::Index index ; 

    // [todo] reset/recover if destructed?
    
} ; }

#endif // INCLUDE_RpiExt_Pwm_h
