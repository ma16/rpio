// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef Console_Spi0_Alarm_h
#define Console_Spi0_Alarm_h

#include <chrono>
#include <cmath>

namespace Console { namespace Spi0 {

struct Alarm
{
    using Clock = std::chrono::steady_clock ;
    using Time = Clock::time_point ;
    using Duration = Clock::duration ;

    template <typename D> static Alarm start(D period)
    {
	auto now = Clock::now() ;
	auto p = std::chrono::duration_cast<Duration>(period) ;
	return Alarm(now,now,now,p) ;
    } 

    bool expired()
    {
	now = std::chrono::steady_clock::now() ;
	if (now - armed < period)
	    return false ;
	auto n = std::chrono::duration<float>(now-t0).count() / std::chrono::duration<float>(period).count() ;
	auto d = std::chrono::duration<float>(floor(n+0.5) * period) ;
	armed = t0 + std::chrono::duration_cast<Duration>(d) ;
	return true ;
    }

    Duration passed() const
    {
	return now - t0 ;
    }
    
private:

    Time t0 ; Time armed ; Time now ; Duration period ; 

    Alarm(Time t0,Time armed,Time now,Duration period) : t0(t0),armed(armed),now(now),period(period) {}
} ;

} }

#endif // Console_Spi0_Alarm_h
