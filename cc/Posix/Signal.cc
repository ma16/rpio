// BSD 2-Clause License, see github.com/ma16/rpio

#include "Signal.h"
#include "base.h"
#include "Error.h"
#include <signal.h>
#include <sstream>

void Posix::Signal::block(int signal)
{
    sigset_t set ;
    sigemptyset(&set) ;
    auto result = sigaddset(&set,signal) ;
    if (result != 0)
    {
	std::ostringstream os ;
	os << "sigaddset(" << signal << ") failed:" << Posix::strerror(errno) ;
	throw Posix::Error(os.str()) ;
    }
    result = sigprocmask(SIG_BLOCK,&set,nullptr) ;
    if (result != 0)
    {
	std::ostringstream os ;
	os << "sigprocmask(SIG_BLOCK," << signal << ") failed:" << Posix::strerror(errno) ;
	throw Posix::Error(os.str()) ;
    }
}

bool Posix::Signal::pending(int signal)
{
    sigset_t set ;
    sigemptyset(&set) ;
    auto result = sigpending(&set) ;
    if (result != 0)
    {
	std::ostringstream os ;
	os << "sigpending(" << signal << ") failed: "<< Posix::strerror(errno) ;
	throw Posix::Error(os.str()) ;
    }
    result = sigismember(&set,signal) ;
    if (result==0 || result==1)
	return result ;
    std::ostringstream os ;
    os << "sigismember(" << signal << ") failed:" << Posix::strerror(errno) ;
    throw Posix::Error(os.str()) ;
}
