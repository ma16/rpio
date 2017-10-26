// BSD 2-Clause License, see github.com/ma16/rpio

#include "ArmTimer.h"
#include <chrono>

constexpr Rpi::Peripheral::PNo Rpi::ArmTimer::PNo ;

double Rpi::ArmTimer::frequency() const
{
    using namespace std::chrono ;
    auto c0 = this->counter().read() ;
    auto t0 = steady_clock::now() ;
    auto t1 = steady_clock::now() ;
    while (duration<double>(t1-t0).count() < 0.1)
	t1 = steady_clock::now() ;
    auto c1 = this->counter().read() ;
    return static_cast<double>(c1-c0) / duration<double>(t1-t0).count() ;
}
  
