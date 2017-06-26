// BSD 2-Clause License, see github.com/ma16/rpio

#include "../rpio.h"
#include <Rpi/Function.h>
#include <Rpi/Gpio.h>
#include <Ui/strto.h>
#include <iomanip>
#include <iostream>

static uint32_t getPins(Ui::ArgL *argL)
{
  auto arg = argL->pop() ;
  if (arg == "all") 
    return 0xffffffff ;
  if (arg == "-m") 
    return Ui::strto<uint32_t>(argL->pop()) ;
  if (arg == "-l") {
    auto arg = argL->pop() ;
    auto tail = arg.find(',') ;
    decltype(tail) head = 0 ;
    auto mask = 0u ;
    while (tail != arg.npos) {
      auto pin = Ui::strto(arg.substr(head,tail-head),Rpi::Pin()) ;
      mask |=  (1u << pin.value()) ;
      head = tail + 1 ;
      tail = arg.find(',',head) ;
    }
    auto pin = Ui::strto(arg.substr(head),Rpi::Pin()) ;
    mask |=  (1u << pin.value()) ;
    return mask ;
  }
  auto pin = Ui::strto(arg,Rpi::Pin()) ;
  return (1u << pin.value()) ;
}

// --------------------------------------------------------------------
    
static std::string mkhdr(uint32_t mask)
{
  std::ostringstream os ;
  for (auto i=0 ; i<32 ; ++i) 
    if (mask & (1u<<i))
      os << i/10 << ' ' ;
  os << '\n' ;
  for (auto i=0 ; i<32 ; ++i) 
    if (mask & (1u<<i))
      os << i%10 << ' ' ;
  return os.str() ;
}

static std::string mksep(uint32_t mask)
{
  std::ostringstream os ;
  for (auto i=0 ; i<32 ; ++i) 
    if (mask & (1u<<i))
      os << "--" ;
  return os.str() ;
}

static std::string mkstr(uint32_t mask,uint32_t bits)
{
  char s[32*2] ;
  auto ofs = 0u ;
  for (uint32_t m=1u ; m!=0 ; m<<=1) {
    if (m & mask) {
      s[ofs++] = (bits & m) ? '1' : '0' ;
      s[ofs++] = ' ' ;
    }
  }  
  return std::string(s,ofs) ;
} 

// --------------------------------------------------------------------
    
static void enableInvoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
  if (argL->empty() || argL->peek() == "help") {
    std::cout << "arguments: PINS TYPE [off]\n"
	      << '\n'
	      << "TYPE : rise\n"
	      << "     | fall\n"
	      << "     | high\n"
	      << "     | low\n"
	      << "     | async-rise\n"
	      << "     | async-fall\n"
	      << "     | any\n"
	      << std::flush ;
    return ;
  }
  auto pins = getPins(argL) ;
  auto type = argL->pop({"rise","fall","high","low","async-rise","async-fall","any"}) ;
  auto on = !argL->pop_if("off") ;
  argL->finalize() ;
  Rpi::Gpio gpio(rpi) ;
  if (type == 6) {
    auto event = Rpi::Gpio::EventN::first() ;
    do gpio.enable(pins,event.e(),on) ;
    while (event.next()) ;
  }
  else {
    auto event = Rpi::Gpio::EventN::make(type).e() ;
    gpio.enable(pins,event,on) ;
  }
}

static void modeInvoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
  if (argL->empty() || argL->peek() == "help") {
    std::cout << "arguments: PINS MODE\n"
	      << '\n'
	      << "MODE : i  # as input\n"
	      << "     | o  # as output\n"
	      << "     | 0  # as alternate function 0\n"
	      << "     | 1  # as alternate function 1\n"
	      << "     | 2  # as alternate function 2\n"
	      << "     | 3  # as alternate function 3\n"
	      << "     | 4  # as alternate function 4\n"
	      << "     | 5  # as alternate function 5\n"
	      << std::flush ;
    return ; 
  }
  auto pins = getPins(argL) ;
  auto i = argL->pop({"i","o","5","4","0","1","2","3"}) ;
  auto mode = Rpi::Gpio::ModeN::make(i).e() ;
  argL->finalize() ;
  Rpi::Gpio gpio(rpi) ;
  gpio.setMode(pins,mode) ;
}

static void outputInvoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
  if (argL->empty() || argL->peek() == "help") {
    std::cout << "arguments: PINS MODE\n"
	      << '\n'
	      << "MODE : lo  # set output low\n"
	      << "     | hi  # set output high\n"
	      << std::flush ;
    return ;
  }
  auto pins = getPins(argL) ;
  auto i = argL->pop({"hi","lo"}) ;
  auto output = Rpi::Gpio::OutputN::make(i).e() ;
  argL->finalize() ;
  Rpi::Gpio gpio(rpi) ;
  gpio.setOutput(pins,output) ;
}

static void pullInvoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
  if (argL->empty() || argL->peek() == "help") {
    std::cout << "arguments: PINS MODE\n"
	      << '\n'
	      << "MODE : down  # apply pull-down resistor\n"
	      << "     | off   # set tri-state (high-impedance)\n"
	      << "     | up    # apply pull-up resistor\n"
	      << std::flush ;
    return ;
  }
  auto pins = getPins(argL) ;
  auto i = argL->pop({"off","down","up"}) ;
  auto pull = Rpi::Gpio::PullN::make(i).e() ;
  argL->finalize() ;
  Rpi::Gpio gpio(rpi) ;
  gpio.setPull(pins,pull) ; 
}
    
static void resetInvoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
  if (!argL->empty() && argL->peek() == "help") {
    std::cout << "argument: [PINS]" << std::endl ;
    return ;
  }
  uint32_t pins = 0xffffffff ;
  if (!argL->empty())
    pins = getPins(argL) ;
  argL->finalize() ;
  Rpi::Gpio gpio(rpi) ;
  gpio.reset(pins) ;
}

static void statusDefault(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
  auto pins = 0xffffffffu ;
  if (!argL->empty())
    pins = getPins(argL) ;
  argL->finalize() ;
  Rpi::Gpio gpio(rpi) ;
  std::cout << mkhdr(pins) << '\n'
	    << mksep(pins) << '\n' ;
  auto i = Rpi::Pin::first() ;
  do {
    if (0 == (pins & (1u << i.value())))
      continue ;
    static const char m[] = { 'i','o','5','4','0','1','2','3' } ;
    auto mode = gpio.getMode(i) ;
    std::cout << m[Rpi::Gpio::ModeN(mode).n()] << ' ' ;
  } while (i.next()) ;
  std::cout << "mode\n" 
	    << mkstr(pins,gpio.getLevels()) << "level\n" 
	    << mkstr(pins,gpio.getEvents()) << "event\n" ;
  std::cout << std::flush ;
}

static void statusEvents(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
  auto pins = 0xffffffffu ;
  if (!argL->empty())
    pins = getPins(argL) ;
  argL->finalize() ;
  Rpi::Gpio gpio(rpi) ;
  std::cout << mkhdr(pins) << '\n'
	    << mksep(pins) << '\n' 
	    << mkstr(pins,gpio.enable(0,Rpi::Gpio::Event::     Rise,true)) << "rise\n"
	    << mkstr(pins,gpio.enable(0,Rpi::Gpio::Event::     Fall,true)) << "fall\n"
	    << mkstr(pins,gpio.enable(0,Rpi::Gpio::Event::     High,true)) << "high\n"
	    << mkstr(pins,gpio.enable(0,Rpi::Gpio::Event::      Low,true)) << "low\n"
	    << mkstr(pins,gpio.enable(0,Rpi::Gpio::Event::AsyncRise,true)) << "async-rise\n"
	    << mkstr(pins,gpio.enable(0,Rpi::Gpio::Event::AsyncFall,true)) << "async-fall\n" ;
  std::cout << std::flush ;
}

static void statusFunctions(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
  auto verbose = argL->pop_if("-v") ;
  auto pins = 0xffffffffu ;
  if (!argL->empty())
    pins = getPins(argL) ;
  argL->finalize() ;
  Rpi::Gpio gpio(rpi) ;
  auto pin = Rpi::Pin::first() ;
  do {
    if (0 == (pins & (1u << pin.value())))
      continue ;
    auto mode = gpio.getMode(pin) ;
    static char const m[] = { 'i','o','5','4','0','1','2','3' } ;
    std::cout << std::setw(2) << pin.value() << ' ' 
	      << m[Rpi::Gpio::ModeN(mode).n()] ;
    if (mode == Rpi::Gpio::Mode::In) {
      std::cout << " (Input)" ;
    }
    else if (mode == Rpi::Gpio::Mode::Out) {
      std::cout << " (Output)" ;
    }
    else {
      for (auto const &r : Rpi::Function::records()) {
	if ((r.pin.value() == pin.value()) && (mode == r.mode))
	  std::cout << " (" << Rpi::Function::name(r.type) << ')' ;
      }
    }
    if (verbose) {
      for (auto const &r : Rpi::Function::records()) 
	if (r.pin.value() == pin.value())
	  std::cout << ' ' << m[Rpi::Gpio::ModeN(r.mode).n()] << ':' << Rpi::Function::name(r.type) ;
    }
    std::cout << '\n' ;
  }
  while (pin.next()) ;
}

static void statusInvoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
  if (!argL->empty() && argL->peek() == "help") {
    std::cout << "arguments: [-e|[-f -v]] [PINS]\n"
	      << '\n'
	      << "Displays for all (default) or for selected pins:\n"
	      << "* mode  : i,o,0..5 (see mode's help)\n"
	      << "* level : the input level (0:low, 1:high)\n"
	      << '\n'
	      << "-e: include enabled and detected events\n"
	      << std::flush ;
    return ;
  }
  auto n = argL->pop_if({"-e","-f"}) ;
  if (n) {
    if      ((*n) == 0)    statusEvents(rpi,argL) ;
    else if ((*n) == 1) statusFunctions(rpi,argL) ;
    else assert(false) ;
  }
  else statusDefault(rpi,argL) ;
}

void Console::Gpio::invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
  if (argL->empty() || argL->peek() == "help") { 
    std::cout << "arguments: MODE [help]\n"
	      << '\n'
	      << "MODE : enable  # enable event detection\n"
	      << "     | mode    # set mode\n"
	      << "     | output  # set level\n"
	      << "     | pull    # switch resistor\n"
	      << "     | reset   # reset detected events\n"
	      << "     | status  # display status\n"
	      << '\n'
	      << "Arguments may require a PINS value:\n"
	      << '\n'
	      << "PINS : NO             # a single pin number\n"
	      << "     | -l NO[,NO]...  # a set of pin numbers as list\n"
	      << "     | -m MASK        # a set of pin number as bit mask\n"
	      << "     | all            # all pins\n"
	      << std::flush ;
    return ;
  }

  std::string arg = argL->pop() ;

  if      (arg == "enable") enableInvoke(rpi,argL) ; 
  else if (arg ==   "mode")   modeInvoke(rpi,argL) ;
  else if (arg == "output") outputInvoke(rpi,argL) ;
  else if (arg ==   "pull")   pullInvoke(rpi,argL) ;
  else if (arg ==  "reset")  resetInvoke(rpi,argL) ;
  else if (arg == "status") statusInvoke(rpi,argL) ;
  
  else throw std::runtime_error("not supported option:<"+arg+'>') ;
}
