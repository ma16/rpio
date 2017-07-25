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
    struct Error : Neat::Error
    {
      Error(std::string const &s) : Neat::Error("RpiExt:Pwm:" + s) {}
    } ;

    void start() ;
  
    void wait() ;
  
    void send(std::vector<uint32_t> const&) ; // [todo] some kind of bitset?

    // guess frequency by flooding the FIFO for the given duration
    double frequency(double duration) ;
    
    Pwm(Rpi::Peripheral *rpi,Rpi::Pwm::Index index)
	: timer(rpi),pwm(rpi),index(index) {}
    
private:
  
    void fill_fifo(uint32_t const*p,unsigned n) ;
  
    void send_fifo(uint32_t const *grb,unsigned n) ;

    std::pair<size_t,uint32_t> top_up(uint32_t const buffer[],uint32_t timeout) ;
    
    std::pair<size_t,uint32_t> top_up(uint32_t const buffer[]) ;
  
    Rpi::Timer timer ; Rpi::Pwm pwm ; Rpi::Pwm::Index index ; 

    // [todo] reset/recover if destructed?
    
} ; }

#endif // INCLUDE_RpiExt_Pwm_h
