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

double RpiExt::Pwm::frequency(double seconds)
{
    // ...well, "exactly" might be rather fuzzy in userland
    
    // [todo] channel parameter
    // backup pwm settings
    auto saved_ctrl = this->pwm.getControl() ;
    // prepare pwm test
    auto nbits = this->pwm.getRange(this->index) ;

    {
	auto c = Rpi::Pwm::Control() ;
	auto x = c.get(this->index) ;
	x.pwen = 0 ;
	c.set(this->index,x) ;
	c.clrf1() = 1 ; 
	this->pwm.setControl(c) ;
	// [defect] don't call setControl() twice in a row
    }
    
    this->pwm.resetStatus(this->pwm.getStatus()) ;
    
    // We cannot determine when a pwm transmission is completed
    // (neither by sta, gap nor empt). Hence:
    // * we fill the fifo until full
    // * get the current time t0
    // * start transmission and top up the fifo for a while
    // * top up the fifo until full
    // * get the current time ti

    // [todo] that should work much better with DMA
    
    // start pwm
    auto c = Rpi::Pwm::Control() ;
    auto x = c.get(this->index) ;
    x.mode = 1 ; // serialize
    x.usef = 1 ; 
    x.sbit = 1 ; // (for debugging/monitoring and nbits>32)
    x.pwen = 1 ;
    c.set(this->index,x) ;
    this->pwm.setControl(c) ;

    // fill queue
    this->pwm.write(0x55000033) ; // [todo]
    while (0 == this->pwm.getStatus().cfull())
	this->pwm.write(0x55555555) ;
    // [todo] make sure that's not indefinite!!

    // don't fill the queue before PWM is enabled; it appears to
    // immediately deque two fifo entries which messes up our timing
    
    // start
    //using clock = std::chrono::steady_clock ;
    //using clock = std::chrono::high_resolution_clock ;
    //auto span = Neat::chrono::bySeconds<clock::duration>(seconds) ;
    //auto t0 = clock::now() ;
    auto t0 = this->timer.cLo() ;
    auto span = static_cast<uint32_t>(seconds * 1000 * 1000 + .5) ;
    // ...[todo] maybe the ARM counter is the better choice?

    // write pwm queue until time has passed
    auto count = 0u ;
    auto ti = t0 ;
    uint32_t mask = 0 ;
    while (ti - t0 < span)
    {
	auto c = 0u ;
	while (true)
	{
	    auto status = this->pwm.getStatus() ;
	    if (0 != status.cfull())
		break ;
	    if (0 != status.cempt())
		throw Error("fifo underrun") ;
	    // ...[note] there is actually no safe way to detect an underrun
	    // ...[todo] recover
	    this->pwm.write(mask) ; mask = ~mask ;
	    if (++c < 0x100)
		break ;
	}
	count += c ;
	//ti = clock::now() ;
	ti = this->timer.cLo() ;
    }

    double freq ;
    if ((count == 0) && (0 != this->pwm.getStatus().cfull()))
    {
	// either the given duration was too short, or the frequency is
	// zero, for instance because the clock-manager isn't enabled.
	freq = 0.0 ;
    }
    else
    {
	// the PWM might be in the middle of a word, which may mess up
	// our measurement (for long words) since ti assumes that
	// the complete word was serialized.
	while (0 != this->pwm.getStatus().cfull())
	    ;
	++count ;
	//ti = clock::now() ;
	ti = this->timer.cLo() ;
	//auto elapsed = Neat::chrono::toSeconds(ti - t0) ;
	auto elapsed = (ti - t0) / 1000.0 / 1000.0 ;
	freq = static_cast<double>(nbits) * count / elapsed ;
	std::cerr << "### " << elapsed << ' ' << count << std::endl ;
    }
    // recover
#if 0    
    x.pwen = 0 ;
    c.set(this->index,x) ;
    c.clrf1() = 1 ;
    this->pwm.setControl(c) ;
    // it appears that two "subsequent" writes to the control register
    // may result in a BERR and have weird consequences (one or the
    // other command fails) [observed on Pi-2]
#endif    
    this->pwm.resetStatus(this->pwm.getStatus()) ;
    this->pwm.setControl(saved_ctrl) ;
    return freq ;
}

