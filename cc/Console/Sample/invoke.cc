// BSD 2-Clause License, see github.com/ma16/rpio

#include "../rpio.h"
#include <Rpi/ArmTimer.h>
#include <Rpi/Pin.h>
#include <Rpi/Register.h>
#include <Ui/strto.h>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <vector>

// --------------------------------------------------------------------
    
static uint32_t getPins(Ui::ArgL *argL)
{
    auto arg = argL->pop() ;
    if (arg == "all") 
	return 0xffffffff ;
    if (arg == "-m") 
	return Ui::strto<uint32_t>(argL->pop()) ;
    if (arg == "-l")
    {
	auto arg = argL->pop() ;
	auto tail = arg.find(',') ;
	decltype(tail) head = 0 ;
	auto mask = 0u ;
	while (tail != arg.npos)
	{
	    auto pin = Ui::strto(arg.substr(head,tail-head),Rpi::Pin()) ;
	    mask |=  (1u << pin.value()) ;
	    head = tail + 1 ;
	    tail = arg.find(',',head) ;
	}
	auto pin = Ui::strto(arg.substr(head),Rpi::Pin()) ;
	mask |=  (1u << pin.value()) ;
	return mask ;
    }
    auto pin = Ui::strto(arg,Rpi::Pin()) ;
    return (1u << pin.value()) ;
}
// [todo] deduplicate (gpio)
    
// --------------------------------------------------------------------
    
static std::string mkhdr(uint32_t mask)
{
    std::ostringstream os ;
    for (auto i=0 ; i<32 ; ++i) 
	if (mask & (1u<<i))
	    os << i/10 << ' ' ;
    os << '\n' ;
    for (auto i=0 ; i<32 ; ++i) 
	if (mask & (1u<<i))
	    os << i%10 << ' ' ;
    return os.str() ;
}

static std::string mksep(uint32_t mask)
{
    std::ostringstream os ;
    for (auto i=0 ; i<32 ; ++i) 
	if (mask & (1u<<i))
	    os << "--" ;
    return os.str() ;
}

static std::string mkstr(uint32_t mask,uint32_t bits)
{
    char s[32*2] ;
    auto ofs = 0u ;
    for (uint32_t m=1u ; m!=0 ; m<<=1)
    {
	if (m & mask)
	{
	    s[ofs++] = (bits & m) ? '1' : '0' ;
	    s[ofs++] = ' ' ;
	}
    }  
    return std::string(s,ofs) ;
} 
// [todo] deduplicate (gpio)

// --------------------------------------------------------------------

// [todo] change name
static void count(Rpi::Peripheral *rpi,Ui::ArgL *argL) 
{
    auto pins = getPins(argL) ;
    // [future] does also work with a pin mask
    auto nsamples = Ui::strto<unsigned>(argL->pop()) ;
    auto dry = argL->pop_if("-d") ;
    argL->finalize() ;
    auto status = rpi->at<Rpi::Register::Gpio::Event::Status0>() ;
    auto mask = pins ;
    decltype(nsamples) nevents = 0 ;
    decltype(nsamples) nsubseq = 0 ;
    status.write(mask) ;
    auto t0 = std::chrono::steady_clock::now() ;
    if (dry)
    {
	for (decltype(nsamples) i=0 ; i<nsamples ; ++i)
	{
	    auto w = status.read() ;
	    if (w.test(mask))
		status.write(w) ;
	}
    }
    else
    {
	auto active = false ;
	for (decltype(nsamples) i=0 ; i<nsamples ; ++i)
	{
	    auto w = status.read() ;
	    if (w.test(mask))
	    {
		status.write(w) ;
		++nevents ;
		if (active) ++nsubseq ;
		else active = true ;
	    }
	    else active = false ;
	}
    }
    auto tn = std::chrono::steady_clock::now() ; 
    auto dt = std::chrono::duration<double>(tn-t0).count() ;
    std::cout.setf(std::ios::scientific) ;
    std::cout.precision(2) ;
    std::cout << "r=" << static_cast<double>(nsamples)/dt << "/s " 
	      << "f=" << static_cast<double>( nevents)/dt << "/s "
	      << "s=" << static_cast<double>( nsubseq)/dt << "/s\n" ;
}

// --------------------------------------------------------------------

// sample input level [todo] we need a better name
static void duty(Rpi::Peripheral *rpi,Ui::ArgL *argL) 
{
    auto pin = Ui::strto(argL->pop(),Rpi::Pin()) ;
    auto nsamples = Ui::strto<uint32_t>(argL->pop()) ;
    auto dry = argL->pop_if("-d") ;
    argL->finalize() ;
    auto input = rpi->at<Rpi::Register::Gpio::Input::Bank0>() ;
    auto mask = 1u << pin.value() ;
    decltype(nsamples) nchanges = 0 ; // transition counter
    decltype(nsamples) nhi = 0 ; // High-level counter
    auto level = input.read().test(mask) ;
    auto t0 = std::chrono::steady_clock::now() ;
    if (dry)
    {
	// this dry-run might be useful to estimate the additional
	// cpu-time that is required to execute the real test.
	for (decltype(nsamples) i=0 ; i<nsamples ; ++i)
	{
	    level ^= input.read().test(mask) ;
	    // ...level is used to prevent compiler optimizations
	}
	auto volatile x = level ; (void)x ;
    }
    else
    {
	for (decltype(nsamples) i=0 ; i<nsamples ; ++i)
	{
	    auto next = input.read().test(mask) ;
	    if (next != level)
	    {
		level = next ;
		++nchanges ;
	    }
	    if (next)
		++nhi ;
	}
    }
    auto t1 = std::chrono::steady_clock::now() ; 
    auto dt = std::chrono::duration<double>(t1-t0).count() ;
    std::cout.setf(std::ios::scientific) ;
    std::cout.precision(2) ;
    
    std::cout << "r=" << nsamples/dt << "/s " 
	      << "f=" << nchanges/dt/2 << "/s "
	      << "duty=" << 1.0 * nhi / nsamples << "\n" ;
}

// --------------------------------------------------------------------

static void pulse(Rpi::Peripheral *rpi,Ui::ArgL *argL) 
{
    auto pin = Ui::strto(argL->pop(),Rpi::Pin()) ;
    argL->finalize() ;
    
    auto mask = 1u << pin.value() ;
    
    namespace Register = Rpi::Register::Gpio ;
    auto gpio = rpi->page<Register::PageNo>() ;

    using Clock = std::chrono::steady_clock ;
    
    auto wait4edge = [&](Clock::time_point *t0,Clock::time_point *t1)
	{
	    auto status = gpio.at<Register::Event::Status0>().value() ;
	    (*status) = mask ;
	    while (0 == (mask & (*status)))
		(*t0) = Clock::now() ;
	    (*t1) = Clock::now() ;
	} ;

    auto t0=Clock::now() ; decltype(t0) t1 ;
    using Rise = Register::Event::AsyncRise0 ;
    auto rise = gpio.at<Rise>().read().value() ;
    gpio.at<Rise>().write(rise | mask) ;
    wait4edge(&t0,&t1) ;
    gpio.at<Rise>().write(rise & ~mask) ;
    
    auto t2 = t1 ; decltype(t2) t3 ;
    using Fall = Register::Event::AsyncFall0 ;
    auto fall = gpio.at<Fall>().read().value() ;
    gpio.at<Fall>().write(fall | mask) ;
    wait4edge(&t2,&t3) ;
    gpio.at<Fall>().write(fall & ~mask) ;

    auto t4 = t3 ; decltype(t4) t5 ;
    gpio.at<Rise>().write(rise | mask) ;
    wait4edge(&t4,&t5) ;
    gpio.at<Rise>().write(rise & ~mask) ;

    auto dt = [](Clock::time_point t0,Clock::time_point t1)
    {
	return std::chrono::duration<double>(t1-t0).count() ;
    } ;
    
    std::cout.setf(std::ios::scientific) ;
    std::cout.precision(2) ;
    std::cout <<         0 << '+' << dt(t0,t1) << ' '
	      << dt(t0,t2) << '+' << dt(t2,t3) << ' '
	      << dt(t0,t4) << '+' << dt(t4,t5) << '\n' ;
}

// --------------------------------------------------------------------

struct Record
{
    uint32_t t0,t1,levels ;
    Record(uint32_t t0,uint32_t t1,uint32_t levels)
    : t0(t0),t1(t1),levels(levels) {}
    Record() : t0(0),t1(0),levels(0) {}
} ;
    
static void watch(Rpi::Peripheral *rpi,Ui::ArgL *argL) 
{
    auto pins = getPins(argL) ;
    auto nsamples = Ui::strto<unsigned>(argL->pop()) ;
    argL->finalize() ;
  
    auto input = rpi->at<Rpi::Register::Gpio::Input::Bank0>().value() ;
    Rpi::ArmTimer timer(rpi) ;
    std::vector<Record> v(0x10000) ; v.resize(0) ;
  
    auto t0 = timer.counter().read() ;
    auto levels = pins & (*input) ;
    auto t1 = timer.counter().read() ;
    v.push_back(Record(0,t1-t0,levels)) ;
  
    for (decltype(nsamples) i=0 ; i<nsamples ; ++i)
    {
	auto ti = timer.counter().read() ;
	auto next = pins & (*input) ;
	if (levels != next)
	{
	    auto tj = timer.counter().read() ;
	    levels = next ;
	    v.push_back(Record(ti-t0,tj-ti,levels)) ;
	    t0 = ti ;
	}
    }

    std::cout << mkhdr(pins) << '\n'
	      << mksep(pins) << '\n' ;
    for (auto &r: v)
    {
	std::cout << mkstr(pins,r.levels) << r.t0 << ' ' << r.t1 << '\n' ;
    }
}

// --------------------------------------------------------------------

void Console::Sample::invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (argL->empty() || argL->peek() == "help")
    { 
	std::cout
	    << "arguments: MODE PIN NSAMPLES\n"
	    << '\n'
	    << "displays the sampling rate (f) and the signal rate" 
	    << '\n'
	    << "MODE : count [-d] # count number of subsequent events\n"
	    << "     | duty  [-d] # determine signal's duty cycle\n"
	    << "     | pulse      # determine signal's pulse length\n"
	    << "     | watch      # ?\n"
	    << '\n'
	    << "-d : dry run to determine maximum sampling rate f\n"
	    ;
	return ;
    }

    std::map<std::string,void(*)(Rpi::Peripheral*,Ui::ArgL*)> map =
    {
	{ "count" , count },
	{ "duty"  ,  duty },
	{ "pulse" , pulse },
	{ "watch" , watch },
    } ;
    argL->pop(map)(rpi,argL) ;
    
}
