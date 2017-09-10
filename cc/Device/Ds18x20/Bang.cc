// BSD 2-Clause License, see github.com/ma16/rpio

#include <Device/Ds18x20/Bang.h>
#include <cassert>

static std::string toStr(Device::Ds18x20::Bang::Error::Type type,int line)
{
    static char const *v[] = { "NotPresent","Retry","Timing","Vanished" } ;
    auto s = v[Device::Ds18x20::Bang::Error::TypeN(type).n()] ;
    std::ostringstream os ;
    os << "Device:Ds18x20:Bang: " << s << " #" << line ;
    return os.str() ;
}

Device::Ds18x20::Bang::Error::Error(Type type,int line)
    : Neat::Error(toStr(type,line)),type_(type)
{
    // void
}

constexpr Device::Ds18x20::Bang::Timing<double> Device::Ds18x20::Bang::spec ;

bool Device::Ds18x20::Bang::init()
{
    //  t0 t1 t2 t3 t4 t5 t6 t7
    // ---+     +-----+     +----...
    //    |     |     |     |
    //    +-----+     +-----+
    
    // tx: Reset-Pulse
    this->io.mode(this->busPin,Rpi::Gpio::Mode::Out) ;
    // ...assumes the configured output level is Low
    // ...note, errors must not be thrown as long as Out or Events enabled
    this->io.sleep(this->timing.rstl) ;
    auto v = this->intr.status() ;
    // https://www.raspberrypi.org/forums/viewtopic.php?f=66&t=192908
    this->intr.disable(v) ;
    this->io.detect(this->busPin,Rpi::Gpio::Event::Fall) ;
    // the event status bit gets immediately set (todo: why?)
    this->io.events(this->pinMask) ;
    auto t2 = this->io.recent() ;
    this->io.mode(this->busPin,Rpi::Gpio::Mode::In) ;
    auto t3 = this->io.time() ;
    // ...if we got suspended, t3 and following time-stamps may lay
    // even behind the end of the Presence-Pulse (if there was any)

    auto isPresent = 0 != this->io.waitForEvent(t3,
						this->timing.pdhigh.max,
						this->pinMask) ;
    auto t4 = this->io.recent() ;
    auto t5 = this->io.time() ;
    this->io.detect(this->busPin,Rpi::Gpio::Event::Fall,false) ;
    this->io.events(this->pinMask) ; // reset late events
    this->intr.enable(v) ;

    if (isPresent)
    {
	if (t4 == t3)
	    throw Error(Error::Type::Retry,__LINE__) ;
	if (t5 - t2 < this->timing.pdhigh.min)
	    throw Error(Error::Type::Timing,__LINE__) ;

	// rx: wait for LH-edge (end of Presence-Pulse)
	this->io.waitForLevel(t5,this->timing.pdlow.max,
			      this->pinMask,/*High*/this->pinMask) ;
	auto t7 = this->io.recent() ;
	if (t7 - t4 < this->timing.pdlow.min)
	    throw Error(Error::Type::Timing,__LINE__) ;
	
	// note: we don't check against max values at the moment
	// because the results may be false positives (due to delays
	// when recording the time-stamps). in order to get a better
	// grip we may want to check the gap when recording the time
	// before and after every (!) edge.
    }
    
    // rx: wait for end of Presence-Pulse cycle
    this->io.wait(t3,this->timing.rsth) ;

    return isPresent ;
}

void Device::Ds18x20::Bang::write(bool bit) 
{
    //  t0 t1 t2 t3
    // ---+     +---
    //    |     |
    //    +-----+

    // tx: short or long Bit Pulse
    auto t0 = this->io.time() ;
    this->io.mode(this->busPin,Rpi::Gpio::Mode::Out) ;
    auto t1 = this->io.time() ;
    auto range = bit ? this->timing.low1 : this->timing.low0 ;
    this->io.wait(t1,range.min) ; 
    this->io.mode(this->busPin,Rpi::Gpio::Mode::In) ;
    auto t3 = this->io.time() ; 
    if (t3 - t0 > range.max) 
	throw Error(Error::Type::Retry,__LINE__) ;

    // minimum recovery time (after LH edge)
    this->io.wait(t3,this->timing.rec) ;
	
    // rx: wait for end of cycle
    this->io.wait(t1,this->timing.slot.min) ;
}

void Device::Ds18x20::Bang::write(uint8_t byte)
{
    for (auto i=0u ; i<8u ; ++i)
    {
	auto bit = 0 != (byte & (1u<<i)) ;
	this->write(bit) ;
    }
}

bool Device::Ds18x20::Bang::read()
{
    //  t0 t1 t2 t3 t4 t5
    // ---+     +-----+---
    //    |     |     |   
    //    +-----+-----+

    // tx: initiate Read-Time-Slot
    auto t0 = this->io.time() ; 
    this->io.mode(this->busPin,Rpi::Gpio::Mode::Out) ;
    auto t1 = this->io.time() ; 
    this->io.wait(t1,this->timing.rinit) ;
    // the device keeps holding the wire low in order to "send" a
    // 0-bit; there is no LH-HL gap as long as the pulse to initiate
    // the Read-Time-Slot is long enough
    this->io.mode(this->busPin,Rpi::Gpio::Mode::In) ;
    this->io.sleep(this->timing.rrc) ;
    auto sample_t3 = 0 != (this->pinMask & this->io.levels()) ;
    auto t3 = this->io.time() ; 
    // ...if we got suspended, t3 and following time-stamps may lay
    // far behind the end of the 0-bit Pulse (if there was any)
    if (t3 - t0 > this->timing.rdv) 
	throw Error(Error::Type::Retry,__LINE__) ;
    // ...this does also cover t3 - t0 > rstl

    // rx: wait for LH edge 
    this->io.waitForLevel(t1,this->timing.slot.max,
			  this->pinMask,/*High*/this->pinMask) ;
    auto t5 = this->io.recent() ;
    auto timeout = t5 - t1 >= this->timing.slot.max ;
    if (timeout)
	throw Error(Error::Type::Retry,__LINE__) ;

    auto sample_rdv = (t5 - t1) <= this->timing.rdv ;
    if (sample_t3 != sample_rdv)
	throw Error(Error::Type::Retry,__LINE__) ;

    // minimum recovery time (after LH edge)
    this->io.wait(t5,this->timing.rec) ;
	
    // rx: wait for end of cycle
    this->io.wait(t1,this->timing.slot.min) ;

    return sample_t3 ;
}

void Device::Ds18x20::Bang::read(size_t nbits,bool *bitA)
{
    for (auto i=0u ; i<nbits ; ++i)
    {
	bitA[i] = this->read() ;
    }
}

void Device::Ds18x20::Bang::convert()
{
    auto feedback = this->init() ;
    if (!feedback)
	throw Error(Error::Type::NotPresent,__LINE__) ;
    // ROM-command: Skip-ROM-Code
    this->write(static_cast<uint8_t>(0xcc)) ;
    // Function-command: Convert-T
    this->write(static_cast<uint8_t>(0x44)) ;
    // [todo]
    //   here we can issue Read-Time-Slot until 1;
    //   we would need our script to loop
}

Device::Ds18x20::Bang::Pad Device::Ds18x20::Bang::readPad()
{
    auto feedback = this->init() ;
    if (!feedback)
	throw Error(Error::Type::NotPresent,__LINE__) ;
    // ROM-command: Skip-ROM-Code
    this->write(static_cast<uint8_t>(0xcc)) ;
    // Function-command: Read-Sratch-Pad
    this->write(static_cast<uint8_t>(0xbe)) ;
    bool rx[72] ;
    Bang::read(72,rx) ;
    Pad pad ;
    for (auto i=0u ; i<72 ; ++i)
	pad[i] = rx[i] ;
    return pad ;
}

bool Device::Ds18x20::Bang::isBusy()
{
    return !this->read() ;
}

bool Device::Ds18x20::Bang::isPowered()
{
    auto feedback = this->init() ;
    if (!feedback)
	throw Error(Error::Type::NotPresent,__LINE__) ;
    // ROM-command: Skip-ROM-Code
    this->write(static_cast<uint8_t>(0xcc)) ;
    // Function-command: Read-Power-Supply
    this->write(static_cast<uint8_t>(0xb4)) ;
    // Read-Time-Slot returns power mode
    return this->read() ;
}

// ----

boost::optional<Device::Ds18x20::Bang::Address> Device::Ds18x20::Bang::
address()
{
    auto feedback = this->init() ;
    if (!feedback)
	return boost::none ;
    // ROM-command: Read-ROM-Code
    this->write(static_cast<uint8_t>(0x33)) ;
    bool rx[64] ;
    this->read(64,rx) ;
    Address address ;
    for (auto i=0u ; i<64 ; ++i)
	address[i] = rx[i] ;
    return address ;
}

void Device::Ds18x20::Bang::complete(Address *address,size_t offset)
{
    for (auto i=offset ; i<64 ; ++i)
    {
	auto bit = this->read() ;
	auto inv = this->read() ;

	if ((bit == 1) && (inv == 1))
	    throw Error(Error::Type::Vanished,__LINE__) ;
	
	// if 0-1: address bit is 1 (for all attached devices)
	// if 1-0: address bit is 0 (for all attached devices)
	// if 0-0: attached devices split into 0 and 1 addresses
	this->write(bit) ;
	// ...proceed with the lowest address bit (if more than one)
	
	(*address)[i] = bit ;
    }
}

unsigned Device::Ds18x20::Bang::scan(Address const &address)
{
    auto feedback = this->init() ;
    if (!feedback)
	throw Error(Error::Type::Vanished,__LINE__) ;
    // ROM-command: Search Read-ROM-Code
    this->write(static_cast<uint8_t>(0xf0)) ;
    // [todo] support Search Alarm ROM code too

    auto branch = 64u ; // default: no branches found
    
    for (auto i=0u ; i<64 ; ++i)
    {
	auto bit = this->read() ;
	auto inv = this->read() ;

	if ((bit == 1) && (inv == 1))
	    throw Error(Error::Type::Vanished,__LINE__) ;
	if (bit != inv) // (0,1) and (1,0)
	{
	    if (address[i] != bit)
		throw Error(Error::Type::Vanished,__LINE__) ;
	}
	else // (0,0)
	{
            // attached devices split into 0 and 1 addresses
	    if (address[i] == 0)
	    {
		// address is in the low-address-branch, so we might
		// wanna branch here (unless there are other branches)
		branch = i ;
	    }
	}

	this->write(address[i]) ;
    }
    return branch ;
}

void Device::Ds18x20::Bang::track(Address const &address,size_t nbits)
{
    for (auto i=0u ; i<nbits ; ++i)
    {
	auto bit = this->read() ;
	auto inv = this->read() ;

	if ((bit == 1) && (inv == 1))
	    throw Error(Error::Type::Vanished,__LINE__) ;
	if (bit != inv) // (0,1) and (1,0)
	{
	    if (address[i] != bit)
		throw Error(Error::Type::Vanished,__LINE__) ;
	}
	this->write(address[i]) ;
    }
}

Device::Ds18x20::Bang::Address Device::Ds18x20::Bang::
branch(Address const &address,size_t offset)
{
    auto bit = this->read() ;
    auto inv = this->read() ;

    if ((bit == 1) && (inv == 1))
	throw Error(Error::Type::Vanished,__LINE__) ;
    if (bit != inv) // (0,1) and (1,0)
	throw Error(Error::Type::Vanished,__LINE__) ;
    else // (0,0)
    {
	assert(address[offset] == 0) ;
	// otherwise we got a bug in scan()
    }
    this->write(true) ;

    auto next = address ;
    next[offset] = 1 ;
    return next ;
}


boost::optional<Device::Ds18x20::Bang::Address> Device::Ds18x20::Bang::
first()
{
    auto feedback = this->init() ;
    if (!feedback)
	return boost::none ;
    // ROM-command: Search Read-ROM-Code
    this->write(static_cast<uint8_t>(0xf0)) ;
    // [todo] support Search Alarm ROM code too
    Address address ;
    this->complete(&address,0) ;
    return address ;
} 

boost::optional<Device::Ds18x20::Bang::Address> Device::Ds18x20::Bang::
next(Address const &prev)
{
    auto offset = this->scan(prev) ;
    if (offset == 64)
	return boost::none ;
    
    auto feedback = this->init() ;
    if (!feedback)
	throw Error(Error::Type::NotPresent,__LINE__) ;
    // ROM-command: Search Read-ROM-Code
    this->write(static_cast<uint8_t>(0xf0)) ;
    // [todo] support Search Alarm ROM code too

    this->track(prev,offset) ;
    auto next = this->branch(prev,offset) ;
    this->complete(&next,offset+1) ;
    
    return next ;
}

Device::Ds18x20::Bang::Timing<uint32_t>
Device::Ds18x20::Bang::ticks(Timing<double> const &seconds,double tps)
{
    Device::Ds18x20::Bang::Timing<uint32_t> ticks ;
    auto f = [tps](double s) { return static_cast<uint32_t>(s * tps + 0.5) ; } ;
    
    ticks.slot.min = f(seconds.slot.min) ;
    ticks.slot.max = f(seconds.slot.max) ;

    ticks.rec = f(seconds.rec) ;
    
    ticks.low0.min = f(seconds.low0.min) ;
    ticks.low0.max = f(seconds.low0.max) ;

    ticks.low1.min = f(seconds.low1.min) ;
    ticks.low1.max = f(seconds.low1.max) ;

    ticks.rinit = f(seconds.rinit) ;
    ticks.rrc   = f(seconds.  rrc) ;
    ticks.rdv   = f(seconds.  rdv) ;
    ticks.rsth  = f(seconds. rsth) ;
    ticks.rstl  = f(seconds. rstl) ;
    
    ticks.pdhigh.min = f(seconds.pdhigh.min) ;
    ticks.pdhigh.max = f(seconds.pdhigh.max) ;

    ticks.pdlow.min = f(seconds.pdlow.min) ;
    ticks.pdlow.max = f(seconds.pdlow.max) ;

    return ticks ;
}
