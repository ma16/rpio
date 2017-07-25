// BSD 2-Clause License, see github.com/ma16/rpio

#include "Pwm.h"
#include <cassert>
#include <chrono>
#include <iostream> // [debug]
#include <Neat/chrono.h>

void RpiExt::Pwm::start()
{
    // [todo] clear fifo; clear errors
    this->pwm.setRange(this->index,32) ;
    auto c = this->pwm.getControl() ;
    auto x = c.get(this->index) ;
    x.mode = 1 ;
    x.pwen = 1 ;
    x.usef = 1 ;
    c.set(index,x) ;
    this->pwm.setControl(c) ;
}
  
void RpiExt::Pwm::wait()
{
    while (0 == this->pwm.getStatus().cempt())
	;
    // [defect] 
    // There seems to be no way to figure out when the PWM peripheral
    // actually completes processing of all the FIFO entries. Even if
    // the status flag indicates an empty FIFO, the serializer may
    // still be busy. As it appears, the serializer reads two FIFO
    // entries ahead.
    // So, what the client needs to do, is to put two additional words
    // into the FIFO. These words may be transmitted, or not. The
    // client should keep that in mind and set the levels
    // appropriately. Also, the last word is always transmitted again
    // if the FIFO runs empty (and if the serializer gets to this point
    // at all).
    // [todo] document this defect properly
    auto c = this->pwm.getControl() ;
    auto x = c.get(index) ;
    x.pwen = 0 ;
    c.set(index,x) ;
    this->pwm.setControl(c) ;
}
  
void RpiExt::Pwm::send(std::vector<uint32_t> const &v)
{
    if (v.size() < 16)
    {
	this->fill_fifo(&v[0],v.size()) ; 
	this->start() ;
    }
    else
    {
	this->fill_fifo(&v[0],16) ; 
	this->start() ;
	this->send_fifo(&v[16],v.size()-16) ;
    }
    this->wait() ;
}

void RpiExt::Pwm::fill_fifo(uint32_t const*p,unsigned n)
{
    for (decltype(n) i=0 ; i<n ; ++i) 
	this->pwm.write(p[i]) ;
    assert(0 == this->pwm.getStatus().cwerr()) ; // [todo] throw
}

void RpiExt::Pwm::send_fifo(uint32_t const *rgb,unsigned n)
{
    for (decltype(n) i=0 ; i<n ; ++i)
    {
	auto t0 = this->timer.cLo() ;
	while (0 != this->pwm.getStatus().cfull())
	    ;
	if (0 != this->pwm.getStatus().cempt())
	{
	    auto t1 = this->timer.cLo() ;
	    auto c = this->pwm.getControl() ;
	    auto x = c.get(index) ;
	    x.pwen = 0 ;
	    c.set(index,x) ;
	    this->pwm.setControl(c) ;
	    throw std::runtime_error("Pwm:unexpected empty FIFO:" + std::to_string(i) + " " + std::to_string(t1-t0)) ;
	}
	this->pwm.write(rgb[i]) ;
	if (0 != this->pwm.getStatus().cwerr())
	    throw std::runtime_error("Pwm:write error even if not full") ;
    }
}

#if 0
double RpiExt::Pwm::frequency(double seconds)
{
    auto span = static_cast<uint32_t>(seconds * 1000 * 1000 + .5) ;
    
    auto t0 = this->timer.cLo() ;
    
    // wait till fifo got empty [optional]
    while (0 == this->pwm.getStatus().cempt()) ;
    {
	if (this->timer.cLo() - t0 >= span)
	    throw Error("timeout:empty out") ;
	// either the given duration was too short, or the frequency is
	// zero, maybe, because the clock-manager wasn't enabled at all.
    }
    
    // fill fifo [todo] pattern by client
    while (0 == this->pwm.getStatus().cfull())
	this->pwm.write(0x55555555) ;

    uint32_t t2 ;
    do // wait till one word got emptied out
    {
	t2 = this->timer.cLo() ; 
	if (t2 - t0 >= span)
	    throw Error("timeout:start-word") ;
    }
    while (0 != this->pwm.getStatus().cfull()) ;
    
    // right now began the serialization of a new (long) word
    
    auto count = 0 ;
    uint32_t t3 ;
    do // wait till duration has lapsed (while topping up fifo)
    {
	auto status = this->pwm.getStatus() ;
	if (0 == status.cfull())
	{
	    if (0 != status.cempt())
		throw Error("fifo underrun") ;
	    // ...[note] there is actually no safe way to detect an underrun
	    this->pwm.write(0x55555555) ; // [todo] configurable by client
	    ++count ;
	}
	t3 = this->timer.cLo() ;
    }
    while (t3 - t0 <= span) ;

    // [todo] recover from exceptions !?

    // don't fill the queue before PWM is enabled; it appears to
    // immediately deque two fifo entries which messes up our timing
    
    // start
    // clock: (resolution, granularity, cost)
    
    // the PWM might be in the middle of a word, which may mess up
    // our measurement (for long words) since ti assumes that
    // the complete word was serialized.
    while (0 != this->pwm.getStatus().cfull())
	;
    t3 = this->timer.cLo() ;
    auto elapsed = (t3 - t2) / 1000.0 / 1000.0 ;
    auto nbits = this->pwm.getRange(this->index) ;
    auto freq = static_cast<double>(nbits) * count / elapsed ;
    std::cerr << "### " << elapsed << ' ' << count << std::endl ;
    return freq ;
}

send()
{
    stop() ; clear() ; reset() ;

    fill_fifo(buffer) ; get_time(t0) ; start() ;

    until completed:
    
	top_up(buffer) ; get_time(ti) ; verify(ti-t0/i) ;

    top_up(padding) till fifo is full ; get_time(ti) ; verify(ti-t0/i) ;

    // todo: silent-bit; repeat-last-word; idle-bit <-- test cases
    
    stop ; clear ; reset ;
}
#endif

std::pair<size_t,uint32_t> RpiExt::Pwm::
top_up(uint32_t const buffer[],uint32_t timeout)
{
    size_t i = 0 ;
    
    // fill fifo until full
    while (0 == this->pwm.getStatus().cfull())
	this->pwm.write(buffer[i++]) ;

    // wait until the full-flag turns to false (@t1)
    auto t0 = this->timer.cLo() ;
    auto t1 = t0 ;
    while (0 != this->pwm.getStatus().cfull()) 
    {
	if (t1 - t0 >= timeout)
	    throw Error("top_up:timeout") ;
	// possible reasons:
	// * the given time span was too short
	// * pwm wasn't enabled
	// * no or invalid setup of the clock-manager
	t1 = this->timer.cLo() ; 
    }

    // fill the empty space
    this->pwm.write(buffer[i++]) ;
    return std::make_pair(i,t1) ;
}

std::pair<size_t,uint32_t> RpiExt::Pwm::
top_up(uint32_t const buffer[])
{
    size_t i = 0 ;
    
    // fill fifo until full
    while (0 == this->pwm.getStatus().cfull())
	this->pwm.write(buffer[i++]) ;

    // wait until the full-flag turns to false (@t1)
    auto t1 = this->timer.cLo() ;
    while (0 != this->pwm.getStatus().cfull())
	;
    t1 = this->timer.cLo() ; 
    this->pwm.write(buffer[i++]) ;
    return std::make_pair(i,t1) ;
}

double RpiExt::Pwm::frequency(double seconds)
{
    auto span = static_cast<uint32_t>(seconds * 1000.0 * 1000.0 + .5) ;

    static uint32_t const block[16] = { 0x55555555 } ;
    
    auto t0 = this->top_up(block,span).second ;

    // first block
    auto tuple = top_up(block,span) ;
    auto n = tuple.first ;
    auto ti = tuple.second ;

    while (ti - t0 < span)
    {
	// subsequent blocks
	tuple = top_up(block) ;
	n += tuple.first ;
	ti = tuple.second ;
    }

    auto elapsed = (ti - t0) / 1000.0 / 1000.0 ;
    auto nbits = this->pwm.getRange(this->index) ;
    auto freq = static_cast<double>(nbits) * n / elapsed ;
    std::cerr << "### " << elapsed << ' ' << n << std::endl ;
    return freq ;
}

// to contemplate:
// * clock: (resolution, granularity, cost)
//
// * timing:
//   the serializer may be at any point in a (long) word when a time-
//   stamp is taken. hence, the time-stamp should be taken the moment
//   the FIFO's flag _full_ turns to false (zero).
// (be aware, otherwise it may mess up the timing/measuring)
//
// [note] the moment that pwm is enabled, the serializer appears to
// dequeue immediately two entries from the fifo. [todo] test case.
// (be aware, otherwise it may mess up the timing/measuring)
//
// [note] there is actually no safe way to detect an underrun
//
// [todo] recover from exceptions!?

