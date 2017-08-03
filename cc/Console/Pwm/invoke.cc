// BSD 2-Clause License, see github.com/ma16/rpio

#include "Lib.h"
#include "../rpio.h"
#include <deque>
#include <iomanip>
#include <iostream>
#include <Neat/stream.h>
#include <Posix/base.h>
#include <Rpi/GpuMem.h>
#include <Rpi/Timer.h>
#include <RpiExt/VcMem.h>
#include <RpiExt/Pwm.h>
#include <Ui/ostream.h>
#include <Ui/strto.h>

// --------------------------------------------------------------------

static void clear(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (argL->empty() || argL->peek() == "help")
    {
	std::cout << "arguments: OPTION+\n"
		  << "OPTION: all,berr,gap1,gap2,rerr,werr\n" ;
	return ;
    }
    Rpi::Pwm::Status::Word w ;
    while (!argL->empty())
    {
	using Status = Rpi::Pwm::Status ;
	auto arg = argL->pop() ;
	if (arg == "all" || arg == "berr") w.at(Status::Berr) = 1 ;
	if (arg == "all" || arg == "gap1") w.at(Status::Gap1) = 1 ;
	if (arg == "all" || arg == "gap2") w.at(Status::Gap2) = 1 ;
	if (arg == "all" || arg == "rerr") w.at(Status::Rerr) = 1 ;
	if (arg == "all" || arg == "werr") w.at(Status::Werr) = 1 ;
	else throw std::runtime_error("not supported option:<"+arg+'>') ;
    }
    Rpi::Pwm(rpi).status().clear(w) ;
}

static void control(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (argL->empty() || argL->peek() == "help")
    {
	std::cout
	    << "arguments: OPTION+\n"
	    << '\n'
	    << "clear              # clear FIFO (single shot)\n"
	    << "mode.CHANNEL=BOOL  # 1=Serial 0=PWM\n"
	    << "msen.CHANNEL=BOOL  # 1=Mark-Space 0=coherent (if mode=0)\n"
	    << "pwen.CHANNEL=BOOL  # 1=start transmission 0=stop\n"
	    << "pola.CHANNEL=BOOL  # 1=inverse output polarity\n"
	    << "rptl.CHANNEL=BOOL  # 1=repeat word when idle (if usef=1)\n"
	    << "sbit.CHANNEL=BOOL  # 1=High 0=Low output when idle\n"
	    << "usef.CHANNEL=BOOL  # 1=read FIFO 0=read Data register\n"
	    << '\n'
	    << "CHANNEL must be either 1 or 2; BOOL must be either 0 or 1\n" ;
	return ;
    }
  
    Rpi::Pwm pwm(rpi) ;
    auto w = pwm.control().read() ;
    while (!argL->empty())
    {
	using Control = Rpi::Pwm::Control ;
    
	auto arg = argL->pop() ;
	if (false) ;

	else if (arg == "clear")  w.at(Control::Clrf) = 1 ;
	
    	else if (arg == "mode.1=0") w.at(Control::Mode1) = 0 ;
    	else if (arg == "mode.2=0") w.at(Control::Mode2) = 0 ;
    	else if (arg == "msen.1=0") w.at(Control::Msen1) = 0 ;
    	else if (arg == "msen.2=0") w.at(Control::Msen2) = 0 ;
    	else if (arg == "pola.1=0") w.at(Control::Pola1) = 0 ;
    	else if (arg == "pola.2=0") w.at(Control::Pola2) = 0 ;
    	else if (arg == "pwen.1=0") w.at(Control::Pwen1) = 0 ;
    	else if (arg == "pwen.2=0") w.at(Control::Pwen2) = 0 ;
    	else if (arg == "rptl.1=0") w.at(Control::Rptl1) = 0 ;
    	else if (arg == "rptl.2=0") w.at(Control::Rptl2) = 0 ;
    	else if (arg == "sbit.1=0") w.at(Control::Sbit1) = 0 ;
    	else if (arg == "sbit.2=0") w.at(Control::Sbit2) = 0 ;
    	else if (arg == "usef.1=0") w.at(Control::Usef1) = 0 ;
    	else if (arg == "usef.2=0") w.at(Control::Usef2) = 0 ;

    	else if (arg == "mode.1=1") w.at(Control::Mode1) = 1 ;
    	else if (arg == "mode.2=1") w.at(Control::Mode2) = 1 ;
    	else if (arg == "msen.1=1") w.at(Control::Msen1) = 1 ;
    	else if (arg == "msen.2=1") w.at(Control::Msen2) = 1 ;
    	else if (arg == "pola.1=1") w.at(Control::Pola1) = 1 ;
    	else if (arg == "pola.2=1") w.at(Control::Pola2) = 1 ;
    	else if (arg == "pwen.1=1") w.at(Control::Pwen1) = 1 ;
    	else if (arg == "pwen.2=1") w.at(Control::Pwen2) = 1 ;
    	else if (arg == "rptl.1=1") w.at(Control::Rptl1) = 1 ;
    	else if (arg == "rptl.2=1") w.at(Control::Rptl2) = 1 ;
    	else if (arg == "sbit.1=1") w.at(Control::Sbit1) = 1 ;
    	else if (arg == "sbit.2=1") w.at(Control::Sbit2) = 1 ;
    	else if (arg == "usef.1=1") w.at(Control::Usef1) = 1 ;
    	else if (arg == "usef.2=1") w.at(Control::Usef2) = 1 ;

	else throw std::runtime_error("not supported option:<"+arg+'>') ;
    }
    pwm.control().write(w) ;
}

static void data(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (argL->empty() || argL->peek() == "help")
    {
	std::cout << "arguments: CHANNEL U32\n" ;
	return ;
    }
    auto index = Rpi::Pwm::Index::make(argL->pop({"1","2"})) ;
    auto word = Ui::strto<uint32_t>(argL->pop()) ;
    argL->finalize() ;
    Rpi::Pwm(rpi).data(index).write(word) ;
}

static void dmaC(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (argL->empty() || argL->peek() == "help")
    {
	std::cout << "arguments: OPTION+\n"
		  << '\n'
		  << "OPTION : enable BOOL\n"
		  << "       | dreq     U8\n"
		  << "       | panic    U8\n" ;
	return ;
    }
    Rpi::Pwm pwm(rpi) ;
    auto w = pwm.dmaC().read() ;
    while (!argL->empty())
    {
	auto arg = argL->pop() ;
	if (false) ;
	else if (arg == "enable") w.enable = Ui::strto   <bool>(argL->pop()) ;
	else if (arg ==   "dreq") w.  dreq = Ui::strto<uint8_t>(argL->pop()) ;
	else if (arg ==  "panic") w. panic = Ui::strto<uint8_t>(argL->pop()) ;
	else throw std::runtime_error("not supported option:<"+arg+'>') ;
    }
    argL->finalize() ;
    pwm.dmaC().write(w) ;
}
	       
static void enqueue(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (argL->empty() || argL->peek() == "help")
    {
	std::cout << "arguments: [ -c U32 | -u ] DATA\n"
		  << '\n'
		  << "-c  # detect underruns, this requires padding\n"
		  << "-u  # unpaced (ignore FIFO status)\n"
		  << "default: fill FIFO whenver there is space\n"
		  << '\n'
		  << "DATA : file FILE  # file with binary data to transfer\n" 
		  << "     | U32+       # list of words\n" ;
	return ;
    }
    enum Mode { Checked,Topup,Unpaced } mode = Topup ;
    uint32_t padding = 0x0 ;
    if (argL->pop_if("-c"))
    {
	mode = Checked ;
	padding = Ui::strto<uint32_t>(argL->pop()) ;
    }
    else if (argL->pop_if("-u"))
	mode = Unpaced ;
    std::vector<uint32_t> data ;
    if (argL->pop_if("file"))
    {
	std::ifstream is ; Neat::open(&is,argL->pop()) ;
	auto nbytes = Neat::demote<size_t>(Neat::size(&is).as_unsigned()) ; 
	auto nwords = nbytes / 4 ;
	if (nwords * 4 != nbytes)
	    throw std::runtime_error("file-size must be a multiple of 4 bytes") ;
	data.resize(nwords) ;
	Neat::read(&is,&data[0],Neat::ustreamsize::make(nbytes)) ; 
    }
    else
    {
	std::deque<uint32_t> q ;
	while (!argL->empty())
	    q.push_back(Ui::strto<uint32_t>(argL->pop())) ;
	data = std::vector<uint32_t>(q.begin(),q.end()) ;
    }
    argL->finalize() ;
    if (mode == Checked)
    {
	auto nwords = RpiExt::Pwm(rpi).convey(&data[0],data.size(),padding) ;
	if (nwords < data.size())
	    std::cout << nwords << " / " << data.size() << " words written\n" ;
    }
    else if (mode == Unpaced)
    {
	Rpi::Pwm pwm(rpi) ;
	for (auto w: data)
	    pwm.fifo().write(w) ;
    }
    else
    {
	assert(mode == Topup) ;
	RpiExt::Pwm(rpi).write(&data[0],data.size()) ;
    }
}

static void frequency(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (!argL->empty() && argL->peek() == "help")
    {
	std::cout
	    << "arguments: [-d DURATION]\n"
	    << '\n'
	    << "DURATION: time in seconds to fill-up FIFO (default: 0.1)\n" ;
	return ;
    }
    auto duration = Ui::strto<double>(argL->option("-d","0.1")) ;
    argL->finalize() ;
    Rpi::Pwm pwm(rpi) ;
    auto control = pwm.control().read() ;
    double width ;
    unsigned nchannels = 0 ;
    if (control.test(Rpi::Pwm::Control::Pwen1))
    {
	if (!control.test(Rpi::Pwm::Control::Usef1))
	    throw std::runtime_error("channel #1 enabled w/o USEF=1") ;
	width = pwm.range(Rpi::Pwm::Channel1).read() ;
	++nchannels ;
    }
    if (control.test(Rpi::Pwm::Control::Pwen2))
    {
	if (!control.test(Rpi::Pwm::Control::Usef2))
	    throw std::runtime_error("channel #2 enabled w/o USEF=1") ;
	width = pwm.range(Rpi::Pwm::Channel2).read() ;
	++nchannels ;
    }
    if (nchannels == 0)
	throw std::runtime_error("no channel enabled") ;
    if (nchannels > 1)
	throw std::runtime_error("both channels enabled") ;
    auto rate = RpiExt::Pwm(rpi).measureRate(duration) ;
    std::cout.setf(std::ios::scientific) ;
    std::cout.precision(2) ;
    std::cout << rate.first * width << ' '
	      << '(' << rate.first << ',' << rate.second << ")\n" ;
}

static void range(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (argL->empty() || argL->peek() == "help")
    {
	std::cout << "arguments: CHANNEL U32\n" ;
	return ;
    }
    auto index = Rpi::Pwm::Index::make(argL->pop({"1","2"})) ;
    auto word = Ui::strto<uint32_t>(argL->pop()) ;
    argL->finalize() ;
    Rpi::Pwm(rpi).range(index).write(word) ;
}

static void status(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (!argL->empty() && argL->peek() == "help")
    {
	std::cout << "no arguments supported\n" ;
	return ;
    }
    argL->finalize() ;
    Rpi::Pwm pwm(rpi) ;
    
    auto d = pwm.dmaC().read() ;
    std::cout << std::hex
	      << "DMA-Control: enable=" << d.enable << " "
	      << "panic=" << d.panic << " "
	      << "dreq="  << d.dreq << "\n\n" ;
    
    using Status = Rpi::Pwm::Status ;
    auto s = pwm.status().read() ;
    std::cout << "berr rerr werr empt full\n"
	      << "------------------------\n"
	      << std::setw(4) << s.test(Status::Berr)
	      << std::setw(5) << s.test(Status::Rerr)
	      << std::setw(5) << s.test(Status::Werr)
	      << std::setw(5) << s.test(Status::Empt)
	      << std::setw(5) << s.test(Status::Full)
	      << " (0x" << s.value() << ")\n\n" ;

    std::cout
	<< "# sta gap msen usef pola sbit rptl mode pwen     data    range\n"
	<< "--------------------------------------------------------------\n" ;
    auto c = pwm.control().read() ;
    auto i = Rpi::Pwm::Channel1 ;
    do
    {
	auto cbank = Rpi::Pwm::Control::Bank::select(i) ;
	auto sbank = Rpi::Pwm:: Status::Bank::select(i) ;
	
	std::cout << std::setw(1) << i.value() + 1
		  << std::setw(4) << s.test(sbank.sta) 
		  << std::setw(4) << s.test(sbank.gap)
	
		  << std::setw(5) << c.test(cbank.msen)
		  << std::setw(5) << c.test(cbank.usef)
		  << std::setw(5) << c.test(cbank.pola)
		  << std::setw(5) << c.test(cbank.sbit)
		  << std::setw(5) << c.test(cbank.rptl)
		  << std::setw(5) << c.test(cbank.mode)
		  << std::setw(5) << c.test(cbank.pwen)
	
		  << std::setw(9) << pwm. data(i).read() 
		  << std::setw(9) << pwm.range(i).read()
		  << '\n' ;
    }
    while (i.next()) ;
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
  
    auto ti = Console::Dma::Lib::optTi(argL,Rpi::Dma::Ti::send(Rpi::Pwm::DmaC::Permap)) ;

    auto allof = RpiExt::VcMem::getFactory(rpi,argL,RpiExt::VcMem::defaultFactory()) ;
  
    auto data = RpiExt::VcMem::read(argL->pop(),allof.get()) ;

    argL->finalize() ;

    // ---- remaining setup ----

    auto ts = allof->allocate(2 * sizeof(uint32_t)) ; // 2x time stamp
  
    Console::Dma::Lib::Control ctl(allof->allocate((2+1) * 32)) ;
  
    Console::Dma::Lib::write(&ctl,Rpi::Dma::Ti(),Rpi::Timer::cLoAddr,       ts.get(),0u,sizeof(uint32_t)) ;
    Console::Dma::Lib::write(&ctl,            ti,      data.get(),0u,Rpi::Pwm::Fifo::Address,  data->nbytes()) ; 
    Console::Dma::Lib::write(&ctl,Rpi::Dma::Ti(),Rpi::Timer::cLoAddr,       ts.get(),4u,sizeof(uint32_t)) ;
    // ...[todo] should be defined in Console/Dma/Lib

    // (3) ---- run ----

    Console::Pwm::Lib::setup(&pwm,pwm_index) ; pwm.range(pwm_index).write(32) ; // [todo] leave to ctrl
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
  
    auto tix = Rpi::Dma::Ti::send(Rpi::Pwm::DmaC::Permap) ;
    tix = Console::Dma::Lib::optTi(argL,tix) ;
    // + no_wide_bursts
    // + waits
    // + burst_length
    // + src.width
    // + wait_resp
  
    Rpi::Dma::Ti tid ; // just for debugging/monitoring
    tid.srcInc()= true ;
    tid.permap() = Rpi::Pwm::DmaC::Permap ;
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
  
    Console::Dma::Lib::write(&ctl,           tid,        ts.get(),8u, Rpi::Pwm::Fifo::Address,            8u  ) ; // prefix for monitoring
    Console::Dma::Lib::write(&ctl,Rpi::Dma::Ti(),Rpi::Timer::cLoAddr,        ts.get(),0u,sizeof(uint32_t)) ;
    ctl.write            (                tix,Rpi::Bus::null_addr, Rpi::Pwm::Fifo::Address,     nwords*4,0 ) ;
    Console::Dma::Lib::write(&ctl,Rpi::Dma::Ti(),Rpi::Timer::cLoAddr,        ts.get(),4u,sizeof(uint32_t)) ;
    Console::Dma::Lib::write(&ctl,           tid,        ts.get(),8u, Rpi::Pwm::Fifo::Address,           12u  ) ; // postfix for monitoring
  
    ts->as<uint32_t*>()[2] = 0xffffffff ; 
    ts->as<uint32_t*>()[3] = 0x0 ;
    ts->as<uint32_t*>()[4] = 0xffffffff ; // repeat
  
    // (3) ---- run ----

    Console::Pwm::Lib::setup(&pwm,pwm_index) ; pwm.range(pwm_index).write(nbits) ; // [todo] leave to ctrl
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

void Console::Pwm::invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (argL->empty() || argL->peek() == "help") { 
	std::cout << "arguments: MODE [help]\n"
		  << '\n'
		  << "MODE : clear      # clear Status register\n"
		  << "     | control    # set Control registers\n"
		  << "     | data       # set Data register\n"
		  << "     | dmac       # set DMA-Control register\n"
		  << "     | enqueue    # put word(s) into FIFO register\n"
		  << "     | frequency  # estimate current output rate\n"
		  << "     | range      # set Range register\n"
		  << "     | status     # display register values\n"
		  << '\n'
		  << "     | dma        # send data in DMA/FIFO mode\n"
		  << "     | dummy      # send dummy data in DMA/FIFO mode\n" ;
	return ;
    }

    std::map<std::string,void(*)(Rpi::Peripheral*,Ui::ArgL*)> map = {
	{ "clear"    ,    clear },
	{ "control"  ,  control },
	{ "data"     ,     data },
	{ "dma"      ,      dma },
	{ "dmac"     ,     dmaC },
	{ "dummy"    ,    dummy },
	{ "enqueue"  ,  enqueue },
	{ "frequency",frequency },
	{ "range"    ,    range },
	{ "status"   ,   status },
    } ;
    argL->pop(map)(rpi,argL) ;
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

#if 0
    // [todo] test case to force BERR
    else if (arg == "berr")
    {
	auto c = pwm.control().read() ;
	pwm.control().write(c) ;
	pwm.control().write(c) ;
	// [todo] comment as defect-test-case
    }
#endif
