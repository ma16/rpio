// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef INCLUDE_Device_Ws2812b_Spi0_h
#define INCLUDE_Device_Ws2812b_Spi0_h

#include <Rpi/Spi0.h>
#include <deque>
#include <vector>

namespace Device { namespace Ws2812b {

struct Spi0
{
    struct Timing
    {
	double res =  50e-6 ;
	double t0h = 400e-9 ;
	double t0l = 850e-9 ;
	double t1h = 800e-9 ;
	double t1l = 450e-9 ;
    } ;

    struct BitStream
    {
	void push_back(bool bit) { this->q.push_back(bit) ; }

	size_t size() const { return this->q.size() ; }

	void push_back(bool bit,double ticks) ;

	void push_back(Timing const &t,double f,uint32_t rgb) ;
  
	void push_back(Timing const &t,double f,uint32_t rgb,size_t n) ;

	std::vector<uint8_t> to_uint8() const ;

	//private:
    
	std::deque<bool> q ;
	// rather slow and memory consuming...
	// however we go with simple at this time here
    } ;

    void send(uint32_t nleds,uint32_t rgb) ;

    Spi0(Rpi::Spi0 spi,Timing const &t,double f) : spi(spi),t(t),f(f) {}
    
private:
  
    Rpi::Spi0 spi ; Timing t ; double f ;

    // [todo] reset if destructed
    
} ; } }

#endif // INCLUDE_Device_Ws2812b_Spi0_h
