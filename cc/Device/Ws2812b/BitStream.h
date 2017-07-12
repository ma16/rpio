// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef INCLUDE_Device_Ws2812b_BitStream_h
#define INCLUDE_Device_Ws2812b_BitStream_h

#include <cstdint>
#include <cstddef>
#include <deque>
#include <string>
#include <vector>

namespace Device { namespace Ws2812b {

struct BitStream
{
    template<typename T> struct Timing
    {
	struct Pulse
	{
	    T hi,lo ; // duration of an HL-pulse
	    constexpr Pulse(T hi,T lo) : hi(hi),lo(lo) {}
	} ;
	Pulse bit_0 ; // for a 0-bit
	Pulse bit_1 ; // for a 1-bit
	T reset ; // min time to latch data into WS2812B chain

	constexpr Timing(Pulse bit_0,Pulse bit_1,T reset) :
	    bit_0(bit_0),bit_1(bit_1),reset(reset) {}

	std::string toStr() const ;
	// ...only implemented for float and size_t
    } ;

    using Seconds = Timing<float> ;

    // timing defaults by datasheet
    constexpr static Seconds strict()
    {
	return Seconds(
	    Seconds::Pulse(40e-8f,85e-8f),
	    Seconds::Pulse(80e-8f,45e-8f),
	    50e-6f) ;
    }

    // this implementatio uses ticks however; the tick duration is left
    // to the client, i.e. to the configuration of the PWM peripheral
    using Ticks = Timing<size_t> ;

    // the returned vector can be used for RpiExt::Pwm::send()
    static std::vector<uint32_t> make32(Ticks const&,uint32_t grb,size_t n) ;

    // the returned vector can be used for RpiExt::Spi0::xfer()
    static std::vector<uint8_t> make8(Ticks const&,uint32_t grb,size_t n) ;

private:
    
    // A deque of bools is memory consuming, and there are faster 
    // implementations too. Still, we go with simple at this time.

    std::deque<bool> q ; Ticks ticks ;

    BitStream(Ticks const &ticks) : ticks(ticks) {}
    
    void pushBit(bool bit,size_t n) ;

    void pushWord(uint32_t grb) ;
  
    void push(uint32_t grb,size_t n) ;

    template<typename T> std::vector<T> pack() const ;

    // [todo] all these private functions and the related includes can
    // be hidden in the implementation file (visibility + compile time).
    
} ;

} } 

#endif // INCLUDE_Device_Ws2812b_BitStream_h
