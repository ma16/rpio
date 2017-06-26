// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Console_rpio_h_
#define _Console_rpio_h_

#include <Rpi/Peripheral.h>
#include <Ui/ArgL.h>

namespace Console
{
  namespace    Ads1115 { void invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL) ; }
  namespace       Bang { void invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL) ; }
  namespace      Clock { void invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL) ; }
  namespace         Cp { void invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL) ; }
  namespace        Dma { void invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL) ; }
  namespace       Gpio { void invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL) ; }
  namespace       Intr { void invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL) ; }
  namespace    Max7219 { void invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL) ; }
  namespace       Mbox { void invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL) ; }
  namespace    Mcp3008 { void invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL) ; }
  namespace       Poke { void invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL) ; }
  namespace        Pwm { void invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL) ; }
  namespace        Shm { void invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL) ; }
  namespace     Sample { void invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL) ; }
  namespace       Spi0 { void invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL) ; }
  namespace       Spi1 { void invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL) ; }
  namespace   SpiSlave { void invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL) ; }
  namespace Throughput { void invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL) ; }
  namespace    Ws2812b { void invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL) ; }
}

#endif // _Console_rpio_h_
