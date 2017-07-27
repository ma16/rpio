// BSD 2-Clause License, see github.com/ma16/rpio

#include "Pwm.h"
#include <cassert>
#include <iostream> // [debug]

bool RpiExt::Pwm::fillUp(size_t n,uint32_t word)
{
    auto status = this->pwm.getStatus() ;
    this->pwm.resetStatus(status) ;
    // ...[todo] just reset the werr
    for (decltype(n) i=0 ; i<n ; ++i)
	this->pwm.write(word) ; 
    status = this->pwm.getStatus() ;
    if (0 == status.cwerr())
	// [todo] just reset the werr
	this->pwm.resetStatus(status) ; 
    return 0 != status.cwerr() ;
}

size_t RpiExt::Pwm::topUp(uint32_t const buffer[],size_t nwords)
{
    for (decltype(nwords) i=0 ; i<nwords ; ++i)
    {
	if (0 != this->pwm.getStatus().cfull())
	    return i ;
	this->pwm.write(buffer[i]) ;
    }
    return nwords ;
}

bool RpiExt::Pwm::wait(uint32_t timeout)
{
    auto t0 = this->timer.cLo() ; 
    while (0 != this->pwm.getStatus().cfull())
    {
	if (this->timer.cLo() - t0 >= timeout)
	    return false ;
    }
    return true ;
}

void RpiExt::Pwm::write(uint32_t const buffer[],size_t nwords)
{
    for (decltype(nwords) i=0 ; i<nwords ; ++i)
    {
	while (0 != this->pwm.getStatus().cfull())
	    ;
        // ...blocks indefinitely if serializer doesn't read
	this->pwm.write(buffer[i]) ;
    }
}	

size_t RpiExt::Pwm::convey(uint32_t const buffer[],size_t nwords,uint32_t pad)
{
    // keep track of number of words written to FIFO
    decltype(nwords) nwritten = 0 ;

    auto full = this->fillUp(0x20,pad) ;
    // ...0x20 is rather arbitrary, should be anything above FIFO-size
    if (!full)
	return nwritten ; // serializer too fast (underrun)

    // check if serializer is reading (optional, might be dropped here)
    bool writable = this->wait(1000000) ; 
    if (!writable)
	return nwritten ; // serializer is not working

    // top-up FIFO in blocks of FIFO-size
    while (nwords - nwritten >= 16)
    {
	auto n = this->topUp(buffer+nwritten,16) ;
	if (n == 16)
	    return nwritten ;
	nwritten += n ;
    }

    // top-up the rest 
    while (true)
    {
	auto n = this->topUp(buffer+nwritten,nwords-nwritten) ;
	if (n == nwords - nwritten)
	{
	    // pad the last block to FIFO-size
	    auto full = this->fillUp(16-n,pad) ;
	    return full ? nwords : nwritten ;
	}
	nwritten += n ;
    }
}

// --

double RpiExt::Pwm::frequency(double seconds)
{
    // we top-up the fifo (and thus, keep the FIFO full) for the given
    // duration and then calculate the frequency based on the number of
    // words we wrote to the FIFO.

    // the timer is using milli-seconds
    auto span = static_cast<uint32_t>(seconds * 1000 * 1000 + .5) ;
    // [todo] introduce time-point/duration class for the Raspberry
    // Pi's clocks (i.e. for the BCM timer and the ARM counter)

    // actually, the data we write to the FIFO doesn't matter. however,
    // if we analyze the output with a logic analyzer, the pattern may
    // become quite useful.
    static uint32_t buffer[40] = {
	0x80000000,0x88000000,0x8a000000,0x8a800000,
	0x8aa00000,0x8aa80000,0x8aaa0000,0x8aaa8000,
	0xa0000000,0xa2000000,0xa2800000,0xa2a00000,
	0xa2a80000,0xa2aa0000,0xa2aa8000,0xa2aaa000,
	0xa8000000,0xa8800000,0xa8a00000,0xa8a80000,
	0xa8aa0000,0xa8aa8000,0xa8aaa000,0xa8aaa800,
	0xaa000000,0xaa200000,0xaa280000,0xaa2a0000,
	0xaa2a8000,0xaa2aa000,0xaa2aa800,0xaa2aaa00,
	0xaa800000,0xaa880000,0xaa8a0000,0xaa8a8000,
	0xaa8aa000,0xaa8aa800,0xaa8aaa00,0xaa8aaa80 } ;
    // ...[note] feel free to change these values to a different
    // pattern that does better display on a logic analyzer

    // initial fifo fill-up
    for (unsigned i=0 ; i<40 ; ++i)
	this->pwm.write(buffer[i]) ;
    // ...[note] if the serializer is working rather fast, more
    // than 16 words may be written to the FIFO
    auto status = this->pwm.getStatus() ;
    if (0 == status.cwerr())
	// serializer is faster than we're able to write
	return std::numeric_limits<double>::infinity() ;
    this->pwm.resetStatus(status) ;
    // [todo] reset only werr
    
    auto t0 = this->timer.cLo() ; 
    decltype(t0) t1 ; // time at (t0 + span)

    // check whether the serializer is working
    while (0 != this->pwm.getStatus().cfull())
    {
	t1 = this->timer.cLo() ;
	if (t1 - t0 > span)
	    return 0 ;
	// possible reasons:
	// * the given time span was too short
	// * pwm wasn't enabled
	// * CM::PWM is not enabled
    }

    size_t nwords = 0 ; // number of words enqueued
    size_t  ngaps = 0 ; // number of (potential) fifo underruns

    // top-up the fifo and update (nwords,ngaps)
    auto enqueue = [this,&nwords,&ngaps](uint32_t const buffer[])
    {
	while (0 != this->pwm.getStatus().cfull())
	    ;
	auto n = this->topUp(buffer,24) ;
	// ...24 is quite random, 18 should work too
	nwords += n ;
	if (n > 15)
	    // since a word is in the serializer, 16 might work too
	    ++ngaps ;
    } ;
    
    do // top-up the fifo until the test-period has passed
    {
	// we switch the data for each call...
	enqueue(&buffer[ 0]) ;
	enqueue(&buffer[16]) ;
	// ...so we can see repetitions (if underrun) on the analyzer
	t1 = this->timer.cLo() ;
    }
    while (t1 - t0 <= span) ;

    auto range = this->pwm.getRange(this->index) ;
    // ...[todo] make thins function independend from channel
    auto elapsed = (t1 - t0) / 1000.0 / 1000.0 ;
    auto f = static_cast<double>(range) * nwords / elapsed ;
    return f ;

    // [todo]
    // * return ngaps so the caller knows about wrong values
    // * we might stop timing whenever a gap is encountered
}

// --------------------------------------------------------------------

#if 0

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
	    throw Error("Pwm:unexpected empty FIFO:" +
			std::to_string(i) + " " + std::to_string(t1-t0)) ;
	}
	this->pwm.write(rgb[i]) ;
	if (0 != this->pwm.getStatus().cwerr())
	    throw std::runtime_error("Pwm:write error even if not full") ;
    }
}

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

// --------------------------------------------------------------------

void RpiExt::Pwm::send(uint32_t const buffer[],size_t nwords,double f)
{
    // first block
    auto t0 = this->top_up(&buffer[0],100000/*todo*/).second ;

    // first block again
    auto tuple = top_up(&buffer[0]) ;
    auto n = tuple.first ;
    auto ti = tuple.second ;

    if (1e+6 * (32*n+2) / f < ti-t0)
	throw Error("timeout") ;

    while (n < nwords)
    {
	// subsequent blocks
	tuple = top_up(&buffer[n]) ;

	auto  n_x = tuple.first ;
	auto ti_x = tuple.second ;
	    
	if (n_x > 15)
	{
	    if (1e+6 * (32*n_x+2) / f < ti_x - ti)
	    {
		std::cerr << "### "
			  << (ti_x - ti) << " "
			  << n_x << " "
			  << n << std::endl ;
		throw Error("timeout") ;
	    }
	}
	    
	n += tuple.first ;
	ti = tuple.second ;
    }

    std::cerr << "### "
	      << (ti - t0) << " "
	      << n << " "
	      << (32e+6 * n / (ti-t0)) << std::endl ;
    // [todo] wait till finished!?
}

void RpiExt::Pwm::send(uint32_t const buffer[],size_t nwords)
{
    auto t0 = this->top_up(&buffer[0],100000/*todo*/).second ;

    // first block
    auto tuple = top_up(&buffer[0]) ;
    auto n = tuple.first ;
    auto ti = tuple.second ;

    while (n < nwords)
    {
	// subsequent blocks
	tuple = top_up(&buffer[n]) ;
	n += tuple.first ;
	ti = tuple.second ;
    }

    std::cerr << "### "
	      << (ti - t0) << " "
	      << n << " "
	      << (static_cast<double>(n)/(ti-t0)) << std::endl ;
    // [todo] wait till finished!?
}

void RpiExt::Pwm::send(std::vector<uint32_t> const &v)
{
    this->send(&v[0],v.size()) ;
}
#endif

// --------------------------------------------------------------------

#if 0
std::pair<size_t,uint32_t> RpiExt::Pwm::
topUp(uint32_t const buffer[],size_t nwords,uint32_t timeout)
{
    // A time-stamp can be severely inaccurate if the thread gets
    // suspended between the "event" and taking the associated time-
    // stamp.
    //
    // Here we try to determine the time the fifo gets full. 
    
    size_t i = 0 ;

  Again: ;

    if (i == nwords)
	return std::make_pair(i,0) ;
    
    i += this->topUp(buffer+i,nwords-i) ;
    
    if (i == nwords)
	return std::make_pair(i,0)

    // fifo (most likely) full
    
    this->poll(timeout) ;

    // space for at least one word
    
    auto t1 = this->timer.cLo() ;
    
    this->pwm.write(buffer[i++]) ;

    // fifo full again?
    
    if (0 == this->pwm.getStatus().cfull())
	goto Again ;

    return std::make_pair(i,t1) ;
}

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
#endif

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

// PROCEED HERE
//
// ponder this: if the last time the fifo was full and now we need
// to enqueue 15 or more entries, then this may indicate an underrun,
// doesn't it!? (including false positives, still, that would be
// a quite simple implementation for problem detection)

