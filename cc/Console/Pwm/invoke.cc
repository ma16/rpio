// BSD 2-Clause License, see github.com/ma16/rpio

#include "../rpio.h"
#include <deque>
#include <iomanip>
#include <iostream>
#include <Console/Dma/Lib.h>
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
	      << std::setw(5) << s.test(Status::Full) ;

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
	std::cout << "arguments: DIX [CS] [TI] [MEM] FILE\n"
		  << '\n'
		  << " DIX : 0..15  # DMA channel index to use\n"
		  << '\n'
		  << "  CS = DMA control and status\n"
		  << "  TI = DMA transfer information\n"
		  << " MEM = type of memory to be used for DMA setup\n"
		  << "FILE = name of file with data to be sent\n"
		  << std::flush ;
	return ;
    }

    using namespace Console::Dma ; // makes Lib:: visible
    
    // ---- configuration ----

    auto dma_index = Ui::strto(argL->pop(),Rpi::Dma::Ctrl::Index()) ;
    
    auto dma_cs = Lib::optCs(argL,Rpi::Dma::Cs()) ;
  
    auto dma_ti = Lib::optTi(argL,Rpi::Dma::Ti::send(Rpi::Pwm::DmaC::Permap)) ;

    auto allocator = RpiExt::VcMem::
	getFactory(rpi,argL,RpiExt::VcMem::defaultFactory()) ;
  
    auto file_data = RpiExt::VcMem::read(argL->pop(),allocator.get()) ;

    argL->finalize() ;

    // ---- remaining setup ----

  
    Lib::Control ctl(allocator->allocate((2+1) * 32)) ;
  
    auto t0 = allocator->allocate(sizeof(uint32_t)) ; 
    Lib::write(&ctl,Rpi::Dma::Ti(),
	       Rpi::Timer::Address,
	       t0.get(),0u,
	       sizeof(uint32_t)) ;
    
    Lib::write(&ctl,dma_ti,
	       file_data.get(),0u,
	       Rpi::Pwm::Fifo::Address,
	       file_data->nbytes()) ;
    
    auto t1 = allocator->allocate(sizeof(uint32_t)) ; 
    Lib::write(&ctl,Rpi::Dma::Ti(),
	       Rpi::Timer::Address,
	       t1.get(),0u,
	       sizeof(uint32_t)) ;

    // (3) ---- run ----

    auto dma_channel = Rpi::Dma::Ctrl(rpi).channel(dma_index) ;
    dma_channel.setup(ctl.addr(),dma_cs) ;
    dma_channel.start() ;
    // ...fill up the PWM queue first ?!

    while (0 != (dma_channel.getCs().active().bits()))
	Posix::nanosleep(1e3) ;
    // ...arbitrary sleep value
  
    // (4) ---- log statistics ----
  
    std::cout.setf(std::ios::scientific) ;
    std::cout.precision(2) ;
    auto dt = static_cast<double>(t1->as<uint32_t*>()[0] -
				  t0->as<uint32_t*>()[0]) / 1e6 ;
    std::cout << dt << "s "
	      << static_cast<double>(file_data->nbytes()/4*32)/dt << "/s\n" ;
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
		  << "     | dma        # put word(s) into FIFO register (DMA)\n"
		  << "     | dmac       # set DMA-Control register\n"
		  << "     | enqueue    # put word(s) into FIFO register (CPU)\n"
		  << "     | frequency  # estimate current output rate\n"
		  << "     | range      # set Range register\n"
		  << "     | status     # display register values\n" ;
	return ;
    }

    std::map<std::string,void(*)(Rpi::Peripheral*,Ui::ArgL*)> map = {
	{ "clear"    ,    clear },
	{ "control"  ,  control },
	{ "data"     ,     data },
	{ "dma"      ,      dma },
	{ "dmac"     ,     dmaC },
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

// observed behavior:
// --if a gap occurs right here, before wrting to the queue, we
//   won't detect it. there is actually no indication by any status
//   flag (BERR,GAPO,RERR) whether a gap occured -- even the spec
//   says so.
// --even though the last word of the queue has been read by the
//   serializer, the transfer of the previous (!) word is still in
//   progress. PWEN=0 at this point will interrupt the transfer in
//   middle of the previous word.
// --there is no indication by any status flag (BERR,GAPO,RERR) when
//   the transfer is completed (even so the spec says so).
// --after the transfer of the last word is completed, the transfer
//   of the last word will be indefinitely repeated regardless
//   whether the RPTL flag is set or not (spec is wrong again).
// --hence, if the transmission is stopped (PWEN=0) as soon as the
//   FIFO gets empty, the caller should append two (?) dummy word
//   (i.e. all bits high or all bits low, as needed).
