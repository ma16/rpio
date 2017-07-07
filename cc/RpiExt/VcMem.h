// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef INCLUDE_RpiExt_VcMem_h
#define INCLUDE_RpiExt_VcMem_h

#include <Rpi/Bus/Memory.h>
#include <Rpi/Peripheral.h>
#include <Ui/ArgL.h>

namespace RpiExt { namespace VcMem
{
  using Memory = Rpi::Bus::Memory ;
  
  Rpi::Bus::Coherency getCo(Ui::ArgL *argL) ;
  
  Memory::Factory::shared_ptr defaultFactory() ;

  Memory::Factory::shared_ptr getFactory(Rpi::Peripheral *rpi,Ui::ArgL *argL) ;

  Memory::Factory::shared_ptr getFactory(Rpi::Peripheral *rpi,Ui::ArgL *argL,Memory::Factory::shared_ptr def) ;
  
  Memory::shared_ptr getMemory(Rpi::Peripheral *rpi,Ui::ArgL *argL) ;

  Memory::shared_ptr read(std::string const &fname,Memory::Factory *mem) ;
  
} } 

#endif // INCLUDE_RpiExt_VcMem_h
