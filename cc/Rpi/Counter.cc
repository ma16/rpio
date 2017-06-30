// BSD 2-Clause License, see github.com/ma16/rpio

#include "Counter.h"
#include <chrono>

double Rpi::Counter::frequency() const
{
    using namespace std::chrono ;
    auto c0 = this->clock() ;
    auto t0 = steady_clock::now() ;
    auto t1 = steady_clock::now() ;
    while (duration<double>(t1-t0).count() < 0.1)
	t1 = steady_clock::now() ;
    auto c1 = this->clock() ;
    return static_cast<double>(c1-c0) / duration<double>(t1-t0).count() ;
}
  
