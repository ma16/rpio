// BSD 2-Clause License, see github.com/ma16/rpio

#include "Bang.h"
#include <chrono>
#include <iostream>
#include <Neat/cast.h>

Device::Mcp3008::Bang::Bang(
    Rpi::Peripheral *rpi,
    Rpi::Pin       csPin,
    Rpi::Pin      clkPin,
    Rpi::Pin      dinPin,
    Rpi::Pin     doutPin,
    Timing const &timing)
    : rpi(rpi),pins(Pins(csPin,clkPin,dinPin,doutPin)),timing(timing)
{
    Rpi::GpioOld gpio(this->rpi) ;    
    gpio.setMode(  csPin,Rpi::GpioOld::Mode::Out) ;
    gpio.setMode( clkPin,Rpi::GpioOld::Mode::Out) ;
    gpio.setMode( dinPin,Rpi::GpioOld::Mode::Out) ;
    gpio.setMode(doutPin,Rpi::GpioOld::Mode:: In) ;
}

Device::Mcp3008::Circuit::Sample
Device::Mcp3008::Bang::msb(Record const &record) const
{
    auto mask = this->pins.dout ;
    Device::Mcp3008::Circuit::Sample::Unsigned d ;
    d = (0 != (record.sample[0] & mask)) ? 1 : 0 ;
    for (int i=1 ; i<10 ; ++i)
    {
	d <<= 1 ;
	if (0 != (record.sample[i] & mask))
	    ++d ;
    }
    return Device::Mcp3008::Circuit::Sample::coset(d) ;
}

Device::Mcp3008::Circuit::Sample
Device::Mcp3008::Bang::lsb(Record const &record) const
{
    auto mask = this->pins.dout ;
    Device::Mcp3008::Circuit::Sample::Unsigned d ;
    d = (0 != (record.sample[18] & mask)) ? 1 : 0 ;
    for (int i=17 ; i>=9 ; --i)
    {
	d <<= 1 ;
	if (0 != (record.sample[i] & mask))
	    ++d ;
    }
    return Device::Mcp3008::Circuit::Sample::coset(d) ;
}

Device::Mcp3008::Bang::Error
Device::Mcp3008::Bang::error(Record const &record) const
{
    Error error ;
    
    error.reset_cs   = (0 == (record.resetLevel & this->pins.  cs)) ;
    error.reset_clk  = (0 != (record.resetLevel & this->pins. clk)) ;
    error.reset_din  = (0 != (record.resetLevel & this->pins. din)) ;
    error.reset_dout = (0 == (record.resetLevel & this->pins.dout)) ;
    error.recv_dout  = (0 != (record.startLevel & this->pins.dout)) ;
    error.end_dout   = (0 == (record. endLevel & this->pins.dout)) ;

    error.bled_off = (record.t_end - record.t_start > this->timing.bled) ;
    error.recv_mismatch = (this->lsb(record).value() !=
			   this->msb(record).value()) ;
    return error ;
}

Device::Mcp3008::Bang::Record
Device::Mcp3008::Bang::query(Circuit::Source source,bool monitor)
{
    Record record ;

    auto q = this->makeScript(source,monitor,&record) ;
    
    RpiExt::Bang scheduler(this->rpi) ;

    scheduler.execute(q) ;

    return record ;
} 
    
Device::Mcp3008::Bang::Script Device::Mcp3008::Bang::
makeScript(Circuit::Source source,bool monitor,Record *record)
{
    RpiExt::Bang::Enqueue q ;
    
    // +++ reset +++
    
    q.set(this->pins.cs) ; 
    
    q.time(&record->t0) ;
    
    q.reset(this->pins.din | this->pins.clk) ;
    
    q.wait(&record->t0,this->timing.csh) ;
    
    if (monitor)
	q.levels(&record->resetLevel) ;
    
    // +++ start-bit +++

    if (this->timing.sucs > this->timing.su)
    {
	q.reset(this->pins. cs) ; q.time(&record->t0) ;
	q.  set(this->pins.din) ; q.time(&record->t1) ;
    }
    else
    {
	q.  set(this->pins.din) ; q.time(&record->t1) ;
	q.reset(this->pins. cs) ; q.time(&record->t0) ;
    }
    q.wait(&record->t0,this->timing.sucs) ;
    q.wait(&record->t1,this->timing.  su) ;
    
    q.set(this->pins.clk) ;

    // +++ D3,D2,D1,D0 +++

    for (unsigned i=0 ; i<4 ; ++i)
    {
	q.sleep(this->timing.hd) ; 
	q.reset(this->pins.clk) ; 
	auto dx = 0 != (source.value() & (1u<<(3-i))) ;
	if (dx) q.set(this->pins.din) ;
	else    q.reset(this->pins.din) ;
	q.sleep(this->timing.su) ;
	q.set(this->pins.clk) ;
    }
    
    // +++ sample +++
    
    // "The device will begin to sample the analog input on the
    //  fourth rising edge of the clock after the start bit has
    //  been received. The sample period will end on the falling
    //  edge of the fifth clock following the start bit."

    q.time(&record->t_start) ;

    q.wait(&record->t_start,this->timing.hd) ;

    q.reset(this->pins.clk) ;

    q.sleep(this->timing.su) ; 

    q.set(this->pins.clk) ;

    q.sleep(this->timing.hd) ;

    q.reset(this->pins.clk) ;

    // +++ start-bit (output) +++
    
    // "On the falling edge the device will output a null bit."

    q.time(&record->t0) ;
    
    if (monitor)
    {
	q.wait(&record->t0,this->timing.en) ;
	q.levels(&record->startLevel) ;
    }

    q.wait(&record->t0,this->timing.lo) ;

    // +++ 10-bit sample (MSB first) +++
    
    // "The next 10 clocks will output the result of the conversion
    //  with MSB first. Data is always output from the device on the
    //  falling edge of the clock."

    for (int i=0 ; i<10 ; ++i)
    {
	q.set(this->pins.clk) ;

	q.sleep(this->timing.hi) ;
    
	q.reset(this->pins.clk) ;

	q.time(&record->t0) ;

	q.wait(&record->t0,this->timing.dov) ;

	q.levels(&record->sample[i]) ;

	q.wait(&record->t0,this->timing.lo) ;
    }
    
    if (monitor)
    {
	q.recent(&record->t_end) ;
	
	// +++ 10-bit sample (LSB first) +++

        // "After completing the data transfer, if further clocks are
	//  applied with THIS->PINS.CS low, the A/D converter will output LSB
	//  first data, then followed with zeros indefinitely."

	// however, the first bit is shared with the last bit above
	
	for (int i=10 ; i<19 ; ++i)
	{
	    q.set(this->pins.clk) ;

	    q.sleep(this->timing.hi) ;
    
	    q.reset(this->pins.clk) ;

	    q.time(&record->t0) ;

	    q.wait(&record->t0,this->timing.dov) ;

	    q.levels(&record->sample[i]) ;

	    q.wait(&record->t0,this->timing.lo) ;
	}
    }
    
    // +++ end dialogue +++
    
    q.set(this->pins.cs) ; q.sleep(this->timing.dis) ;
    
    if (monitor)
	q.levels(&record->endLevel) ;

    return q.vector() ;
}
