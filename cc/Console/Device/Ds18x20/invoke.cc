// BSD 2-Clause License, see github.com/ma16/rpio

#include "../invoke.h"
#include <RpiExt/Bang.h>
#include <Ui/strto.h>

struct Bang
{
    Bang(
	Rpi::Peripheral *rpi,
	Rpi::Pin      busPin)
	
	: rpi(rpi),busPin(busPin) {}

    using Script = std::vector<RpiExt::Bang::Command> ;

    struct Record
    {
	uint32_t t0 ;
	uint32_t t1 ;
	uint32_t t2 ;
	uint32_t t3 ;

	uint32_t low ;
	uint32_t high ;

	uint32_t buffer[0x1000] ;
    } ;
    
    Script makeScript(Record *record) const ;
    Script makeScript2(Record *record) const ;
    
private:
  
    void read(RpiExt::Bang::Enqueue *q,uint32_t *levels) const ;
    void read(RpiExt::Bang::Enqueue *q,size_t nwords,uint32_t *levels) const ;
    
    void write(RpiExt::Bang::Enqueue *q,bool bit) const ;
    void write(RpiExt::Bang::Enqueue *q,uint8_t byte) const ;

    Rpi::Peripheral *rpi ;

    Rpi::Pin busPin ;
} ;

static uint32_t ticks(double seconds)
{
    return static_cast<uint32_t>(seconds * 250e+6 + .5) ;
}

void Bang::write(RpiExt::Bang::Enqueue *q,bool bit) const
{
    // 60us + 1us per bit
    q->mode(this->busPin,Rpi::Gpio::Mode::Out) ;
    auto t = bit ? 15e-6 : 60e-6 ;
    q->sleep(ticks(t)) ;
    q->mode(this->busPin,Rpi::Gpio::Mode::In) ;
    q->sleep(ticks(61e-6 - t)) ;
}

void Bang::write(RpiExt::Bang::Enqueue *q,uint8_t byte) const
{
    for (auto i=0u ; i<8u ; ++i)
    {
	auto bit = 0 != (byte & (1u<<i)) ;
	write(q,bit) ;
    }
}

void Bang::read(RpiExt::Bang::Enqueue *q,uint32_t *levels) const
{
    // 60us + 1us per bit
    q->mode(this->busPin,Rpi::Gpio::Mode::Out) ;
    q->sleep(ticks(1e-6)) ;
    q->mode(this->busPin,Rpi::Gpio::Mode::In) ;
    q->sleep(ticks(5e-6)) ;
    q->levels(levels) ;
    q->sleep(ticks(61e-6 - 6e-6)) ;
}

void Bang::read(RpiExt::Bang::Enqueue *q,size_t nwords,uint32_t *levels) const
{
    for (auto i=0u ; i<nwords ; ++i)
    {
	read(q,levels+i) ;
    }
}

Bang::Script Bang::makeScript(Record *record) const
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

    // rusage
    // long   ru_nvcsw;         /* voluntary context switches */
    // long   ru_nivcsw;        /* involuntary context switches */
}

static uint8_t crc(uint32_t const *buffer,uint32_t mask,size_t nwords)
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

Bang::Script Bang::makeScript2(Record *record) const
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

static void display(uint32_t const *buffer,uint32_t mask,size_t nwords)
{
    for (auto i=0u ; i<nwords/8 ; ++i)
    {
	unsigned code = 0 ;
	for (auto j=0u ; j<8u ; ++j)
	{
	    code <<= 1 ;
	    if (buffer[8*i+7-j] & mask)
		code |= 1u ;
	}
	std::cout << std::hex << code << ' ' ;
    }
    std::cout << '\n' ;
    {
	for (auto i=0u ; i<nwords ; ++i)
	{
	    auto bit = 0 != (buffer[i] & mask) ;
	    std::cout << bit ;
	    if ((i % 8) == 7) std::cout << ' ' ;
	    else if ((i % 4) == 3) std::cout << ':' ;
	}
	std::cout << '\n' ;
    }
}

static void doit(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    auto pin = Ui::strto(argL->pop(),Rpi::Pin()) ;
    argL->finalize() ;
    
    ::Bang::Record record ;
    RpiExt::Bang scheduler(rpi) ;

    auto script = ::Bang(rpi,pin).makeScript(&record) ;
    scheduler.execute(script) ;

    display(record.buffer,1u<<pin.value(),64) ;
    std::cout << std::hex << (unsigned)crc(record.buffer,1u<<pin.value(),56) << '\n' ;

    auto script2 = ::Bang(rpi,pin).makeScript2(&record) ;
    scheduler.execute(script2) ;

    display(record.buffer,1u<<pin.value(),72) ;
    std::cout << std::hex << (unsigned)crc(record.buffer,1u<<pin.value(),64) << '\n' ;
}

void Console::Device::Ds18x20::invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (argL->empty() || argL->peek() == "help")
    {
	std::cout << "arguments: ... \n" ;
	return ;
    }

    std::string arg = argL->pop() ;
    if (false) ;
  
    else if (arg == "doit") doit(rpi,argL) ;
  
    else throw std::runtime_error("not supported option:<"+arg+'>') ;
}
