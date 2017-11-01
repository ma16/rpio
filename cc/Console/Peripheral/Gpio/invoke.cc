// BSD 2-Clause License, see github.com/ma16/rpio

#include "../invoke.h"
#include <Rpi/Gpio/Function.h>
#include <Rpi/Gpio/Pull.h>
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
    if (argL->empty() || argL->peek() == "help")
    {
	std::cout
	    << "arguments: PINS TYPE [off]\n"
	    << '\n'
	    << "TYPE : rise\n"
	    << "     | fall\n"
	    << "     | high\n"
	    << "     | low\n"
	    << "     | async-rise\n"
	    << "     | async-fall\n"
	    << "     | any\n"
	    ;
	return ;
    }
    auto pins = getPins(argL) ;
    auto mode = argL->pop(
    {"rise","fall","high","low","async-rise","async-fall","any"}) ;
    auto on = !argL->pop_if("off") ;
    argL->finalize() ;
    auto gpio = rpi->page<Rpi::Register::Gpio::PageNo>() ;
    
    auto rise      = gpio.at<Rpi::Register::Gpio::Event::     Rise0>().value() ;
    auto fall      = gpio.at<Rpi::Register::Gpio::Event::     Fall0>().value() ;
    auto high      = gpio.at<Rpi::Register::Gpio::Event::     High0>().value() ;
    auto low       = gpio.at<Rpi::Register::Gpio::Event::      Low0>().value() ;
    auto asyncRise = gpio.at<Rpi::Register::Gpio::Event::AsyncRise0>().value() ;
    auto asyncFall = gpio.at<Rpi::Register::Gpio::Event::AsyncFall0>().value() ;

    auto set = [pins,on](uint32_t volatile *p)
    {
	if (on) (*p) |=  pins ;
	else    (*p) &= ~pins ;
    } ;
    
    switch (mode)
    {
    case 0: set(rise) ; break ;
    case 1: set(fall) ; break ;
    case 2: set(high) ; break ;
    case 3: set(low) ; break ;
    case 4: set(asyncRise) ; break ;
    case 5: set(asyncFall) ; break ;
    case 6:
	set(rise) ;
	set(fall) ;
	set(high) ;
	set(low) ;
	set(asyncRise) ; 
	set(asyncFall) ; 
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
  auto mode = Rpi::Gpio::Function::TypeEnum::make(i).e() ;
  argL->finalize() ;
  auto gpio = rpi->page<Rpi::Register::Gpio::PageNo>() ;
  auto pin = Rpi::Pin::first() ; 
  do
  {
      if (0 != (pins & (1u << pin.value())))
	  Rpi::Gpio::Function::set(gpio,pin,mode) ;
  }
  while (pin.next()) ;
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
  argL->finalize() ;
  auto p = (i == 0)
      ? rpi->at<Rpi::Register::Gpio::Output::Raise0>().value() 
      : rpi->at<Rpi::Register::Gpio::Output::Clear0>().value() ;
  (*p) = pins ;
}

static void pullInvoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (argL->empty() || argL->peek() == "help")
    {
	std::cout << "arguments: PINS MODE\n"
		  << '\n'
		  << "MODE : down  # apply pull-down resistor\n"
		  << "     | off   # set tri-state (high-impedance)\n"
		  << "     | up    # apply pull-up resistor\n"
	    ;
	return ;
    }
    auto pins = getPins(argL) ;
    auto i = argL->pop({"off","down","up"}) ;
    argL->finalize() ;
    auto gpio = rpi->page<Rpi::Register::Gpio::PageNo>() ;
    switch (i)
    {
    case 0: Rpi::Gpio::Pull::set(gpio,pins,Rpi::Gpio::Pull::Mode:: Off) ; break ;
    case 1: Rpi::Gpio::Pull::set(gpio,pins,Rpi::Gpio::Pull::Mode::Down) ; break ;
    case 2: Rpi::Gpio::Pull::set(gpio,pins,Rpi::Gpio::Pull::Mode::  Up) ; break ;
    }
}
    
static void resetInvoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (!argL->empty() && argL->peek() == "help")
    {
	std::cout << "argument: [PINS]\n" ;
	return ;
    }
    uint32_t pins = 0xffffffff ;
    if (!argL->empty())
	pins = getPins(argL) ;
    argL->finalize() ;
    auto status = rpi->at<Rpi::Register::Gpio::Event::Status0>().value() ;
    (*status) = pins ;
}

static void statusDefault(Rpi::Peripheral *rpi,uint32_t pins)
{
    std::cout << mkhdr(pins) << '\n'
	      << mksep(pins) << '\n' ;
    auto gpio = rpi->page<Rpi::Register::Gpio::PageNo>() ;
    auto i = Rpi::Pin::first() ;
    do
    {
	if (0 == (pins & (1u << i.value())))
	    continue ;
	static const char m[] = { 'i','o','5','4','0','1','2','3' } ;
	auto mode = Rpi::Gpio::Function::get(gpio,i) ;
	std::cout << m[Rpi::Gpio::Function::TypeEnum(mode).n()] << ' ' ;
    }
    while (i.next()) ;
    auto  input = gpio.at<Rpi::Register::Gpio::Input::  Bank0>().value() ;
    auto status = gpio.at<Rpi::Register::Gpio::Event::Status0>().value() ;
    std::cout << "mode\n" 
	      << mkstr(pins, (*input)) << "level\n" 
	      << mkstr(pins,(*status)) << "event\n" ;
}

static void statusEvents(Rpi::Peripheral *rpi,uint32_t pins)
{
    auto gpio = rpi->page<Rpi::Register::Gpio::PageNo>() ;
    namespace Register = Rpi::Register::Gpio::Event ;
    auto rise      = gpio.at<Register::     Rise0>().value() ;
    auto fall      = gpio.at<Register::     Fall0>().value() ;
    auto high      = gpio.at<Register::     High0>().value() ;
    auto low       = gpio.at<Register::      Low0>().value() ;
    auto asyncRise = gpio.at<Register::AsyncRise0>().value() ;
    auto asyncFall = gpio.at<Register::AsyncFall0>().value() ;
    std::cout
	<< mkhdr(pins) << '\n'
	<< mksep(pins) << '\n' 
	<< mkstr(pins,     *rise) << "rise\n"
	<< mkstr(pins,     *fall) << "fall\n"
	<< mkstr(pins,     *high) << "high\n"
	<< mkstr(pins,      *low) << "low\n"
	<< mkstr(pins,*asyncRise) << "async-rise\n"
	<< mkstr(pins,*asyncFall) << "async-fall\n"
	;
}

static void statusFunctions(Rpi::Peripheral *rpi,uint32_t pins,bool verbose)
{
    auto gpio = rpi->page<Rpi::Register::Gpio::PageNo>() ;
    auto pin = Rpi::Pin::first() ;
    do
    {
	if (0 == (pins & (1u << pin.value())))
	    continue ;
	auto type = Rpi::Gpio::Function::get(gpio,pin) ;
	static char const m[] = { 'i','o','5','4','0','1','2','3' } ;
	std::cout << std::setw(2) << pin.value() << ' ' 
		  << m[Rpi::Gpio::Function::TypeEnum(type).n()] ;
	if (type == Rpi::Gpio::Function::Type::In)
	{
	    std::cout << " (Input)" ;
	}
	else if (type == Rpi::Gpio::Function::Type::Out)
	{
	    std::cout << " (Output)" ;
	}
	else
	{
	    for (auto const &r : Rpi::Gpio::Function::records())
	    {
		if ((r.pin.value() == pin.value()) && (type == r.type))
		    std::cout << " (" << Rpi::Gpio::Function::name(r.device) << ')' ;
	    }
	}
	if (verbose)
	{
	    for (auto const &r : Rpi::Gpio::Function::records()) 
		if (r.pin.value() == pin.value())
		    std::cout << ' ' << m[Rpi::Gpio::Function::TypeEnum(r.type).n()]
			      << ':' << Rpi::Gpio::Function::name(r.device) ;
	}
	std::cout << '\n' ;
    }
    while (pin.next()) ;
}

static void statusInvoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (!argL->empty() && argL->peek() == "help")
    {
	std::cout
	    << "arguments: [-e | -f | -v] [PINS]\n"
	    << '\n'
	    << "Displays for all (default) or for selected pins:\n"
	    << "* mode  : i,o,0..5 (see mode's help)\n"
	    << "* level : the input level (0:low, 1:high)\n"
	    << '\n'
	    << "-e: include enabled and detected events\n"
	    << "-f: presently selected GPIO functions by name\n"
	    << "-v: -f plus available GPIO functions\n"
	    ;
	return ;
    }
    auto n = argL->pop_if({"-e","-f","-v"}) ;
    auto pins = 0xffffffffu ;
    if (!argL->empty())
	pins = getPins(argL) ;
    argL->finalize() ;
    if (n)
    {
	switch (*n)
	{
	case 0: statusEvents   (rpi,pins      ) ; break ;
	case 1: statusFunctions(rpi,pins,false) ; break ;
	case 2: statusFunctions(rpi,pins, true) ; break ;
	}
    }
    else statusDefault(rpi,pins) ;
}

void Console::Peripheral::Gpio::
invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
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
