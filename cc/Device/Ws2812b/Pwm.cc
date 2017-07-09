// BSD 2-Clause License, see github.com/ma16/rpio

#include "Pwm.h"

void Device::Ws2812b::Pwm::BitStream::push_back(bool bit,uint32_t nticks)
{
    for (auto i=0u ; i<nticks ; ++i)
	this->push_back(bit) ;
}

void Device::Ws2812b::Pwm::BitStream::push_back(Timing::Nticks const &t,uint32_t rgb)
{
    for (uint32_t mask=(1u<<23) ; mask!=0 ; mask>>=1) {
	if (0 == (rgb & mask)) {
	    this->push_back(1,t.t0h) ;
	    this->push_back(0,t.t0l) ;
	}
	else {
	    this->push_back(1,t.t1h) ;
	    this->push_back(0,t.t1l) ;
	}
    }
}
  
void Device::Ws2812b::Pwm::BitStream::push_back(Timing::Nticks const &t,uint32_t rgb,size_t n)
{
    for (unsigned i=0 ; i<n ; ++i)
	this->push_back(t,rgb) ;
}

std::vector<uint32_t> Device::Ws2812b::Pwm::BitStream::to_uint32() const
{
    assert(0 == (this->q.size() % 32)) ;
    auto n = (this->q.size()+31) / 32 ;
    std::vector<uint32_t> v ;
    v.reserve(n) ;
    unsigned i = 0 ;
    uint32_t bits = 0 ; // compiler may complain if not init.
    for (auto bit : this->q) {
	bits <<= 1 ;
	bits |= bit ;
	if (++i < 32)
	    continue ;
	v.push_back(bits) ;
	i = 0 ;
    }
    return v ;
}

void Device::Ws2812b::Pwm::start()
{
    // [todo] clear fifo; clear errors
    this->pwm.setRange(this->index,32) ;
    auto c = this->pwm.getControl() ;
    auto x = c.get(this->index) ;
    x.mode = 1 ;
    x.pwen = 1 ;
    x.usef = 1 ;
    c.set(index,x) ;
    this->pwm.setControl(c) ;
}
  
void Device::Ws2812b::Pwm::wait()
{
    while (0 == this->pwm.getStatus().cempt())
	;
    auto c = this->pwm.getControl() ;
    auto x = c.get(index) ;
    x.pwen = 0 ;
    c.set(index,x) ;
    this->pwm.setControl(c) ;
}
  
void Device::Ws2812b::Pwm::send(uint32_t nleds,uint32_t rgb)
{
    BitStream q ;
    q.push_back(0,t.res) ;
    q.push_back(t,rgb,nleds) ;
    q.push_back(0,t.res) ; 
    for (auto i=0 ; i<32 ; ++i)
	q.push_back(0) ; // for RPTL
    while (0 != (q.size() % 32))
	q.push_back(0) ;
    auto v = q.to_uint32() ;

    if (v.size() < 16) {
	this->fill_fifo(&v[0],v.size()) ; 
	this->start() ;
    }
    else {
	this->fill_fifo(&v[0],16) ; 
	this->start() ;
	this->send_fifo(&v[16],v.size()-16) ;
    }
    this->wait() ;
}

void Device::Ws2812b::Pwm::fill_fifo(uint32_t const*p,unsigned n)
{
    for (decltype(n) i=0 ; i<n ; ++i) 
	this->pwm.write(p[i]) ;
    assert(0 == this->pwm.getStatus().cwerr()) ; // [todo] throw
}

void Device::Ws2812b::Pwm::send_fifo(uint32_t const *rgb,unsigned n)
{
    for (decltype(n) i=0 ; i<n ; ++i) {
	auto t0 = this->timer.cLo() ;
	while (0 != this->pwm.getStatus().cfull())
	    ;
	if (0 != this->pwm.getStatus().cempt()) {
	    auto t1 = this->timer.cLo() ;
	    auto c = this->pwm.getControl() ;
	    auto x = c.get(index) ;
	    x.pwen = 0 ;
	    c.set(index,x) ;
	    this->pwm.setControl(c) ;
	    throw std::runtime_error("Pwm:unexpected empty FIFO:" + std::to_string(i) + " " + std::to_string(t1-t0)) ;
	}
	this->pwm.write(rgb[i]) ;
	if (0 != this->pwm.getStatus().cwerr())
	    throw std::runtime_error("Pwm:write error even if not full") ;
    }
}
