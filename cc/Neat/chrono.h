#ifndef INCLUDE_Neat_chrono_h
#define INCLUDE_Neat_chrono_h

#include <chrono>

namespace Neat {

namespace chrono
{
    // std::chrono::duration is defined as:
    //
    //     template<
    //              typename Rep,
    //              typename Period = std::ratio<1>>
    //     struct duration ;
    //
    // It holds:
    // * a number of ticks (rep)
    // * the period of a single tick
    //
    // The period is determined at compile-time; only the number-of-ticks
    // requires space at runtime.
    //
    // The period is given as rational number in seconds (std::ratio):
    //
    //     template<
    //              std::intmax_t Num,
    //              std::intmax_t Denom = 1>
    //     struct ratio ;
    //
    // There are several pre-defined types, for example:
    //
    //     milli = ratio<1,1000>
    //     giga  = ratio<1000000000,1>
    //
    // Similarly, there are predefined duration types, for example:
    //
    //     milliseconds = duration<int64_t,milli>
    //     hours 	    = duration<int64_t,ratio<3600>>
    //
    // In many situations it is convenient not to deal with all these
    // types. Instead, it may be sufficient to use a floating point
    // value that holds a number of seconds.

    template<typename Duration> inline
    Duration bySeconds(double s)
    {
	auto d = std::chrono::duration<double>(s) ;
	// duration_cast casts from one duration to another
	return std::chrono::duration_cast<Duration>(d) ;
    }

    template<typename Duration> inline
    double toSeconds(Duration d)
    {
	// no cast required for integral to floating point
	return std::chrono::duration<double>(d).count() ;
    }

} }

#endif // INCLUDE_Neat_chrono_h
