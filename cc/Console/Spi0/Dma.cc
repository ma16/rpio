// BSD 2-Clause License, see github.com/ma16/rpio

#include "Dma.h"

#include "Alarm.h"
#include "Dump.h"
#include "Layout.h"
#include "Options.h"
#include "Watch.h"

#include <Posix/base.h> // nanosleep()
#include <Posix/Signal.h>
#include <Rpi/Timer.h>

#include <signal.h> // SIGINT

struct Control
{
    static Control setup(Rpi::Peripheral *rpi,Console::Spi0::Options const &options)
    {
	auto channel = Rpi::Dma::Ctrl(rpi).channel(options.cno) ;

	channel.stop() ;
    
	auto layout = Console::Spi0::Layout::setup(options.mosiV,options.nrecords,options.flags,options.adcs,options.loop) ;
	
	Rpi::Dma::Cs cs ; // make configurable
	
	layout.enter(&channel,cs) ;

	auto dentries = options.qbuffer / (options.brecords * layout.record_size()) ;
	if (dentries == 0) dentries = 1 ;
	
	auto dump = Console::Spi0::Dump::make(options.os.get(),dentries) ;

	auto alarm = Console::Spi0::Alarm::start(std::chrono::duration<float>(options.progress)) ;
	
	Console::Spi0::Watch watch(layout.front(),options.nrecords,layout.record_size(),/*index:*/0u) ;

	return Control(&options,channel,layout,std::move(dump),alarm,watch) ;
    }

    size_t establish_index()
    {
	// record index thru array of DMA control blocks
	auto index = this->layout.index(&this->channel) ;
	
	assert(this->options->nrecords > 1) ;
	// ...otherwise next line would cause a dead-lock

	while (index == this->layout.index(&this->channel))
	    ;
	// ...we need a valid time-stamp to watch for overruns [flawed]
	this->watch.reset(index) ;
	
	return index ;

	// [todo] this is actually only required when loop = true ;
	// so split the code in loop / none-loop
    }

    bool done() const
    {
	if (0 != this->channel.getCs().active().bits())
	    return false ;
	if (0 != this->channel.getCb().value())
	    return false ;
	return true ;
    }

    bool save() 
    {
	std::vector<uint8_t> buffer(this->options->brecords * this->layout.record_size()) ;

	auto success = this->watch.fetch(this->options->brecords,&buffer[0]) ;
	
	if (!success)
	{
	    this->establish_index() ;
	    return false ;
	}

	if (options->os)
	    this->dump->schedule(std::move(buffer)) ;
	return true ;
    }

    size_t flush(size_t i)
    {
	auto n = this->watch.avail(i) ;

	std::vector<uint8_t> buffer(n * this->layout.record_size()) ; 

	auto success = this->watch.fetch(n,&buffer[0]) ;
	assert(success) ; (void)success ;
	
	if (this->options->os)
	    this->dump->schedule(std::move(buffer)) ;

	return n ;
    }
    
    Console::Spi0::Options     const *options ;
    Rpi::Dma::Channel              channel ;
    Console::Spi0::Layout              layout ;
    std::unique_ptr<Console::Spi0::Dump> dump ;
    Console::Spi0::Alarm                alarm ;
    Console::Spi0::Watch                watch ;
    
    Control(
	Console::Spi0::Options     const *options,
	Rpi::Dma::Channel              channel,
	Console::Spi0::Layout              layout,
	std::unique_ptr<Console::Spi0::Dump> dump,
	Console::Spi0::Alarm                alarm,
	Console::Spi0::Watch                watch
	) :
	options     (options),
	channel     (channel),
	layout       (layout),
	dump(std::move(dump)),
	alarm         (alarm),
	watch         (watch)
	{}
} ;

static void flat(Rpi::Peripheral *rpi,Console::Spi0::Options const &options)
{
    std::cout << "setting up DMA memory... " << std::endl ;

    auto control = Control::setup(rpi,options) ;

    std::cout << "start DMA transfer... " << std::endl ;
   
    control.channel.start() ;
    
    while (!control.done())
    {
	if (control.alarm.expired())
	{
	    auto index = control.layout.index(&control.channel) ;
	    auto dt = std::chrono::duration<float>(control.alarm.passed()).count() ;
	    std::cout << dt << ' ' << index << std::endl ;
	}

	if (options.sleep > 0.0)
	    Posix::nanosleep(options.sleep * 1e+9) ;
    }

    if (options.os)
    {
	options.os->write((char*)control.layout.front(),(std::streamsize)(options.nrecords * control.layout.record_size())) ;
	if (!options.os->good())
	    throw std::runtime_error("cannot write file") ;
    }

    control.dump->stop() ; // [todo] actually not used here
    
    std::cout << "terminated normally" << std::endl ;
}

static void loop(Rpi::Peripheral *rpi,Console::Spi0::Options const &options)
{
    Posix::Signal::block(SIGINT) ;
    
    std::cout << "setting up DMA memory... " << std::endl ;

    auto control = Control::setup(rpi,options) ;

    std::cout << "start DMA transfer (press interrupt to stop)... " << std::endl ;
   
    control.channel.start() ;
    
    auto index = control.establish_index() ; // [todo] probably won't catch first block
    
    auto nsaved = 0u ;

    while (!Posix::Signal::pending(SIGINT))
    {
	if (control.alarm.expired())
	{
	    auto dt = std::chrono::duration<float>(control.alarm.passed()).count() ;
	    std::cout << dt << ' ' << nsaved << ' ' << control.dump->pending() << std::endl ;
	    nsaved = 0 ;
	}
	
	index = control.layout.index(&control.channel) ;

	if (control.watch.overrun())
	{
	    std::cout << "overrun (before fetching)" << std::endl ;
	    control.establish_index() ;
	    continue ;
	}

	auto n = control.watch.avail(index) ;

	if (n < options.brecords)
	{
	    if (options.sleep > 0.0)
		Posix::nanosleep(options.sleep * 1e+9) ;
	    continue ;
	}

	if (!control.save())
	{
	    std::cout << "overrun (after fetching)" << std::endl ;
	    continue ;
	}
	
	nsaved += options.brecords ;
    }

    std::cout << "Signal caught" << std::endl ;
    
    index = control.layout.index(&control.channel) ;
    
    control.channel.stop() ;
    
    if (control.watch.overrun())
    {
	std::cout << "overrun (before fetching)" << std::endl ;
    }
    else
    {
	nsaved += control.flush(index) ;
    }

    control.dump->stop() ;

    std::cout << nsaved << std::endl ;
    
    std::cout << "terminated normally" << std::endl ;
}

void Console::Spi0::Dma::invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (true == Options::diagnostics(argL))
	return ;

    auto options = Options::parse(argL) ;
    if (options.loop) loop(rpi,options) ;
    else              flat(rpi,options) ;
}
