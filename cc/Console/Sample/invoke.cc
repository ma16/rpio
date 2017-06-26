// BSD 2-Clause License, see github.com/ma16/rpio

#include "../rpio.h"
#include <Rpi/Counter.h>
#include <Rpi/Gpio.h>
#include <Ui/strto.h>
#include <chrono>
#include <iomanip>
#include <iostream>

namespace Console { namespace Sample {

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
// [todo] deduplicate
    
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
// [todo] deduplicate

// --------------------------------------------------------------------

static void countInvoke(Rpi::Peripheral *rpi,Ui::ArgL *argL) 
{
  auto pin = Ui::strto(argL->pop(),Rpi::Pin()) ;
  // [future] does also work with a pin mask
  auto nsamples = Ui::strto<unsigned>(argL->pop()) ;
  auto dry = argL->pop_if("-d") ;
  argL->finalize() ;
  Rpi::Gpio gpio(rpi) ;
  auto mask = 1u << pin.value() ;
  decltype(nsamples) nevents = 0 ;
  decltype(nsamples) nsubseq = 0 ;
  auto t0 = std::chrono::steady_clock::now() ;
  if (dry) {
    for (decltype(nsamples) i=0 ; i<nsamples ; ++i) {
      auto event = mask & gpio.getEvents() ;
      if (event)
	gpio.reset(event) ;
    }
  }
  else {
    auto active = false ;
    gpio.reset(mask) ;
    for (decltype(nsamples) i=0 ; i<nsamples ; ++i) {
      auto event = mask & gpio.getEvents() ;
      if (event) {
	gpio.reset(event) ;
	++nevents ;
	if (active) ++nsubseq ;
	else active = true ;
      }
      else active = false ;
    }
  }
  auto tn = std::chrono::steady_clock::now() ; 
  auto dt = std::chrono::duration<double>(tn-t0).count() ;
  std::cout.setf(std::ios::scientific) ;
  std::cout.precision(2) ;
  std::cout <<      "f=" << static_cast<double>(nsamples)/dt << "/s " 
	    << "signal=" << static_cast<double>( nevents)/dt << "/s "
	    << "subseq=" << static_cast<double>( nsubseq)/dt << "/s\n" ;
}

// --------------------------------------------------------------------

static void dutyInvoke(Rpi::Peripheral *rpi,Ui::ArgL *argL) 
{
  auto pin = Ui::strto(argL->pop(),Rpi::Pin()) ;
  auto nsamples = Ui::strto<unsigned>(argL->pop()) ;
  auto dry = argL->pop_if("-d") ;
  argL->finalize() ;
  Rpi::Gpio gpio(rpi) ;
  auto mask = 1u << pin.value() ;
  decltype(nsamples) nchanges = 0 ;
  decltype(nsamples) nhis = 0 ;
  auto level = mask & gpio.getLevels() ;
  auto t0 = std::chrono::steady_clock::now() ;
  if (dry) {
    for (decltype(nsamples) i=0 ; i<nsamples ; ++i) {
      level ^= mask & gpio.getLevels() ; 
    }
    auto volatile x = level ; (void)x ;
  }
  else {
    for (decltype(nsamples) i=0 ; i<nsamples ; ++i) {
      auto next = mask & gpio.getLevels() ;
      if (next != level) {
	level = next ;
	++nchanges ;
      }
      if (next)
	++nhis ;
    }
  }
  auto tn = std::chrono::steady_clock::now() ; 
  auto dt = std::chrono::duration<double>(tn-t0).count() ;
  std::cout.setf(std::ios::scientific) ;
  std::cout.precision(2) ;
  std::cout <<      "f=" << static_cast<double>(nsamples)/dt << "/s " 
	    << "signal=" << static_cast<double>(nchanges)/dt << "/s "
	    <<   "duty=" << static_cast<double>(nhis) / static_cast<double>(nsamples) << std::endl ;
}

// --------------------------------------------------------------------

struct Record
{
  uint32_t t0,t1,levels ;
  Record(uint32_t t0,uint32_t t1,uint32_t levels) : t0(t0),t1(t1),levels(levels) {}
  Record() : t0(0),t1(0),levels(0) {}
} ;
    
static void watchInvoke(Rpi::Peripheral *rpi,Ui::ArgL *argL) 
{
  auto pins = getPins(argL) ;
  auto nsamples = Ui::strto<unsigned>(argL->pop()) ;
  argL->finalize() ;
  
  Rpi::Gpio gpio(rpi) ;
  Rpi::Counter counter(rpi) ;
  std::vector<Record> v(0x10000) ; v.resize(0) ;
  
  auto t0 = counter.clock() ;
  auto levels = pins & gpio.getLevels() ;
  auto t1 = counter.clock() ;
  v.push_back(Record(0,t1-t0,levels)) ;
  
  for (decltype(nsamples) i=0 ; i<nsamples ; ++i) {
    auto ti = counter.clock() ;
    auto next = pins & gpio.getLevels() ;
    if (levels != next) {
      auto tj = counter.clock() ;
      levels = next ;
      v.push_back(Record(ti-t0,tj-ti,levels)) ;
      t0 = ti ;
    }
  }

  std::cout << mkhdr(pins) << '\n'
	    << mksep(pins) << '\n' ;
  for (auto &r: v) {
    std::cout << mkstr(pins,r.levels) << r.t0 << ' ' << r.t1 << '\n' ;
  }
}
  
void invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
  if (argL->empty() || argL->peek() == "help") { 
    std::cout << "arguments: MODE PIN NSAMPLES [-d]\n"
	      << '\n'
	      << "displays the sampling rate (f) and the signal rate" 
	      << '\n'
      	      << "MODE : count  # count number of subsequent events\n"
	      << "     | duty   # determine signal's duty cycle\n"
	      << '\n'
	      << "-d : dry run to determine maximum sampling rate f\n"
	      << std::flush ;
    return ;
  }
  std::string arg = argL->pop() ;
  if      (arg == "count") { countInvoke(rpi,argL) ; } 
  else if (arg ==  "duty") {  dutyInvoke(rpi,argL) ; } 
  else if (arg == "watch") { watchInvoke(rpi,argL) ; } 
  else throw std::runtime_error("not supported option:<"+arg+'>') ;
}

} /* Sample */ } /* Console */
