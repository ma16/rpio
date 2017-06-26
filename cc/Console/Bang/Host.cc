// BSD 2-Clause License, see github.com/ma16/rpio

#include "Host.h"
#include <chrono>
#include <deque>

double Console::Bang::Host::frequency(Rpi::Counter counter)
{
    auto c0 = counter.clock() ;
    using clock = std::chrono::steady_clock ;
    auto t0 = clock::now() ;
    using ms = std::chrono::milliseconds ;
    while (std::chrono::duration_cast<ms>(clock::now()-t0).count() < 100)
	;
    auto c1 = counter.clock() ;
    return static_cast<double>(c1-c0) * 10 ;
}
  
bool Console::Bang::Host::send(uint32_t *t0,Edge const &edge)
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

bool Console::Bang::Host::send(std::vector<Edge> const &v)
{
    auto success = true ;
    auto t = this->counter.clock() ;
    for (auto &edge: v)
    {
	success = success && this->send(&t,edge) ;
    }
    return success ;
}
