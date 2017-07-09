// BSD 2-Clause License, see github.com/ma16/rpio

#include "Serialize.h"

bool RpiExt::Serialize::send(uint32_t *t0,Edge const &edge)
{
    auto t1 = this->counter.clock() ;
    while (t1 - (*t0) < edge.t_min)
	t1 = this->counter.clock() ;
    this->gpio.setOutput(edge.pins,edge.output) ;
    auto t2 = this->counter.clock() ;
    auto success = t2 - (*t0) <= edge.t_max ;
    (*t0) = t2 ;
    return success ;
}

bool RpiExt::Serialize::send(std::vector<Edge> const &v)
{
    auto success = true ;
    auto t = this->counter.clock() ;
    for (auto &edge: v)
    {
	success = success && this->send(&t,edge) ;
    }
    return success ;
}
