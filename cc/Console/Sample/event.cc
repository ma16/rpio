// BSD 2-Clause License, see github.com/ma16/rpio

// see README.md for details

#include "invoke.h"
#include <Rpi/Pin.h>
#include <Rpi/Register.h>
#include <Ui/strto.h>
#include <chrono>
#include <iomanip>
#include <iostream>

using Clock = std::chrono::steady_clock ;

using Duration = std::chrono::duration<double> ;

namespace Register = Rpi::Register::Gpio ;

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
    
static void frequency(Rpi::Peripheral *rpi,Ui::ArgL *argL) 
{
    auto pins = getPins(argL) ;
    auto nsamples = Ui::strto<unsigned>(argL->pop()) ;
    auto dry = argL->pop_if("-d") ;
    argL->finalize() ;
    auto status = rpi->at<Register::Event::Status0>() ;
    auto mask = pins ;
    decltype(nsamples) nevents = 0 ;
    decltype(nsamples) nsubseq = 0 ;
    status.write(mask) ;
    auto t0 = Clock::now() ;
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
    auto tn = Clock::now() ; 
    auto dt = Duration(tn-t0).count() ;
    std::cout.setf(std::ios::scientific) ;
    std::cout.precision(2) ;
    std::cout << "r=" << static_cast<double>(nsamples)/dt << "/s " 
	      << "f=" << static_cast<double>( nevents)/dt << "/s "
	      << "s=" << static_cast<double>( nsubseq)/dt << "/s\n" ;
}

// --------------------------------------------------------------------

struct Record
{
    Clock::duration duration ;
    unsigned count ;
} ;

static boost::optional<Record>
inspect(Rpi::Peripheral *rpi,uint32_t mask1,uint32_t mask2,unsigned n)
{
    auto mask = mask1 | mask2 ;
    auto status = rpi->at<Register::Event::Status0>() ;
    auto active = false ;
    decltype(mask) last = 0 ;
    decltype(n) count = 0 ;
    auto t0 = Clock::now() ;
    status.write(mask) ;
    for (decltype(n) i=0 ; i<n ; ++i)
    {
	auto event = status.read() & mask ;
	if (event == 0)
	{
	    active = false ;
	    continue ;
	}
	if (active)
	    return boost::none ; // we got delayed or signal is too fast
	if (event == mask)
	    return boost::none ; // we got delayed or signal is too fast
	if (event == last)
	    return boost::none ; // two falling or two rising edges in a row
	// ...[todo] defect report
	status.write(event) ;
	++count ;
	active = true ;
	last = event.value() ;
    }
    auto dt = Clock::now() - t0 ;
    return Record({dt,count}) ;
}

// [todo] we need a better name: sample event
static void inspect(Rpi::Peripheral *rpi,Ui::ArgL *argL) 
{
    auto pin1 = Ui::strto(argL->pop(),Rpi::Pin()) ;
    auto pin2 = Ui::strto(argL->pop(),Rpi::Pin()) ;
    auto nsamples = Ui::strto<uint32_t>(argL->pop()) ;
    argL->finalize() ;

    // [todo] check pin1 != pin2
    auto mask1 = 1u << pin1.value() ;
    auto mask2 = 1u << pin2.value() ;

    auto t0 = Clock::now() ;
    auto i = 0u ;
  Loop:
    ++i ;
    auto record = inspect(rpi,mask1,mask2,nsamples) ;
    {
	auto t1 = Clock::now() ;
	auto dt = Duration(t1-t0).count() ;
	if (!record && dt < 1.0)
	    goto Loop ;
    }

    if (!record)
    {
	std::cout << "failed\n" ;
	return ;
    }
    
    std::cout.setf(std::ios::scientific) ;
    std::cout.precision(2) ;
    
    auto dt = Duration(record->duration).count() ;
    std::cout
	<< i << ' '
	<< "r=" << nsamples/dt << "/s " 
	<< "f=" << record->count/dt/2 << "/s\n"
	;
}

// --------------------------------------------------------------------

// sample single pulse
static void pulse(Rpi::Peripheral *rpi,Ui::ArgL *argL) 
{
    auto pin = Ui::strto(argL->pop(),Rpi::Pin()) ;
    argL->finalize() ;
    
    auto mask = 1u << pin.value() ;
    
    auto gpio = rpi->page<Register::PageNo>() ;

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
	return Duration(t1-t0).count() ;
    } ;
    
    std::cout.setf(std::ios::scientific) ;
    std::cout.precision(2) ;
    std::cout <<         0 << '+' << dt(t0,t1) << ' '
	      << dt(t0,t2) << '+' << dt(t2,t3) << ' '
	      << dt(t0,t4) << '+' << dt(t4,t5) << '\n' ;
}

// --------------------------------------------------------------------

void Console::Sample::invoke_event(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (argL->empty() || argL->peek() == "help")
    { 
	std::cout
	    << "arguments: MODE PIN NSAMPLES\n"
	    << '\n'
	    << "displays the sampling rate (f) and the signal rate" 
	    << '\n'
	    << "MODE : frequency\n"
	    << "     | inspect  \n"
	    << "     | pulse    \n"
	    << '\n'
	    << "-d : dry run to determine maximum sampling rate f\n"
	    ;
	return ;
    }

    std::map<std::string,void(*)(Rpi::Peripheral*,Ui::ArgL*)> map =
    {
	{ "frequency" , frequency },
	{ "inspect" , inspect },
	{ "pulse" , pulse },
    } ;
    argL->pop(map)(rpi,argL) ;
    
}
