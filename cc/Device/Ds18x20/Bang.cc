// BSD 2-Clause License, see github.com/ma16/rpio

#include <Device/Ds18x20/Bang.h>

static uint32_t ticks(double seconds)
{
    return static_cast<uint32_t>(seconds * 250e+6 + .5) ;
}

void Device::Ds18x20::Bang::write(RpiExt::Bang::Enqueue *q,bool bit) const
{
    // 60us + 1us per bit
    q->mode(this->busPin,Rpi::Gpio::Mode::Out) ;
    auto t = bit ? 15e-6 : 60e-6 ;
    q->sleep(ticks(t)) ;
    q->mode(this->busPin,Rpi::Gpio::Mode::In) ;
    q->sleep(ticks(61e-6 - t)) ;
}

void Device::Ds18x20::Bang::write(RpiExt::Bang::Enqueue *q,uint8_t byte) const
{
    for (auto i=0u ; i<8u ; ++i)
    {
	auto bit = 0 != (byte & (1u<<i)) ;
	write(q,bit) ;
    }
}

void Device::Ds18x20::Bang::read(RpiExt::Bang::Enqueue *q,uint32_t *levels) const
{
    // 60us + 1us per bit
    q->mode(this->busPin,Rpi::Gpio::Mode::Out) ;
    q->sleep(ticks(1e-6)) ;
    q->mode(this->busPin,Rpi::Gpio::Mode::In) ;
    q->sleep(ticks(5e-6)) ;
    q->levels(levels) ;
    q->sleep(ticks(61e-6 - 6e-6)) ;
}

void Device::Ds18x20::Bang::read(RpiExt::Bang::Enqueue *q,size_t nwords,uint32_t *levels) const
{
    for (auto i=0u ; i<nwords ; ++i)
    {
	read(q,levels+i) ;
    }
}

Device::Ds18x20::Bang::Script Device::Ds18x20::Bang::makeScript(Record *record) const
{
    RpiExt::Bang::Enqueue q ;
    
    // assumes busPin.mode=Input

    q.time(&record->t0) ;

    // Reset-Pulse
    
    q.mode(this->busPin,Rpi::Gpio::Mode::Out) ; 
    
    q.sleep(ticks(480e-6)) ;
    
    q.mode(this->busPin,Rpi::Gpio::Mode::In) ;

    // Present-Pulse

    q.time(&record->t1) ;

    q.waitLevel(
	&record->t1,
	ticks(60e-6), // 15..60
	&record->t2,
	1u << this->busPin.value(),
	0,
	&record->low) ;

    q.waitLevel(
	&record->t2,
	ticks(240e-6), // 60..240
	&record->t3,
	1u << this->busPin.value(),
	1u << this->busPin.value(),
	&record->high) ;

    // make sure 240 + 60 us have passed
    q.wait(&record->t1,ticks(301e-6)) ;
    
    // Read-ROM-Code command
    Bang::write(&q,static_cast<uint8_t>(0x33)) ;
    
    Bang::read(&q,64,record->buffer) ;
    
    return q.vector() ;
}

Device::Ds18x20::Bang::Script Device::Ds18x20::Bang::makeScript2(Record *record) const
{
    RpiExt::Bang::Enqueue q ;
    
    // assumes busPin.mode=Input

    q.time(&record->t0) ;

    // Reset-Pulse
    
    q.mode(this->busPin,Rpi::Gpio::Mode::Out) ; 
    
    q.sleep(ticks(480e-6)) ;
    
    q.mode(this->busPin,Rpi::Gpio::Mode::In) ;

    // Present-Pulse

    q.time(&record->t1) ;

    q.waitLevel(
	&record->t1,
	ticks(60e-6), // 15..60
	&record->t2,
	1u << this->busPin.value(),
	0,
	&record->low) ;

    q.waitLevel(
	&record->t2,
	ticks(240e-6), // 60..240
	&record->t3,
	1u << this->busPin.value(),
	1u << this->busPin.value(),
	&record->high) ;

    // make sure 240 + 60 us have passed
    q.wait(&record->t1,ticks(301e-6)) ;
    
    // Skip-ROM-Code command
    Bang::write(&q,static_cast<uint8_t>(0xcc)) ;

    // Read Sratch-Pad
    Bang::write(&q,static_cast<uint8_t>(0xbe)) ;
    
    Bang::read(&q,128,record->buffer) ;
    
    return q.vector() ;

    // rusage
    // long   ru_nvcsw;         /* voluntary context switches */
    // long   ru_nivcsw;        /* involuntary context switches */
}

uint8_t Device::Ds18x20::Bang::crc(uint32_t const *buffer,uint32_t mask,size_t nwords)
{
    uint8_t reg = 0 ;
    for (auto i=0u ; i<nwords ; ++i)
    {
	auto lsb = 0 != (reg & 0x01) ;
	reg = static_cast<uint8_t>(reg >> 1) ;
	auto nxt = 0 != (buffer[i] & mask) ;
	if (nxt ^ lsb)
	    reg ^= 0x8c ;
    }
    return reg ;
}
