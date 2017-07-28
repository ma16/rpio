// BSD 2-Clause License, see github.com/ma16/rpio

#include "Lib.h"
#include "../rpio.h"
#include <Neat/stream.h>
#include <Posix/base.h>
#include <Rpi/GpuMem.h>
#include <Rpi/Timer.h>
#include <RpiExt/VcMem.h>
#include <RpiExt/Pwm.h>
#include <Ui/ostream.h>
#include <Ui/strto.h>
#include <iomanip>
#include <iostream>

// --------------------------------------------------------------------

enum class Type { Mode,Msen,Pola,Pwen,Rptl,Sbit,Usef } ;

static void set(Rpi::Pwm *pwm,Rpi::Pwm::Index i,Type type,Ui::ArgL *argL)
{
    auto control = pwm->getControl() ;
    auto channel = control.get(i) ;
    auto flag = Ui::strto<bool>(argL->pop()) ;
    switch (type)
    {
    case Type::Mode: channel.mode = flag ; break ;
    case Type::Msen: channel.msen = flag ; break ;
    case Type::Pola: channel.pola = flag ; break ;
    case Type::Pwen: channel.pwen = flag ; break ;
    case Type::Rptl: channel.rptl = flag ; break ;
    case Type::Sbit: channel.sbit = flag ; break ;
    case Type::Usef: channel.usef = flag ; break ;
    } 	
    control.set(i,channel) ;
    pwm->setControl(control) ;
}

static void control(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (argL->empty() || argL->peek() == "help")
    {
	std::cout
	    << "arguments: COMMAND+\n"
	    << '\n'
	    << "General commands:\n"
	    << "clear         # clear FIFO\n"
	    << "dma   BOOL    # enable/disable DMA signals\n"
	    << "dreq    U8    # threshold for DMA DREQ signal\n"
	    << "panic   U8    # threshold for DMA PANIC signal\n"
	    << "pwen  BOOL    # start (1) or stop (0) both channels\n"
	    << "send   U32    # enqueue word in FIFO\n"
	    << "reset         # reset status flags\n"
	    << '\n'
	    << "Channel-specific commands:\n"
	    << "data.CH  U32  # set data register\n"
	    << "mode.CH BOOL  # serialize (1) or PWM (0)\n"
	    << "msen.CH BOOL  # enable M/S (only for PWM-mode)\n"
	    << "pwen.CH BOOL  # start (1) or stop (0)\n"
	    << "pola.CH BOOL  # inverse output polarity\n"
	    << "range.CH U32  # set range register\n"
	    << "rptl.CH BOOL  # repeat last data when FIFO is empty\n"
	    << "sbit.CH BOOL  # silence-bit for gaps\n"
	    << "usef.CH BOOL  # use FIFO\n"
	    << "CH must be either 0 or 1, e.g. pola.1 for channel 1\n"
	    << '\n'
	    ;
	return ;
    }
  
    Rpi::Pwm pwm(rpi) ;
    while (!argL->empty())
    {
	constexpr auto Channel0 = Rpi::Pwm::Index::make<0>() ;
	constexpr auto Channel1 = Rpi::Pwm::Index::make<1>() ;
	
	auto arg = argL->pop() ;
	if (false) ;

	else if (arg == "clear")
	{
	    auto c = pwm.getControl() ;
	    c.clrf1() = 1 ;
	    pwm.setControl(c) ;
	}
	
    	else if (arg == "data.0") 
	{
	    pwm. setData(Channel0,Ui::strto<uint32_t>(argL->pop())) ;
	}
    	else if (arg == "data.1") 
	{
	    pwm. setData(Channel1,Ui::strto<uint32_t>(argL->pop())) ;
	}
	
	else if (arg == "dma")
	{
	    auto d = pwm.getDmac() ;
	    d.enable = Ui::strto<bool>(argL->pop()) ;
	    pwm.setDmac(d) ;
	}
	
	else if (arg == "dreq")
	{
	    auto d = pwm.getDmac() ;
	    d.dreq = Ui::strto<uint8_t>(argL->pop()) ;
	    pwm.setDmac(d) ;
	}
	
    	else if (arg == "mode.0") set(&pwm,Channel0,Type::Mode,argL) ;
    	else if (arg == "mode.1") set(&pwm,Channel1,Type::Mode,argL) ;

    	else if (arg == "msen.0") set(&pwm,Channel0,Type::Msen,argL) ;
    	else if (arg == "msen.1") set(&pwm,Channel1,Type::Msen,argL) ;

    	else if (arg == "pola.0") set(&pwm,Channel0,Type::Pola,argL) ;
    	else if (arg == "pola.1") set(&pwm,Channel1,Type::Pola,argL) ;

    	else if (arg == "pwen.0") set(&pwm,Channel0,Type::Pwen,argL) ;
    	else if (arg == "pwen.1") set(&pwm,Channel1,Type::Pwen,argL) ;

	else if (arg == "range.0")
	{
	    pwm.setRange(Channel0,Ui::strto<uint32_t>(argL->pop())) ;
	}
	else if (arg == "range.1")
	{
	    pwm.setRange(Channel1,Ui::strto<uint32_t>(argL->pop())) ;
	}

	else if (arg == "reset")
	{
	    pwm.resetStatus(pwm.getStatus()) ;
	}
	
	else if (arg == "panic")
	{
	    auto d = pwm.getDmac() ;
	    d. panic = Ui::strto<uint8_t>(argL->pop()) ;
	    pwm.setDmac(d) ;
	}
    
	else if (arg == "pwen")
	{
	    auto c = pwm.getControl() ;
	    auto flag = Ui::strto<bool>(argL->pop()) ;
	    c.pwen1() = flag ;
	    c.pwen2() = flag ;
	    pwm.setControl(c) ;
	}
	
    	else if (arg == "rptl.0") set(&pwm,Channel0,Type::Rptl,argL) ;
    	else if (arg == "rptl.1") set(&pwm,Channel1,Type::Rptl,argL) ;

    	else if (arg == "sbit.0") set(&pwm,Channel0,Type::Sbit,argL) ;
    	else if (arg == "sbit.1") set(&pwm,Channel1,Type::Sbit,argL) ;

	else if (arg == "send")
	{
	    pwm.write(Ui::strto<uint32_t>(argL->pop())) ;
	}
	
    	else if (arg == "usef.0") set(&pwm,Channel0,Type::Usef,argL) ;
    	else if (arg == "usef.1") set(&pwm,Channel1,Type::Usef,argL) ;

	else throw std::runtime_error("not supported option:<"+arg+'>') ;
    }
}

// --------------------------------------------------------------------

static void dma(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (argL->empty() || argL->peek() == "help")
    {
	std::cout << "arguments: PIX DIX [CS] [TI] [MEM] FILE\n"
		  << '\n'
		  << " PIX : 0|1    # PWM channel to use\n"
		  << " DIX : 0..15  # DMA channel to use\n"
		  << '\n'
		  << "  CS = DMA control and status\n"
		  << "  TI = DMA transfer information\n"
		  << " MEM = type of memory to be used for DMA setup\n"
		  << "FILE = name of file with data to be sent\n"
		  << std::flush ;
	return ;
    }
  
    // ---- configuration ----

    Rpi::Pwm pwm(rpi) ; auto pwm_index = Ui::strto(argL->pop(),Rpi::Pwm::Index()) ;
  
    auto channel = Rpi::Dma::Ctrl(rpi).channel(Ui::strto(argL->pop(),Rpi::Dma::Ctrl::Index())) ;
  
    auto cs = Console::Dma::Lib::optCs(argL,Rpi::Dma::Cs()) ;
  
    auto ti = Console::Dma::Lib::optTi(argL,Rpi::Dma::Ti::send(Rpi::Pwm::permap())) ;

    auto allof = RpiExt::VcMem::getFactory(rpi,argL,RpiExt::VcMem::defaultFactory()) ;
  
    auto data = RpiExt::VcMem::read(argL->pop(),allof.get()) ;

    argL->finalize() ;

    // ---- remaining setup ----

    auto ts = allof->allocate(2 * sizeof(uint32_t)) ; // 2x time stamp
  
    Console::Dma::Lib::Control ctl(allof->allocate((2+1) * 32)) ;
  
    Console::Dma::Lib::write(&ctl,Rpi::Dma::Ti(),Rpi::Timer::cLoAddr,       ts.get(),0u,sizeof(uint32_t)) ;
    Console::Dma::Lib::write(&ctl,            ti,      data.get(),0u,Rpi::Pwm::fifoAddr,  data->nbytes()) ; 
    Console::Dma::Lib::write(&ctl,Rpi::Dma::Ti(),Rpi::Timer::cLoAddr,       ts.get(),4u,sizeof(uint32_t)) ;
    // ...[todo] should be defined in Console/Dma/Lib

    // (3) ---- run ----

    Console::Pwm::Lib::setup(&pwm,pwm_index) ; pwm.setRange(pwm_index,32) ; // [todo] leave to ctrl
    // ...does not start yet since we need to...
    channel.setup(ctl.addr(),cs) ; channel.start() ;
    // ...fill up the PWM queue first
    Console::Pwm::Lib::start(&pwm,pwm_index) ;
    while (0 != (channel.getCs().active().bits()))
	Posix::nanosleep(1E+3) ;
    // ...arbitrary sleep value
    Console::Pwm::Lib::finish(&pwm,pwm_index) ;
    // ...wait til queue empty, last still in progress and will be repeated
  
    // (4) ---- log statistics ----
  
    std::cout.setf(std::ios::scientific) ;
    std::cout.precision(2) ;
    auto dt = static_cast<double>(ts->as<uint32_t*>()[1]-ts->as<uint32_t*>()[0])/1E6 ;
    std::cout << dt << "s " << static_cast<double>(data->nbytes()/4*32)/dt << "/s" << std::endl ;
}

// --------------------------------------------------------------------

static void dummy(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (argL->empty() || argL->peek() == "help") {
	std::cout << "arguments: PIX DIX [CS] [TI] [MEM] NWORDS NBITS\n"
		  << '\n'
		  << " PIX : 0|1    # PWM channel to use\n"
		  << " DIX : 0..15  # DMA channel to use\n"
		  << '\n'
		  << "    CS = DMA control and status\n"
		  << "    TI = DMA transfer information\n"
		  << "   MEM = type of memory to be used for DMA setup\n"
		  << "NWORDS = number of words to be sent\n"
		  << " NBITS = number of bits per word to be sent\n"
		  << std::flush ;
	return ;
    }
  
    // (1) ---- configuration ----

    auto pwm_index = Ui::strto(argL->pop(),Rpi::Pwm::Index()) ;
    Rpi::Pwm pwm(rpi) ;
  
    auto dma_index = Ui::strto(argL->pop(),Rpi::Dma::Ctrl::Index()) ;
    Rpi::Dma::Ctrl dma(rpi) ;
    auto channel = dma.channel(dma_index) ;
  
    auto cs = Console::Dma::Lib::optCs(argL,Rpi::Dma::Cs()) ;
  
    auto tix = Rpi::Dma::Ti::send(Rpi::Pwm::permap()) ;
    tix = Console::Dma::Lib::optTi(argL,tix) ;
    // + no_wide_bursts
    // + waits
    // + burst_length
    // + src.width
    // + wait_resp
  
    Rpi::Dma::Ti tid ; // just for debugging/monitoring
    tid.srcInc()= true ;
    tid.permap() = Rpi::Pwm::permap() ;
    tid.destDreq()= true ;
  
    auto mem = (argL->pop_if("--mem"))
	? RpiExt::VcMem::getFactory(rpi,argL)
	: RpiExt::VcMem::defaultFactory() ;
  
    auto nwords = Ui::strto<uint32_t>(argL->pop()) ;
  
    auto nbits = Ui::strto<uint32_t>(argL->pop()) ;
  
    argL->finalize() ;

    // (2) ---- prepare ----

    auto ts = mem->allocate(5 * sizeof(uint32_t)) ; // time stamp + monitoring + repeat
    Console::Dma::Lib::Control ctl(mem->allocate(5 * 32)) ;
  
    Console::Dma::Lib::write(&ctl,           tid,        ts.get(),8u, Rpi::Pwm::fifoAddr,            8u  ) ; // prefix for monitoring
    Console::Dma::Lib::write(&ctl,Rpi::Dma::Ti(),Rpi::Timer::cLoAddr,        ts.get(),0u,sizeof(uint32_t)) ;
    ctl.write            (                tix,Rpi::Bus::null_addr, Rpi::Pwm::fifoAddr,     nwords*4,0 ) ;
    Console::Dma::Lib::write(&ctl,Rpi::Dma::Ti(),Rpi::Timer::cLoAddr,        ts.get(),4u,sizeof(uint32_t)) ;
    Console::Dma::Lib::write(&ctl,           tid,        ts.get(),8u, Rpi::Pwm::fifoAddr,           12u  ) ; // postfix for monitoring
  
    ts->as<uint32_t*>()[2] = 0xffffffff ; 
    ts->as<uint32_t*>()[3] = 0x0 ;
    ts->as<uint32_t*>()[4] = 0xffffffff ; // repeat
  
    // (3) ---- run ----

    Console::Pwm::Lib::setup(&pwm,pwm_index) ; pwm.setRange(pwm_index,nbits) ; // [todo] leave to ctrl
    // ...does not start yet since we need to...
    channel.setup(ctl.addr(),cs) ; channel.start() ;
    // ...fill up the PWM queue first
    Console::Pwm::Lib::start(&pwm,pwm_index) ;
    while (0 != (channel.getCs().active().bits()))
	Posix::nanosleep(1E+3) ;
    // ...arbitrary sleep value
    Console::Pwm::Lib::finish(&pwm,pwm_index) ;
    // ...wait til queue empty, last still in progress and will be repeated

    // (4) ---- log statistics ----

    std::cout.setf(std::ios::scientific) ;
    std::cout.precision(2) ;
    auto dt = static_cast<double>(ts->as<uint32_t*>()[1]-ts->as<uint32_t*>()[0])/1E6 ;
    std::cout << dt << "s " << static_cast<double>(nwords*nbits)/dt << "/s" << std::endl ;
}

// --------------------------------------------------------------------

static void frequency(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (!argL->empty() && argL->peek() == "help")
    {
	std::cout
	    << "arguments: [DURATION]\n"
	    << '\n'
	    << "DURATION = time in seconds to sample (default 0.1)\n"
	    << '\n'
	    << "you may want to set up the range register beforehand\n"
	    ;
	return ;
    }
    auto duration = 0.1 ;
    if (!argL->empty())
	duration = Ui::strto<double>(argL->pop()) ;
    argL->finalize() ;
    // [todo] verify whether the serializer is working at all
    auto sample = RpiExt::Pwm(rpi).measureRate(duration) ;
    auto f = sample.first * Rpi::Pwm(rpi).getRange(Rpi::Pwm::Index::make<0>()) ;
    std::cout.setf(std::ios::scientific) ;
    std::cout.precision(2) ;
    std::cout << f << " (" << sample.first << ',' << sample.second << ")\n" ;
}

// --------------------------------------------------------------------

static void send(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (argL->empty() || argL->peek() == "help") {
	std::cout << "arguments: INDEX FILE\n"
		  << '\n'
		  << "INDEX = channel to use (0,1)\n"
		  << " FILE = name of file with data to transfer\n"
		  << '\n'
		  << "you may want to set up the registers beforehand\n"
		  << std::flush ;
	return ;
    }
    auto index = Ui::strto(argL->pop(),Rpi::Pwm::Index()) ;
    std::ifstream is ; Neat::open(&is,argL->pop()) ;
    auto nbytes = Neat::demote<size_t>(Neat::size(&is).as_unsigned()) ; 
    auto nwords = nbytes / 4 ;
    if (nwords * 4 != nbytes)
	throw std::runtime_error("file-size must be a multiple of 4 bytes") ;
    auto data = std::unique_ptr<uint32_t[]>(new uint32_t[nwords]) ;
    Neat::read(&is,data.get(),Neat::ustreamsize::make(nbytes)) ; 
    argL->finalize() ;
    Rpi::Pwm pwm(rpi) ;
    pwm.setRange(index,32) ;
    auto c = pwm.getControl() ;
    auto x = c.get(index) ;
    x.sbit = 0 ; 
    x.pola = 0 ; 
    x.rptl = 0 ;
    x.pwen = 1 ;
    c.set(index,x) ;
    pwm.setControl(c) ;
    auto ngaps = Console::Pwm::Lib::send(pwm,index,data.get(),nwords) ;
    std::cout << ngaps << std::endl ;
    //RpiExt::Pwm(rpi,index).send(data.get(),nwords,2.5e+6) ;
}

// --------------------------------------------------------------------

static void status(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (!argL->empty() && argL->peek() == "help")
    {
	std::cout << "no arguments supported\n" ;
	return ;
    }
    argL->finalize() ;
    Rpi::Pwm pwm(rpi) ;
    
    auto d = pwm.getDmac() ;
    std::cout << std::hex
	      << "DMA-Control: enable=" << d.enable << " "
	      << "panic=" << d.panic << " "
	      << "dreq="  << d.dreq << "\n\n" ;
    
    auto s = pwm.getStatus() ;
    std::cout << "sta2 sta1 berr gap2 gap1 rerr werr empt full\n"
	      << "--------------------------------------------\n"
	      << std::setw(4) << (0 != s. csta2())
	      << std::setw(5) << (0 != s. csta1())
	      << std::setw(5) << (0 != s. cberr())
	      << std::setw(5) << (0 != s.cgapo2())
	      << std::setw(5) << (0 != s.cgapo1())
	      << std::setw(5) << (0 != s. crerr())
	      << std::setw(5) << (0 != s. cwerr())
	      << std::setw(5) << (0 != s. cempt())
	      << std::setw(5) << (0 != s. cfull())
	      << " (0x" << s.value() << ")\n\n" ;
    
    std::cout << "# msen usef pola sbit rptl mode pwen     data    range\n"
	      << "------------------------------------------------------\n" ;
    auto c = pwm.getControl() ;
    Rpi::Pwm::Index i ;
    do
    {
	auto x = c.get(i) ;
	auto d = pwm.getData(i) ;
	auto r = pwm.getRange(i) ;
	std::cout << std::setw(1) << i.value()
		  << std::setw(5) << x.msen
		  << std::setw(5) << x.usef
		  << std::setw(5) << x.pola
		  << std::setw(5) << x.sbit
		  << std::setw(5) << x.rptl
		  << std::setw(5) << x.mode
		  << std::setw(5) << x.pwen
		  << std::setw(9) << d
		  << std::setw(9) << r
		  << '\n' ;
    }
    while (i.next()) ;
}

// --------------------------------------------------------------------

void Console::Pwm::invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (argL->empty() || argL->peek() == "help") { 
	std::cout << "arguments: MODE [help]\n"
		  << '\n'
		  << "MODE : control    # write peripheral registers\n"
		  << "     | dma        # send data in DMA/FIFO mode\n"
		  << "     | dummy      # send dummy data in DMA/FIFO mode\n"
		  << "     | frequency  # estimate current frequency\n"
		  << "     | send       # send data in CPU/FIFO mode\n"
		  << "     | status     # display status\n"
		  << std::flush ;
	return ;
    }
    std::string arg = argL->pop() ;
    if (false) ;
    
    else if (arg ==   "control")     control(rpi,argL) ; 
    else if (arg ==       "dma")         dma(rpi,argL) ; 
    else if (arg ==     "dummy")       dummy(rpi,argL) ; 
    else if (arg == "frequency")   frequency(rpi,argL) ; 
    else if (arg ==      "send")        send(rpi,argL) ; 
    else if (arg ==    "status")      status(rpi,argL) ; 
    else throw std::runtime_error("not supported option:<"+arg+'>') ;
}

// serializer of both channels does not always work in-sync
// $ rpio cp set 3 -f 0 -i 200 -m 0 -s 6
// $ rpio cp switch 3 on
// $ rpio gpio mode -l 12,13 0
// $ rpio pwm ctrl
//   index 0 pwen 0 msen 0 usef 1 pola 0 sbit 1	rptl 0 mode 1 range 40
//   index 1 pwen 0 msen 0 usef 1 pola 0 sbit 1 rptl 0 mode 1 range 40
//   reset clear 
//   queue 0x55555554 queue 0x0000fffe queue 0x33333332 
//   queue 0x00ff00fe queue 0x55555554 queue 0x0000fffe	
//   sync
// $ rpio pwm ctrl index 0 pwen 0 index 1 pwen 0
