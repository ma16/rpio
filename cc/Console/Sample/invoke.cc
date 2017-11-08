// BSD 2-Clause License, see github.com/ma16/rpio

#include "invoke.h"
#include "../rpio.h"
#include <iostream>

void Console::Sample::invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (argL->empty() || argL->peek() == "help")
    { 
	std::cout << "arguments: event | level\n" ;
	return ;
    }

    std::map<std::string,void(*)(Rpi::Peripheral*,Ui::ArgL*)> map =
    {
	{ "event" , invoke_event },
	{ "level" , invoke_level },
    } ;
    
    argL->pop(map)(rpi,argL) ;
}
