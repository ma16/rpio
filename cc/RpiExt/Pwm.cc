// BSD 2-Clause License, see github.com/ma16/rpio

#include "Pwm.h"
#include <cassert>
#include <chrono>

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

double RpiExt::Pwm::frequency(double duration)
{
    // backup pwm settings
    auto saved_ctrl = this->pwm.getControl() ;
    // prepare pwm test
    auto nbits = this->pwm.getRange(this->index) ;
    Rpi::Pwm::Control c ;
    // pwen = 0
    c.clrf1() = 1 ; 
    this->pwm.setControl(c) ;
    this->pwm.resetStatus(this->pwm.getStatus()) ;
    // fill queue
    while (0 == this->pwm.getStatus().cfull())
	this->pwm.write(0) ;
    // enable pwm
    c = Rpi::Pwm::Control() ;
    auto x = c.get(this->index) ;
    x.mode = 1 ; // serialize
    x.usef = 1 ; 
    x.sbit = 1 ; // (for debugging/monitoring and nbits>32)
    x.pwen = 1 ;
    c.set(this->index,x) ;
    using clock = std::chrono::steady_clock ;
    auto t0 = clock::now() ;
    this->pwm.setControl(c) ;
    // write pwm queue until time has passed
    using Seconds = std::chrono::duration<double,std::chrono::seconds::period> ;
    auto t1 = clock::now() ;
    auto count = 0u ;
    while (std::chrono::duration_cast<Seconds>(t1-t0).count() < duration)
    {
	auto status = this->pwm.getStatus() ;
	if (0 == status.cfull())
	{
	    if (0 != status.cempt())
		throw Error("fifo underrun") ;
	    // [note] there is actually no safe way to detect an underrun
	    this->pwm.write(0) ;
	    ++count ;
	}
	t1 = clock::now() ;
    }
    auto freq = 0.0 ;
    // ...assume that no clock is running unless at least one entry has
    // been read from the queue (which should have happened even if d=0)
    if ((count>0) || (0 == this->pwm.getStatus().cfull()))
    {
	// get clock "exactly" after queue has read
	// ...well, "exactly" might be rather fuzzy in userland
	auto status = this->pwm.getStatus() ;
	if (0 == status.cfull())
	{
	    if (0 != status.cempt())
		throw Error("fifo underrun") ;
	    this->pwm.write(0) ;
	    ++count ;
	}
	while (0 != this->pwm.getStatus().cfull())
	    ;
	t1 = clock::now() ;
	freq = static_cast<double>(nbits) * (count-1) / std::chrono::duration_cast<Seconds>(t1-t0).count() ;
    }
    // recover
    x.pwen = 0 ;
    c.set(this->index,x) ;
    c.clrf1() = 1 ;
    this->pwm.setControl(c) ;
    this->pwm.resetStatus(this->pwm.getStatus()) ;
    this->pwm.setControl(saved_ctrl) ;
    return freq ;
}

