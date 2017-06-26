// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Console_Memory_Lib_h_
#define _Console_Memory_Lib_h_

// --------------------------------------------------------------------
// command line arguments
// --------------------------------------------------------------------

#include <Rpi/Bus/Memory.h>
#include <Rpi/Peripheral.h>
#include <Ui/ArgL.h>

namespace Console { namespace Memory { namespace Lib
{
  using Memory = Rpi::Bus::Memory ;
  
  Rpi::Bus::Coherency getCo(Ui::ArgL *argL) ;
  
  Memory::Factory::shared_ptr defaultFactory() ;

  Memory::Factory::shared_ptr getFactory(Rpi::Peripheral *rpi,Ui::ArgL *argL) ;

  Memory::Factory::shared_ptr getFactory(Rpi::Peripheral *rpi,Ui::ArgL *argL,Memory::Factory::shared_ptr def) ;
  
  Memory::shared_ptr getMemory(Rpi::Peripheral *rpi,Ui::ArgL *argL) ;

  Memory::shared_ptr read(std::string const &fname,Memory::Factory *mem) ;
  
} } } 

#endif // _Console_Memory_Lib_h_
