// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef INCLUDE_Device_Ws2812b_Pwm_h
#define INCLUDE_Device_Ws2812b_Pwm_h

// --------------------------------------------------------------------
// The PWM's (hardware) serializer is used to setup the RGB values of
// a WS2812B (LED) chain. PWM is used in Poll mode (no DMA) which might
// even work if the process is de-scheduled for just a brief moment in
// time.
// --------------------------------------------------------------------

#include "Timing.h"
#include <Rpi/Timer.h>
#include <Rpi/Pwm.h>
#include <deque>
#include <vector>

namespace Device { namespace Ws2812b {

struct Pwm
{
    struct BitStream
    {
	void push_back(bool bit) { this->q.push_back(bit) ; }

	size_t size() const { return this->q.size() ; }

	void push_back(bool bit,uint32_t nticks) ;

	void push_back(Timing::Nticks const&,uint32_t rgb) ;
  
	void push_back(Timing::Nticks const&,uint32_t rgb,size_t n) ;

	std::vector<uint32_t> to_uint32() const ;

    private:
    
	std::deque<bool> q ;
	// rather slow and memory consuming...
	// however we go with simple at this time here
    } ;

    void start() ;
  
    void wait() ;
  
    void send(uint32_t nleds,uint32_t rgb) ;

    Pwm(Rpi::Peripheral *rpi,Rpi::Pwm::Index index,Timing::Nticks t)
	: timer(rpi),pwm(rpi),index(index),t(t) {}
    
private:
  
    void fill_fifo(uint32_t const*p,unsigned n) ;
  
    void send_fifo(uint32_t const *rgb,unsigned n) ;
  
    Rpi::Timer timer ; Rpi::Pwm pwm ; Rpi::Pwm::Index index ; Timing::Nticks t ; 

    // [todo] reset if destructed
    
} ; } }

#endif // INCLUDE_Device_Ws2812b_Pwm_h
