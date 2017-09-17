// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef INCLUDE_Console_rpio_h
#define INCLUDE_Console_rpio_h

#include <Rpi/Peripheral.h>
#include <Ui/ArgL.h>

namespace Console
{
    namespace      Clock { void invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL) ; }
    namespace     Defect { void invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL) ; }
    namespace     Device { void invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL) ; }
    namespace         Cm { void invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL) ; }
    namespace        Dma { void invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL) ; }
    namespace       Gpio { void invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL) ; }
    namespace       Intr { void invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL) ; }
    namespace       Mbox { void invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL) ; }
    namespace       Poke { void invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL) ; }
    namespace        Pwm { void invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL) ; }
    namespace        Shm { void invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL) ; }
    namespace     Sample { void invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL) ; }
    namespace       Spi0 { void invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL) ; }
    namespace       Spi1 { void invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL) ; }
    namespace   SpiSlave { void invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL) ; }
    namespace Throughput { void invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL) ; }
}

#endif // INCLUDE_Console_rpio_h
