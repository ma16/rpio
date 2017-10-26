// BSD 2-Clause License, see github.com/ma16/rpio

#include "../rpio.h"
#include "invoke.h"
#include <iostream>

void Console::Peripheral::invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (argL->empty() || argL->peek() == "help")
    {
	std::cout
	    << "arguments: MODE [help]\n"
	    << "MODE | arm-timer   # free running ARM counter\n"
	    << "     | cm          # clock manager\n"
	    << "     | dma         # direct memory access\n"
	    << "     | gpio        # general purpose input/output\n"
	    << "     | intr        # interrupt control\n"
	    << "     | mbox        # mailbox control\n"
	    << "     | pwm         # pulse width modulation\n"
	    << "     | spi0        # serial peripheral interface:0\n"
	    << "     | spi1        # serial peripheral interface:1\n"
	    << "     | spi-slave   # serial peripheral interface:slave\n"
	    ;
	return ;
    }

    using namespace Console::Peripheral ;
    std::map<std::string,void(*)(Rpi::Peripheral*,Ui::ArgL*)> map =
    {
	{ "arm-timer",ArmTimer::invoke },
	{ "cm",Cm::invoke },
	{ "dma",Dma::invoke },
	{ "gpio",Gpio::invoke },
	{ "intr",Intr::invoke },
	{ "mbox",Mbox::invoke },
	{ "pwm",Pwm::invoke },
	{ "spi0",Spi0::invoke },
	{ "spi1",Spi1::invoke },
	{ "spi-slave",SpiSlave::invoke },
    } ;
    argL->pop(map)(rpi,argL) ;
}
