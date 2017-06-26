// BSD 2-Clause License, see github.com/ma16/rpio

#include "../rpio.h"
#include "Lib.h"
#include <Console/Dma/Lib.h>
#include <Console/Memory/Lib.h>
#include <Neat/stream.h>
#include <Posix/base.h>
#include <Rpi/GpuMem.h>
#include <Rpi/Timer.h>
#include <Ui/ostream.h>
#include <Ui/strto.h>
#include <iomanip>
#include <iostream>

static void ctrl(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
  if (argL->empty() || argL->peek() == "help") {
    std::cout << "arguments: CMD+\n"
	      << '\n'
	      << "CMD : index 0|1  # set channel (@channel) for subsequent commands\n"
	      << "    | msen  0|1  # @channel: enable M/S PWM\n"
	      << "    | usef  0|1  # @channel: use FIFO\n"
	      << "    | pola  0|1  # @channel: inverse output polarity\n"
	      << "    | sbit  0|1  # @channel: silence bit for gaps\n"
	      << "    | rptl  0|1  # @channel: repeat last data when FIFO is empty\n"
	      << "    | mode  0|1  # @channel: serializer (1) or PWM (0)\n"
	      << "    | pwen  0|1  # @channel: start (1) or stop (0)\n"
	      << "    | sync       # start both channels at the same time\n"
	      << "    | data  U32  # @channel: set data register\n"
	      << "    | range U32  # @channel: set range register\n"
	      << "    | queue U32  # enqueue word in FIFO\n"
	      << "    | clear      # clear FIFO\n"
	      << "    | reset      # reset status flags\n"
	      << "    | dma   0|1  # enable/disable DMA signals\n"
	      << "    | dreq  U8   # set DMA DREQ value\n"
	      << "    | panic U8   # set DMA PANIC value\n"
	      << std::flush ;
    return ;
  }
  
  Rpi::Pwm::Index index = Rpi::Pwm::Index() ;
  Rpi::Pwm pwm(rpi) ;
  while (!argL->empty()) {
    auto arg = argL->pop() ;
    if      (arg == "index") { index = Ui::strto(argL->pop(),Rpi::Pwm::Index()) ; }
    
    else if (arg ==  "msen") { auto c = pwm.getControl() ; auto x = c.get(index) ; x.msen = Ui::strto<bool>(argL->pop()) ; c.set(index,x) ; pwm.setControl(c) ; }
    else if (arg ==  "usef") { auto c = pwm.getControl() ; auto x = c.get(index) ; x.usef = Ui::strto<bool>(argL->pop()) ; c.set(index,x) ; pwm.setControl(c) ; }
    else if (arg ==  "pola") { auto c = pwm.getControl() ; auto x = c.get(index) ; x.pola = Ui::strto<bool>(argL->pop()) ; c.set(index,x) ; pwm.setControl(c) ; }
    else if (arg ==  "sbit") { auto c = pwm.getControl() ; auto x = c.get(index) ; x.sbit = Ui::strto<bool>(argL->pop()) ; c.set(index,x) ; pwm.setControl(c) ; }
    else if (arg ==  "rptl") { auto c = pwm.getControl() ; auto x = c.get(index) ; x.rptl = Ui::strto<bool>(argL->pop()) ; c.set(index,x) ; pwm.setControl(c) ; }
    else if (arg ==  "mode") { auto c = pwm.getControl() ; auto x = c.get(index) ; x.mode = Ui::strto<bool>(argL->pop()) ; c.set(index,x) ; pwm.setControl(c) ; }
    else if (arg ==  "pwen") { auto c = pwm.getControl() ; auto x = c.get(index) ; x.pwen = Ui::strto<bool>(argL->pop()) ; c.set(index,x) ; pwm.setControl(c) ; }

    else if (arg ==  "sync") { auto c = pwm.getControl() ; c.pwen1() = 1 ; c.pwen2() = 1 ; pwm.setControl(c) ; }
    
    else if (arg ==  "data") pwm. setData(index,Ui::strto<uint32_t>(argL->pop())) ;
    else if (arg == "range") pwm.setRange(index,Ui::strto<uint32_t>(argL->pop())) ;
    
    else if (arg == "clear") { auto c = pwm.getControl() ; c.clrf1() = 1 ; pwm.setControl(c) ; }
    else if (arg == "queue") pwm.write(Ui::strto<uint32_t>(argL->pop())) ;
    else if (arg == "reset") pwm.resetStatus(pwm.getStatus()) ;

    else if (arg ==   "dma") { auto d = pwm.getDmac() ; d.enable = Ui::strto   <bool>(argL->pop()) ; pwm.setDmac(d) ; }
    else if (arg ==  "dreq") { auto d = pwm.getDmac() ; d.  dreq = Ui::strto<uint8_t>(argL->pop()) ; pwm.setDmac(d) ; }
    else if (arg == "panic") { auto d = pwm.getDmac() ; d. panic = Ui::strto<uint8_t>(argL->pop()) ; pwm.setDmac(d) ; }
    
    else throw std::runtime_error("not supported option:<"+arg+'>') ;
  }
}

static void dma(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
  if (argL->empty() || argL->peek() == "help") {
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

  auto allof = Console::Memory::Lib::getFactory(rpi,argL,Console::Memory::Lib::defaultFactory()) ;
  
  auto data = Console::Memory::Lib::read(argL->pop(),allof.get()) ;

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
    ? Console::Memory::Lib::getFactory(rpi,argL)
    : Console::Memory::Lib::defaultFactory() ;
  
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

static void freq(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
  if (!argL->empty() && argL->peek() == "help") {
    std::cout << "arguments: [DELAY]\n"
	      << '\n'
	      << "DELAY = seconds to sample (default 0.1)\n"
	      << '\n'
	      << "you may want to set up the range register (#0) beforehand\n"
	      << std::flush ;
    return ;
  }
  auto duration = 0.1 ;
  if (!argL->empty())
    duration = Ui::strto<double>(argL->pop()) ;
  argL->finalize() ;
  Rpi::Pwm pwm(rpi) ;
  double f = Console::Pwm::Lib::frequency(&pwm,duration) ;
  std::cout.setf(std::ios::scientific) ;
  std::cout.precision(2) ;
  std::cout << f << std::endl ;
}

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
  c.set(index,x) ;
  pwm.setControl(c) ;
  auto ngaps = Console::Pwm::Lib::send(pwm,index,data.get(),nwords) ;
  std::cout << ngaps << std::endl ;
}

static void status(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
  if (!argL->empty() && argL->peek() == "help") {
    std::cout << "no arguments supported\n"
	      << std::flush ;
    return ;
  }
  argL->finalize() ;
  Rpi::Pwm pwm(rpi) ;
  auto d = pwm.getDmac() ;
  std::cout << std::hex << "DMA-Control: enable=" << d.enable << " panic=" << d.panic << " dreq=" << d.dreq << "\n\n" ;
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
  Rpi::Pwm::Index i ; do {
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

void Console::Pwm::invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
  if (argL->empty() || argL->peek() == "help") { 
    std::cout << "arguments: MODE [help]\n"
	      << '\n'
	      << "MODE : ctrl    # write peripheral registers\n"
	      << "     | dma     # send data in DMA/FIFO mode\n"
	      << "     | dummy   # send dummy data in DMA/FIFO mode\n"
	      << "     | freq    # estimate current frequency\n"
	      << "     | send    # send data in CPU/FIFO mode\n"
	      << "     | status  # display status\n"
	      << std::flush ;
    return ;
  }
  std::string arg = argL->pop() ;
  if      (arg ==   "ctrl")   ctrl(rpi,argL) ; 
  else if (arg ==    "dma")    dma(rpi,argL) ; 
  else if (arg ==  "dummy")  dummy(rpi,argL) ; 
  else if (arg ==   "freq")   freq(rpi,argL) ; 
  else if (arg ==   "send")   send(rpi,argL) ; 
  else if (arg == "status") status(rpi,argL) ; 
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
