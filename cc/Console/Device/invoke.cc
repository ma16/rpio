// BSD 2-Clause License, see github.com/ma16/rpio

#include "../rpio.h"
#include "invoke.h"

void Console::Device::invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (argL->empty() || argL->peek() == "help")
    {
	std::cout << "arguments: DEVICE [help]\n" 
		  << '\n'
		  << "DEVICE : ads1115  # analog-to-digital converter\n"
		  << "       | ds18x20  # temperature sensor (DS18B20)\n"
		  << "       | max7219  # LED-dot-matrix controller\n"
		  << "       | mcp3008  # analog-to-digital converter\n"
		  << "       | ws2812b  # LED-integrated controller\n" ;
	return ;
    }
      
    auto arg = argL->pop() ;
    if (false) ;
      
    else if (arg == "ads1115") Device::Ads1115::invoke(rpi,argL) ;
    else if (arg == "ds18x20") Device::Ds18x20::invoke(rpi,argL) ;
    else if (arg == "mcp3008") Device::Mcp3008::invoke(rpi,argL) ;
    else if (arg == "max7219") Device::Max7219::invoke(rpi,argL) ;
    else if (arg == "ws2812b") Device::Ws2812b::invoke(rpi,argL) ;
    
    else throw std::runtime_error("not supported option:<"+arg+'>') ;
}
