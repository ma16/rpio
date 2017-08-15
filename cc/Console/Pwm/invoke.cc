// BSD 2-Clause License, see github.com/ma16/rpio

#include "../rpio.h"
#include <chrono>
#include <cstring> // memset
#include <deque>
#include <iomanip>
#include <iostream>
#include <thread> // this_thread::sleep_for
#include <Neat/cast.h>
#include <Neat/stream.h>
#include <Rpi/Counter.h>
#include <RpiExt/Dma/Control.h>
#include <RpiExt/VcMem.h>
#include <RpiExt/Pwm.h>
#include <Rpi/Ui/Bus/Memory.h>
#include <Rpi/Ui/Dma.h>
#include <Ui/strto.h>

static std::vector<uint32_t> readFile(std::string const &name)
{
    std::ifstream is ; Neat::open(&is,name) ;
    auto nbytes = Neat::demote<size_t>(Neat::size(&is).as_unsigned()) ; 
    auto nwords = nbytes / 4 ;
    if (nwords * 4 != nbytes)
	throw std::runtime_error("file-size must be a multiple of 4 bytes") ;
    std::vector<uint32_t> v(nwords) ;
    Neat::read(&is,&v[0],Neat::ustreamsize::make(nbytes)) ;
    return v ;
}

static std::string statusString(Rpi::Pwm::Status::Word status)
{
    using Status = Rpi::Pwm::Status ;
    std::ostringstream os ;
    os << "berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: "
       << Status::Berr(status).raised() << ' '
       << Status::Empt(status).raised() << ' '
       << Status::Full(status).raised() << ' '
       << Status::Gap1(status).raised() << ' '
       << Status::Gap2(status).raised() << ' '
       << Status::Rerr(status).raised() << ' '
       << Status::Sta1(status).raised() << ' '
       << Status::Sta2(status).raised() << ' '
       << Status::Werr(status).raised() ;
    return os.str() ;
}

// ----

static void berr(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (argL->empty() || argL->peek() == "help")
    {
	std::cout
	    << "arguments: N\n"
	    << '\n'
	    << "This call is used  to force the peripheral to raise the BERR\n"
	    << "flag. The function reads the Control register N times before\n"
	    << "writing it back. This is done in a loop with 100 repetitions.\n"
	    << "With N<5 the BERR flag is almost often raised, with N>20 not.\n"
	    ;
	return ;
    }
    auto n = Ui::strto<size_t>(argL->pop()) ;
    argL->finalize() ;
    Rpi::Pwm pwm(rpi) ;
    for (auto i=0 ; i<100 ; ++i)
    {
	decltype(pwm.control().read()) w ;
	for (decltype(n) j=0 ; j<n ; ++j)
	    w = pwm.control().read() ;
	pwm.control().write(w) ;
    }
}

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
	
	if (arg == "all")
	{
	    w %= Status::Berr::make(1) ;
	    w %= Status::Gap1::make(1) ;
	    w %= Status::Gap2::make(1) ;
	    w %= Status::Rerr::make(1) ;
	    w %= Status::Werr::make(1) ;
	}
	else if (arg == "berr") w %= Status::Berr::make(1) ;
	else if (arg == "gap1") w %= Status::Gap1::make(1) ;
	else if (arg == "gap2") w %= Status::Gap2::make(1) ;
	else if (arg == "rerr") w %= Status::Rerr::make(1) ;
	else if (arg == "werr") w %= Status::Werr::make(1) ;
	
	else throw std::runtime_error("not supported option:<"+arg+'>') ;
    }
    Rpi::Pwm(rpi).status().write(w) ;
}

static void control(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (argL->empty() || argL->peek() == "help")
    {
	std::cout
	    << "arguments: OPTION+\n"
	    << "\n"
	    << "clear             # clear FIFO\n"
	    << "(+|-) mode (1|2)  # Serial or PWM mode\n"
	    << "(+|-) msen (1|2)  # Mark-Space or coherent signal\n"
	    << "(+|-) pwen (1|2)  # enable or disable transmission\n"
	    << "(+|-) pola (1|2)  # inverse output polarity or don't\n"
	    << "(+|-) rptl (1|2)  # repeat last word when idle or don't\n"
	    << "(+|-) sbit (1|2)  # High or Low output when off\n"
	    << "(+|-) usef (1|2)  # read FIFO or use Data register\n"
	    << "\n"
	    << "E.g. -mode2 activates PWM mode for channel #2\n"
	    ;
	return ;
    }
  
    Rpi::Pwm pwm(rpi) ;
    auto w = pwm.control().read() ;
    while (!argL->empty())
    {
	using Control = Rpi::Pwm::Control ;
    
	auto arg = argL->pop() ;
	if (false) ;

	else if (arg == "clear")  w %= Control::Clrf::make(1) ;
	
    	else if (arg == "-mode1") w %= Control::Mode1::make(0) ;
    	else if (arg == "-mode2") w %= Control::Mode2::make(0) ;
    	else if (arg == "-msen1") w %= Control::Msen1::make(0) ;
    	else if (arg == "-msen2") w %= Control::Msen2::make(0) ;
    	else if (arg == "-pola1") w %= Control::Pola1::make(0) ;
    	else if (arg == "-pola2") w %= Control::Pola2::make(0) ;
    	else if (arg == "-pwen1") w %= Control::Pwen1::make(0) ;
    	else if (arg == "-pwen2") w %= Control::Pwen2::make(0) ;
    	else if (arg == "-rptl1") w %= Control::Rptl1::make(0) ;
    	else if (arg == "-rptl2") w %= Control::Rptl2::make(0) ;
    	else if (arg == "-sbit1") w %= Control::Sbit1::make(0) ;
    	else if (arg == "-sbit2") w %= Control::Sbit2::make(0) ;
    	else if (arg == "-usef1") w %= Control::Usef1::make(0) ;
    	else if (arg == "-usef2") w %= Control::Usef2::make(0) ;

    	else if (arg == "+mode1") w %= Control::Mode1::make(1) ;
    	else if (arg == "+mode2") w %= Control::Mode2::make(1) ;
    	else if (arg == "+msen1") w %= Control::Msen1::make(1) ;
    	else if (arg == "+msen2") w %= Control::Msen2::make(1) ;
    	else if (arg == "+pola1") w %= Control::Pola1::make(1) ;
    	else if (arg == "+pola2") w %= Control::Pola2::make(1) ;
    	else if (arg == "+pwen1") w %= Control::Pwen1::make(1) ;
    	else if (arg == "+pwen2") w %= Control::Pwen2::make(1) ;
    	else if (arg == "+rptl1") w %= Control::Rptl1::make(1) ;
    	else if (arg == "+rptl2") w %= Control::Rptl2::make(1) ;
    	else if (arg == "+sbit1") w %= Control::Sbit1::make(1) ;
    	else if (arg == "+sbit2") w %= Control::Sbit2::make(1) ;
    	else if (arg == "+usef1") w %= Control::Usef1::make(1) ;
    	else if (arg == "+usef2") w %= Control::Usef2::make(1) ;

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
    if (index == Rpi::Pwm::Channel1)
	Rpi::Pwm(rpi).data1().write(word) ;
    else
	Rpi::Pwm(rpi).data2().write(word) ;
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
	using DmaC = Rpi::Pwm::DmaC ;
	auto arg = argL->pop({"dreq","enable","panic"}) ;
	if (false) ;
	
	else if (arg == 0)
	{
	    auto arg = Ui::strto(argL->pop(),DmaC::Dreq::Uint()) ;
	    w %= DmaC::Dreq(arg) ;
	}
	else if (arg == 1)
	{
	    auto arg = Ui::strto(argL->pop(),DmaC::Enable::Uint()) ;
	    w %= DmaC::Enable(arg) ;
	}
	else if (arg == 2)
	{
	    auto arg = Ui::strto(argL->pop(),DmaC::Panic::Uint()) ;
	    w %= DmaC::Panic(arg) ;
	} 
    }
    argL->finalize() ;
    pwm.dmaC().write(w) ;
}
	       
static void fifo_cpu(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (argL->empty() || argL->peek() == "help")
    {
	std::cout << "arguments: [ -c U32 | -u ] FILE\n"
		  << '\n'
		  << "-c  # detect underruns, this requires padding\n"
		  << "-u  # unpaced write (ignores FIFO status)\n"
		  << "default: fill FIFO whenver there is space\n"
		  << '\n'
		  << "FILE = file with binary data to enqueue\n"
	    ;
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
    auto data = readFile(argL->pop()) ;
    argL->finalize() ;
    
    Rpi::Pwm pwm(rpi) ; RpiExt::Pwm ext(rpi) ;

    Rpi::Pwm::Status::Word status ;
    if (mode == Checked)
    {
	auto head = ext.headstart(&data[0],data.size()) ;
	auto tail = ext.convey(&data[head],data.size()-head,padding) ;
	status = pwm.status().read() ;
	if (head + tail < data.size())
	    std::cout << "underrun detected after "
		      << head + tail << " / "
		      << data.size() << " words\n" ;
    }
    else if (mode == Unpaced)
    {
	for (auto w: data)
	    pwm.fifo().write(w) ;
	status = pwm.status().read() ;
    }
    else
    {
	assert(mode == Topup) ;
	auto head = ext.headstart(&data[0],data.size()) ;
	ext.write(&data[head],data.size()-head) ;
	status = pwm.status().read() ;
    }
    std::cout << statusString(status) << '\n' ;
}

static void fifo_dma(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (argL->empty() || argL->peek() == "help")
    {
	std::cout
	    << "arguments: CHANNEL [CS] [TI] [ALLOC] FILE\n"
	    << '\n'
	    << "CHANNEL = DMA channel (0..15)\n"
	    << '\n'
	    << "CS = DMA Control and Status:\n"
	    << Rpi::Ui::Dma::csSynopsis()
	    << '\n'
	    << "TI = DMA Transfer Information:\n"
	    << Rpi::Ui::Dma::tiSynopsis()
	    << '\n'
	    << "ALLOC = allocator for DMA bus memory:\n"
	    << Rpi::Ui::Bus::Memory::allocatorSynopsis()
	    << '\n'
	    << "FILE = file with binary data to enqueue\n"
	    ;
	return ;
    }

    // dma channel index
    auto index = Ui::strto(argL->pop(),Rpi::Dma::Ctrl::Index()) ;

    // dma channel
    auto channel = Rpi::Dma::Ctrl(rpi).channel(index) ;

    // dma control and status register
    auto cs = Rpi::Ui::Dma::getCs(argL,Rpi::Dma::Cs()) ;

    // dma transfer information 
    auto ti = Rpi::Ui::Dma::getTi(argL,Rpi::Dma::Ti::make(Rpi::Dma::Ti::Pwm)) ;

    // bus memory (video core memory) allocator
    auto allocator = Rpi::Ui::Bus::Memory::
	getAllocator(rpi,argL,RpiExt::VcMem::defaultAllocator()) ;

    // data to transmit
    auto data = readFile(argL->pop()) ;
    
    argL->finalize() ;

    // stop (all) transmissions in order to top-up FIFO
    auto control = Rpi::Pwm(rpi).control().read() ;
    {
	auto stop = control ;
	stop %= Rpi::Pwm::Control::Pwen1::make(0) ;
	stop %= Rpi::Pwm::Control::Pwen2::make(0) ;
	Rpi::Pwm(rpi).control().write(stop) ;
    }
    
    // (user) data head-block that fits into FIFO buffer
    auto head = RpiExt::Pwm(rpi).topUp(&data[0],data.size()) ;
    
    // manage DMA control block list
    RpiExt::Dma::Control cb(allocator) ;

    // DMA: restore PWM control register
    auto enable = allocator->allocate(sizeof(uint32_t)) ;
    (*enable->as<uint32_t*>()) = control.value() ;
    cb.add(Rpi::Dma::Ti::Word(),enable.get(),Rpi::Pwm::Control::Address) ;
    
    // DMA: take time t0 (DMA transfer just started)
    auto t0 = allocator->allocate(sizeof(uint32_t)) ;
    cb.add(Rpi::Dma::Ti::Word(),Rpi::Counter::Address,t0.get()) ;

    // DMA: (user) data tail-block (did not fit into FIFO buffer)
    auto rest = (data.size() - head) * 4 ;
    if (rest > 0)
    {
	auto tail = allocator->allocate(rest) ;
	memcpy(tail->as<void*>(),&data[head],rest) ;
	cb.add(ti,tail.get(),Rpi::Pwm::Fifo::Address) ;
    }

    // DMA: read PWM Status flags
    auto status = allocator->allocate(sizeof(uint32_t)) ;
    cb.add(Rpi::Dma::Ti::Word(),Rpi::Pwm::Status::Address,status.get()) ;

    // DMA: take time t1 (when DMA transfer was completed)
    auto t1 = allocator->allocate(sizeof(uint32_t)) ;
    cb.add(Rpi::Dma::Ti::Word(),Rpi::Counter::Address,t1.get()) ;
    
    // execute actual DMA transfer
    channel.setup(cb.address(),cs) ;
    channel.start() ;
    while (0 != (channel.getCs().active().bits()))
	std::this_thread::sleep_for(std::chrono::milliseconds(1)) ;
    
    // [note] If there is any exception, then the DMA must stop first
    // and thereafter the memory can be released. On process abortion,
    // the (VC) memory stays allocated and the DMA continues running.

    auto w = Rpi::Pwm::Status::Word::coset(*status->as<uint32_t*>()) ;
    auto d = (*t1->as<uint32_t*>()) - (*t0->as<uint32_t*>()) ;
    std::cout << statusString(w) << " duration: " << d << '\n' ;
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
    if (control.test(Rpi::Pwm::Control::Pwen1::Digit))
    {
	if (!control.test(Rpi::Pwm::Control::Usef1::Digit))
	    throw std::runtime_error("channel #1 enabled w/o USEF=1") ;
	width = pwm.range1().read() ;
	++nchannels ;
    }
    if (control.test(Rpi::Pwm::Control::Pwen2::Digit))
    {
	if (!control.test(Rpi::Pwm::Control::Usef2::Digit))
	    throw std::runtime_error("channel #2 enabled w/o USEF=1") ;
	width = pwm.range2().read() ;
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
    if (index == Rpi::Pwm::Channel1)
	Rpi::Pwm(rpi).range1().write(word) ;
    else
	Rpi::Pwm(rpi).range2().write(word) ;
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
    
    using   DmaC = Rpi::Pwm::  DmaC ;
    using Status = Rpi::Pwm::Status ;
    
    auto d = pwm.dmaC().read() ;
    auto s = pwm.status().read() ;
    std::cout << '\n'
	      << "berr=" << s.test(Status::Berr::Digit) << ' '
	      << "empt=" << s.test(Status::Empt::Digit) << ' '
	      << "full=" << s.test(Status::Full::Digit) << ' ' 
	      << "rerr=" << s.test(Status::Rerr::Digit) << ' '
	      << "werr=" << s.test(Status::Werr::Digit) << ' ' 
	      << "DMA: "
	      << "enable=" << DmaC::Enable(d).count() << ' '
	      << "panic="  << DmaC:: Panic(d).count() << ' '
	      << "dreq="   << DmaC::  Dreq(d).count() << "\n\n" ;

    std::cout
	<< std::hex
	<< "# | gap sta | mode msen pola pwen rptl sbit usef |     data |     range\n"
	<< "--+---------+------------------------------------+----------+----------\n" ;
    auto c = pwm.control().read() ;
    auto i = Rpi::Pwm::Channel1 ;
    do
    {
	auto bank = pwm.select(i) ;
	auto data = (i == Rpi::Pwm::Channel1)
	    ? pwm.data1().read()
	    : pwm.data2().read() ;
	auto range = (i == Rpi::Pwm::Channel1)
	    ? pwm.range1().read()
	    : pwm.range2().read() ;
	
	std::cout << std::setw(1) << i.value() + 1
		  << " |"
		  << std::setw(4) << s.test(bank.gap)
		  << std::setw(4) << s.test(bank.sta)
		  << " |"
		  << std::setw(5) << c.test(bank.mode)
		  << std::setw(5) << c.test(bank.msen)
		  << std::setw(5) << c.test(bank.pola)
		  << std::setw(5) << c.test(bank.pwen)
		  << std::setw(5) << c.test(bank.rptl)
		  << std::setw(5) << c.test(bank.sbit)
		  << std::setw(5) << c.test(bank.usef)
		  << " |"
		  << std::setw(9) << data
		  << " |"
		  << std::setw(9) << range
		  << '\n' ;
    }
    while (i.next()) ;
}

void Console::Pwm::invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (argL->empty() || argL->peek() == "help") { 
	std::cout << "arguments: MODE [help]\n"
		  << '\n'
		  << "MODE : berr       # raise BERR status flag\n"
		  << "     | clear      # clear Status register\n"
		  << "     | control    # set Control registers\n"
		  << "     | data       # set Data register\n"
		  << "     | dmac       # set DMA-Control register\n"
		  << "     | fifo-cpu   # write FIFO by CPU\n"
		  << "     | fifo-dma   # write FIFO by DMA\n"
		  << "     | frequency  # estimate active transfer rate\n"
		  << "     | range      # set Range register\n"
		  << "     | status     # display register values\n" ;
	return ;
    }

    std::map<std::string,void(*)(Rpi::Peripheral*,Ui::ArgL*)> map = {
	{ "berr"     ,     berr },
	{ "clear"    ,    clear },
	{ "control"  ,  control },
	{ "data"     ,     data },
	{ "dmac"     ,     dmaC },
	{ "fifo-cpu" , fifo_cpu },
	{ "fifo-dma" , fifo_dma },
	{ "frequency",frequency },
	{ "range"    ,    range },
	{ "status"   ,   status },
    } ;
    argL->pop(map)(rpi,argL) ;
}

// --------------------------------------------------------------------

// observed behavior [open issues]
//
// --STA may remain set even if PWEN is cleared. This can be observed
//   sometimes for operations that cause BERR=1. In order to clear
//   STA, BERR needs to be cleared first, thereafter PWEN.
// --how long does it take when the FIFO gets empty that the serializer
//   finishes; or, when does the serializer read the next word from FIFO?
// --if both channels are used (even at the same frequency) and e.g.
//   one of them is still transmitting (i.e. RPTL) new FIFO words
//   will be probably be not in sync transmitted
//
// If taking a time-stamp, the serializer may be at any point in a
// (long) word. hence, the time-stamp should be taken the moment
// the FIFO's _full_ flag turns to false. This may become a non-issue
// if there are the odds of process suspensions anyway. Still, if there
// is a sequence (full=1,time-stamp,full=1) the point of time is
// guaranteed to be in the period that takes a word to serialize.
