// BSD 2-Clause License, see github.com/ma16/rpio

#include "../invoke.h"
#include <RpiExt/Bang.h>
#include <Ui/strto.h>

// Read ROM
// This command can be used when there is only one Slave on the bus. It allows the Master to read the Slave’s 64-bit ROM code without using the Search ROM procedure. If this command is used when there is more than one Slave, a data collision will occur since all Slaves attempt to respond at the same time.
// ->Reset-Pulse <-Present-Pulse ->Read-ROM:33 <-Bit:0..63

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

	std::array<uint32_t,64> rx ;
    } ;
    
    Script makeScript(Record *record) const ;
    
private:
  
    Rpi::Peripheral *rpi ;

    Rpi::Pin busPin ;
} ;

static uint32_t ticks(double seconds)
{
    return static_cast<uint32_t>(seconds * 250e+6 + .5) ;
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

#if 0
    // Read-ROM-Code

    write(q,0x33) ;

    read(q,&record.rx[0],record.rx.size) ;
#endif
    
    return q.vector() ;

    // rusage
    // long   ru_nvcsw;         /* voluntary context switches */
    // long   ru_nivcsw;        /* involuntary context switches */
}


static void doit(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    auto pin = Ui::strto(argL->pop(),Rpi::Pin()) ;
    argL->finalize() ;
    ::Bang::Record record ;
    auto script = ::Bang(rpi,pin).makeScript(&record) ;
    RpiExt::Bang scheduler(rpi) ;
    scheduler.execute(script) ;
    std::cout << "back again\n" ;
}

void Console::Device::Ds18s20::invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
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
