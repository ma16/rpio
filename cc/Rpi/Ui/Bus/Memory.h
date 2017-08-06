// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef HEADER_Rpi_Ui_Bus_Memory_h
#define HEADER_Rpi_Ui_Bus_Memory_h

#include <Rpi/Peripheral.h>
#include <Rpi/Bus/Memory.h>
#include <Ui/ArgL.h>

namespace Rpi { namespace Ui { namespace Bus {

namespace Memory
{
    using Memory = Rpi::Bus::Memory ;
  
    Memory::Factory::shared_ptr getFactory(Rpi::Peripheral *rpi,::Ui::ArgL *argL) ;

    Memory::Factory::shared_ptr getFactory(Rpi::Peripheral *rpi,::Ui::ArgL *argL,Memory::Factory::shared_ptr def) ;

    Memory::shared_ptr getMemory(Rpi::Peripheral *rpi,::Ui::ArgL *argL) ;

    Memory::shared_ptr read(std::string const &fname,Memory::Factory *mem) ;
  
} } } }

#endif // HEADER_Rpi_Bus_Memory_h
