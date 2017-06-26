// BSD 2-Clause License, see github.com/ma16/rpio

#include "../rpio.h"
#include "Host.h"
#include <Ui/strto.h>
#include <iostream>

void Console::Bang::invoke(Rpi::Peripheral*,Ui::ArgL *argL)
{
  if (argL->empty() || argL->peek() == "help") {
    std::cout << "only as library, for now\n"
	      << std::flush ;
    // [todo] read data to send from file
    return ;
  }
  std::string arg = argL->pop() ;
  if (false) ;
  
  else throw std::runtime_error("not supported option:<"+arg+'>') ;
}
