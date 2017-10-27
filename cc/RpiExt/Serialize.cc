// BSD 2-Clause License, see github.com/ma16/rpio

#include "Serialize.h"

bool RpiExt::Serialize::send(uint32_t *t0,Edge const &edge)
{
    auto t1 = this->timer.counter().read() ;
    while (t1 - (*t0) < edge.t_min)
	t1 = this->timer.counter().read() ;

    edge.level == Rpi::Gpio::Output::Level::Hi
	? this->output.raise().write(edge.pins)
	: this->output.clear().write(edge.pins) ;
    
    auto t2 = this->timer.counter().read() ;
    auto success = t2 - (*t0) <= edge.t_max ;
    (*t0) = t2 ;
    return success ;
}

bool RpiExt::Serialize::send(std::vector<Edge> const &v)
{
    auto success = true ;
    auto p = v.cbegin() ;
    auto t = this->timer.counter().read() ;
    while (success && (p != v.end()))
    {
	success = this->send(&t,*p++) ;
    }
    return success ;
}
