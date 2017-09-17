// BSD 2-Clause License, see github.com/ma16/rpio

#include "../rpio.h"
#include "invoke.h"

void Console::Defect::invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    std::map<std::string,void(*)(Rpi::Peripheral*,Ui::ArgL*)> map =
    {
	{ "d1" , D1::invoke },
    } ;
    argL->pop(map)(rpi,argL) ;
}
