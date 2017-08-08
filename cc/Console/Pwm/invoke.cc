// BSD 2-Clause License, see github.com/ma16/rpio

#include "../rpio.h"
#include <chrono>
#include <deque>
#include <iomanip>
#include <iostream>
#include <thread> // this_thread::sleep_for
#include <Neat/stream.h>
#include <Rpi/Timer.h>
#include <RpiExt/Dma/Control.h>
#include <RpiExt/VcMem.h>
#include <RpiExt/Pwm.h>
#include <Rpi/Ui/Bus/Memory.h>
#include <Rpi/Ui/Dma.h>
#include <Ui/strto.h>

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
	    << "With N<5 the BERR flag is almost often set, with N>20 none.\n"
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
	    w.at(Status::Berr) = 1 ;
	    w.at(Status::Gap1) = 1 ;
	    w.at(Status::Gap2) = 1 ;
	    w.at(Status::Rerr) = 1 ;
	    w.at(Status::Werr) = 1 ;
	}
	else if (arg == "berr") w.at(Status::Berr) = 1 ;
	else if (arg == "gap1") w.at(Status::Gap1) = 1 ;
	else if (arg == "gap2") w.at(Status::Gap2) = 1 ;
	else if (arg == "rerr") w.at(Status::Rerr) = 1 ;
	else if (arg == "werr") w.at(Status::Werr) = 1 ;
	
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
	    << "clear            # clear FIFO (single shot)\n"
	    << "mode[1|2]=[0|1]  # 1=Serial 0=PWM\n"
	    << "msen[1|2]=[0|1]  # 1=Mark-Space 0=coherent (if mode=0)\n"
	    << "pwen[1|2]=[0|1]  # 1=start transmission 0=stop\n"
	    << "pola[1|2]=[0|1]  # 1=inverse output polarity\n"
	    << "rptl[1|2]=[0|1]  # 1=repeat word when idle (if usef=1)\n"
	    << "sbit[1|2]=[0|1]  # 1=High 0=Low output when idle\n"
	    << "usef[1|2]=[0|1]  # 1=read FIFO 0=read Data register\n"
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

	else if (arg == "clear")  w.at(Control::Clrf) = 1 ;
	
    	else if (arg == "mode1=0") w.at(Control::Mode1) = 0 ;
    	else if (arg == "mode2=0") w.at(Control::Mode2) = 0 ;
    	else if (arg == "msen1=0") w.at(Control::Msen1) = 0 ;
    	else if (arg == "msen2=0") w.at(Control::Msen2) = 0 ;
    	else if (arg == "pola1=0") w.at(Control::Pola1) = 0 ;
    	else if (arg == "pola2=0") w.at(Control::Pola2) = 0 ;
    	else if (arg == "pwen1=0") w.at(Control::Pwen1) = 0 ;
    	else if (arg == "pwen2=0") w.at(Control::Pwen2) = 0 ;
    	else if (arg == "rptl1=0") w.at(Control::Rptl1) = 0 ;
    	else if (arg == "rptl2=0") w.at(Control::Rptl2) = 0 ;
    	else if (arg == "sbit1=0") w.at(Control::Sbit1) = 0 ;
    	else if (arg == "sbit2=0") w.at(Control::Sbit2) = 0 ;
    	else if (arg == "usef1=0") w.at(Control::Usef1) = 0 ;
    	else if (arg == "usef2=0") w.at(Control::Usef2) = 0 ;

    	else if (arg == "mode1=1") w.at(Control::Mode1) = 1 ;
    	else if (arg == "mode2=1") w.at(Control::Mode2) = 1 ;
    	else if (arg == "msen1=1") w.at(Control::Msen1) = 1 ;
    	else if (arg == "msen2=1") w.at(Control::Msen2) = 1 ;
    	else if (arg == "pola1=1") w.at(Control::Pola1) = 1 ;
    	else if (arg == "pola2=1") w.at(Control::Pola2) = 1 ;
    	else if (arg == "pwen1=1") w.at(Control::Pwen1) = 1 ;
    	else if (arg == "pwen2=1") w.at(Control::Pwen2) = 1 ;
    	else if (arg == "rptl1=1") w.at(Control::Rptl1) = 1 ;
    	else if (arg == "rptl2=1") w.at(Control::Rptl2) = 1 ;
    	else if (arg == "sbit1=1") w.at(Control::Sbit1) = 1 ;
    	else if (arg == "sbit2=1") w.at(Control::Sbit2) = 1 ;
    	else if (arg == "usef1=1") w.at(Control::Usef1) = 1 ;
    	else if (arg == "usef2=1") w.at(Control::Usef2) = 1 ;

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

static void dma(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (argL->empty() || argL->peek() == "help")
    {
	std::cout
	    << "arguments: CHANNEL [CS] [TI] [ALLOC] FILE\n"
	    << '\n'
	    << "CHANNEL = DMA channel (0..15)\n"
	    << '\n'
	    << "CS = DMA control and status:\n"
	    << Rpi::Ui::Dma::csSynopsis()
	    << '\n'
	    << "TI = DMA transfer information:\n"
	    << Rpi::Ui::Dma::tiSynopsis()
	    << '\n'
	    << "ALLOC = allocator for DMA memory:\n"
	    << Rpi::Ui::Bus::Memory::allocatorSynopsis()
	    << '\n'
	    << "FILE = name of file with data to be sent\n"
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
    auto ti = Rpi::Ui::Dma::getTi(argL,Rpi::Dma::Ti::make(Rpi::Pwm::DmaC::Permap)) ;

    // bus memory (video core memory) allocator
    auto allocator = Rpi::Ui::Bus::Memory::
	getAllocator(rpi,argL,RpiExt::VcMem::defaultAllocator()) ;

    // bus memory with data to transfer
    auto data = Rpi::Ui::Bus::Memory::read(argL->pop(),allocator.get()) ;

    argL->finalize() ;

    // manage dma control block list
    RpiExt::Dma::Control cb(allocator) ;

    // control block to copy (user) data to PWM FIFO
    cb.add(ti,data.get(),Rpi::Pwm::Fifo::Address) ;

    // intialize dma controller
    channel.setup(cb.address(),cs) ;

    // start dma transfer
    channel.start() ;

    // wait until transfer is completed
    while (0 != (channel.getCs().active().bits()))
	std::this_thread::sleep_for(std::chrono::milliseconds(1)) ;
    
    // [note] If there is any exception, then the DMA must stop first
    // and thereafter the memory can be released. On process abortion,
    // the (VC) memory stays allocated and the DMA continues running.
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
    Rpi::Pwm pwm(rpi) ;
    if (mode == Checked)
    {
	auto nwords = RpiExt::Pwm(rpi).convey(&data[0],data.size(),padding) ;
	if (nwords < data.size())
	    std::cout << nwords << " / " << data.size() << " words written\n" ;
    }
    else if (mode == Unpaced)
    {
	for (auto w: data)
	    pwm.fifo().write(w) ;
    }
    else
    {
	assert(mode == Topup) ;
	RpiExt::Pwm(rpi).write(&data[0],data.size()) ;
    }
    using Status = Rpi::Pwm::Status ;
    auto s = pwm.status().read() ;
    std::cout << "berr=" << s.test(Status::Berr) << ' '
	      << "empt=" << s.test(Status::Empt) << ' '
	      << "full=" << s.test(Status::Full) << ' '
	      << "gap1=" << s.test(Status::Gap1) << ' '
	      << "gap2=" << s.test(Status::Gap2) << ' '
	      << "rerr=" << s.test(Status::Rerr) << ' '
	      << "sta1=" << s.test(Status::Sta1) << ' '
	      << "sta2=" << s.test(Status::Sta2) << ' '
	      << "werr=" << s.test(Status::Werr) << '\n' ;
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
	      << "panic=" << static_cast<int>(d.panic) << " "
	      << "dreq="  << static_cast<int>(d. dreq) << "\n\n" ;
    
    using Status = Rpi::Pwm::Status ;
    auto s = pwm.status().read() ;
    std::cout << "berr rerr werr empt full\n"
	      << "------------------------\n"
	      << std::setw(4) << s.test(Status::Berr)
	      << std::setw(5) << s.test(Status::Rerr)
	      << std::setw(5) << s.test(Status::Werr)
	      << std::setw(5) << s.test(Status::Empt)
	      << std::setw(5) << s.test(Status::Full)
	      << '\n' ;

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

void Console::Pwm::invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (argL->empty() || argL->peek() == "help") { 
	std::cout << "arguments: MODE [help]\n"
		  << '\n'
		  << "MODE : berr       # raise BERR status flag\n"
		  << "     | clear      # clear Status register\n"
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
	{ "berr"     ,     berr },
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

// --------------------------------------------------------------------

// observed behavior [open issues]
//
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
// --it appears, that gaps are detected on channel #2 but not on
//   channel #1
// --STA may remain set even if PWEN is cleared. This can be observed
//   sometimes for operations that cause BERR=1. In order to clear
//   STA, BERR needs to be cleared first, thereafter PWEN.
// --EMPT1: the serializer may still be busy with the transfer even
//   after the FIFO gets empty. So the flag is no indicator to disable
//   PWEN after the end of a transmission.
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
//
// There seems to be no way to figure out when the PWM peripheral
// actually finishes processing of all the FIFO entries. Even if
// the status flag indicates an empty FIFO, the serializer may
// still be busy. Sometimes it appears as if the serializer reads two
// FIFO entries ahead. [provide test-case]
//
// So, what the client needs to do, is to put two additional words
// into the FIFO. These words may be transmitted, or not. The
// client should keep that in mind and set the levels
// appropriately. Also, the last word is always transmitted again
// and again [regardless of the RPT flag] if the FIFO runs empty (and
// if the serializer gets to this point at all).
