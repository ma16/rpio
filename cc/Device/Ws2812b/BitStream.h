// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef INCLUDE_Device_Ws2812b_BitStream_h
#define INCLUDE_Device_Ws2812b_BitStream_h

#include <Rpi/Timer.h>
#include <Rpi/Pwm.h>
#include <deque>
#include <vector>

namespace Device { namespace Ws2812b {

struct BitStream
{
    struct Ticks
    {
	uint32_t t0h ; // for 0-bit; High-level
	uint32_t t0l ; // for 0-bit;  Low-level
	uint32_t t1h ; // for 1-bit; High-level
	uint32_t t1l ; // for 1-bit;  Low-level
	uint32_t res ; // duration to latch (reset) the data into WS2812Bs
    } ;
    
    static std::vector<uint32_t> make(Ticks const&,uint32_t grb,size_t n) ;

private:
    
    // A deque of bools is memory consuming, and there are faster 
    // implementations too. Still, we go with simple at this time.

    std::deque<bool> q ; Ticks ticks ;

    BitStream(Ticks const &ticks) : ticks(ticks) {}
    
    void pushBit(bool bit,size_t n) ;

    void pushWord(uint32_t grb) ;
  
    void push(uint32_t grb,size_t n) ;

    std::vector<uint32_t> copy() const ;
} ;

} } 

#endif // INCLUDE_Device_Ws2812b_BitStream_h
