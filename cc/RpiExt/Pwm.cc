// BSD 2-Clause License, see github.com/ma16/rpio

#include "Pwm.h"
#include <cassert>
#include <iostream> // [debug]

size_t RpiExt::Pwm::convey(uint32_t const buffer[],size_t nwords,uint32_t pad)
{
    // keep track of number of words written to FIFO
    decltype(nwords) nwritten = 0 ;

    auto full = this->fillUp(0x20,pad) ;
    // ...0x20 is rather arbitrary, should be anything above FIFO-size
    if (!full)
	return nwritten ; // serializer too fast (underrun)

    // prevent infinite loops
    if (!this->writable(1000000))
	return nwritten ; 

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

bool RpiExt::Pwm::writable(uint32_t timeout)
{
    auto t0 = this->timer.cLo() ; 
    while (0 != this->pwm.getStatus().cfull())
    {
	if (this->timer.cLo() - t0 >= timeout)
	    return false ;
	// possible reasons:
	// * the given time span was too short
	// * pwm wasn't enabled
	// * CM::PWM is not enabled
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

std::pair<double,size_t> RpiExt::Pwm::measureRate(double seconds)
{
    // we keep the FIFO full for the given duration and then return
    // number of words written divided by the time that has passed.

    // the ARM timer we use is using milli-seconds
    auto span = static_cast<uint32_t>(seconds * 1e+6 + .5) ;

    // actually, the data we write to the FIFO doesn't matter.
    // however, if we analyze the output with a logic analyzer,
    // a distinguishable pattern may become quite useful.
    static uint32_t buffer[40] = {
	0x80000002,0x88000002,0x8a000002,0x8a800002,
	0x8aa00002,0x8aa80002,0x8aaa0002,0x8aaa8002,
	0xa0000002,0xa2000002,0xa2800002,0xa2a00002,
	0xa2a80002,0xa2aa0002,0xa2aa8002,0xa2aaa002,
	0xa8000002,0xa8800002,0xa8a00002,0xa8a80002,
	0xa8aa0002,0xa8aa8002,0xa8aaa002,0xa8aaa802,
	0xaa000002,0xaa200002,0xaa280002,0xaa2a0002,
	0xaa2a8002,0xaa2aa002,0xaa2aa802,0xaa2aaa02,
	0xaa800002,0xaa880002,0xaa8a0002,0xaa8a8002,
	0xaa8aa002,0xaa8aa802,0xaa8aaa02,0xaa8aaa82 } ;
    // ...feel free to change this to other values which do display
    // *better* on a logic analyzer. don't change the number of entries
    // unless you modify the code below too.

    // flood fifo
    for (unsigned i=0 ; i<40 ; ++i)
	this->pwm.write(buffer[i]) ;
    // ...more than 16 words may be written to the FIFO if the
    // serializer is reading fast. Using exactly 40 write operations
    // here is arbitrary, though.
    auto status = this->pwm.getStatus() ;
    if (0 == status.cwerr())
	// either the serializer reads faster than we're able to write
	// (likey) or we got suspended more than once (very unlikely)
	return std::make_pair(std::numeric_limits<double>::infinity(),0) ;
    this->pwm.resetStatus(status) ;
    // [todo] reset only werr

    // set-up statistical data
    auto t0 = this->timer.cLo() ; 
    decltype(t0) t1 ; // time at (t0 + span)
    size_t nwords = 0 ; // number of words enqueued
    size_t  ngaps = 0 ; // number of (potential) fifo underruns

    if (!this->writable(span))
	return std::make_pair(0.0,0) ;

    // local function: top-up the fifo and update (nwords,ngaps)
    auto enqueue = [this,&nwords,&ngaps](uint32_t const buffer[])
    {
	while (0 != this->pwm.getStatus().cfull())
	    ;
	auto n = this->topUp(buffer,24) ;
	// ...24 is quite random, 16 should work too
	nwords += n ;
	if (n >= 16)
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

    return std::make_pair(1e+6 * nwords / (t1 - t0),ngaps) ;

    // [todo] we might stop the timing whenever a gap is encountered
}

// [notes]
//
// If taking a time-stamp, the serializer may be at any point in a
// (long) word. hence, the time-stamp should be taken the moment
// the FIFO's _full_ flag turns to false. This may become a non-issue
// if there are the odds of process suspensions anyway. Still, if there
// is a sequence (full=1,time-stamp,full=1) the point of time is
// guaranteed to be in the period that takes a word to serialize.
//
// There seems to be no way to figure out when the PWM peripheral
// actually finishes processing of all the FIFO entries. Even if
// the status flag indicates an empty FIFO, the serializer may
// still be busy. Sometimes it appears as if the serializer reads two
// FIFO entries ahead. [provide test-case]
//
// So, what the client needs to do, is to put two additional words
// into the FIFO. These words may be transmitted, or not. The
// client should keep that in mind and set the levels
// appropriately. Also, the last word is always transmitted again
// and again [regardless of the RPT flag] if the FIFO runs empty (and
// if the serializer gets to this point at all).