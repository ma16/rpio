// BSD 2-Clause License, see github.com/ma16/rpio

#include "../rpio.h"
#include "Host.h"
#include <Neat/cast.h>
#include <Ui/strto.h>
#include <chrono>
#include <deque>
#include <iostream>

namespace Console { namespace Ads1115 {

static void configInvoke(Host *host,Ui::ArgL *argL)
{
  if (argL->empty()) {
    auto word = host->readConfig() ;
    if (word) {
      std::cout << std::hex << (*word) << std::endl ;
    }
    else {
      std::cout << "error" << std::endl ;
    }
  }
  else {
    auto word = Ui::strto<uint16_t>(argL->pop()) ;
    argL->finalize() ;
    auto success = host->writeConfig(word) ;
    if (!success) 
      std::cout << "error" << std::endl ;
  }
}
    
static void resetInvoke(Host *host,Ui::ArgL *argL)
{
  argL->finalize() ;
  auto success = host->doReset() ;
  std::cout << success << std::endl ;
}
    
static void sampleInvoke(Host *host,Ui::ArgL *argL)
{
  argL->finalize() ;
  auto sample = host->readSample() ;
  if (sample) {
    std::cout << std::hex << (*sample) << std::endl ;
  }
  else {
    std::cout << "error" << std::endl ;
  }
}
    
void invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
  if (argL->empty() || argL->peek() == "help") {
    std::cout << "arguments: SCL SDA ADDR [-m] MODE\n"
	      << '\n'
	      << " SCL: Pi's pin to feed ADS's SCL pin\n"
	      << " SDA: Pi's pin to  r/w ADS's SDA pin\n"
	      << "ADDR: 0..127\n"
	      << '\n'
	      << "-m: enable monitoring to detect communication errors\n"
	      << '\n'
	      << "MODE : config       # read configuration register\n"
	      << "     | config WORD  # write configuration register\n"
	      << "     | reset        # write reset command\n"
	      << "     | sample       # read sample register\n"
	      << std::flush ;
    return ;
  }
  
  auto sclPin = Ui::strto(argL->pop(),Rpi::Pin()) ;
  auto sdaPin = Ui::strto(argL->pop(),Rpi::Pin()) ;
  auto addr = Ui::strto(argL->pop(),Host::addr_t()) ;
  auto monitor = argL->pop_if("-m") ;
  
  Host host(rpi,sclPin,sdaPin,addr,Host::Timing(),monitor) ;
  // ...[future] make timings optional arguments

  auto arg = argL->pop() ;
  if      (arg == "config") configInvoke(&host,argL) ;
  else if (arg ==  "reset")  resetInvoke(&host,argL) ;
  else if (arg == "sample") sampleInvoke(&host,argL) ;
  else throw std::runtime_error("not supported option:<"+arg+'>') ;
}

} /* Ads1115 */ } /* Console */
