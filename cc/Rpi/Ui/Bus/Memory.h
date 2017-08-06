// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef HEADER_Rpi_Ui_Bus_Memory_h
#define HEADER_Rpi_Ui_Bus_Memory_h

#include "Coherency.h"
#include <Rpi/Peripheral.h>
#include <Rpi/Bus/Memory.h>
#include <Ui/ArgL.h>

namespace Rpi { namespace Ui { namespace Bus {

namespace Memory
{
    using Memory = Rpi::Bus::Memory ;
  
    Memory::Allocator::shared_ptr getAllocator(Rpi::Peripheral *rpi,::Ui::ArgL *argL) ;

    static inline std::string allocatorSynopsis()
    {
	std::stringstream os ;
	os << "ALLOC : arm [COHERENCY] [-s]\n"
	   << "      | gpu [-a ALIGN] [-m MODE] [-d COHERENCY]\n"
	   << '\n'
	   << "COHERENCY:\n"
	   << Coherency::synopsis()
	   << '\n'
	   << "ALIGN = align the memory segment (U32)\n"
	   << '\n'
	   << "MODE\n"
	   << "bit:0: discardable\n"
	   << "bit:2: direct allocation\n"
	   << "bit:3: coherent allocation\n"
	   << "bit:4: fill with zeros\n"
	   << "bit:5: don't initialize\n"
	   << "bit:6: lock permenently\n"
	   << '\n'
	   << "-d: don't use /dev/vcio\n"
	    ;
	return os.str() ;
    }

    Memory::Allocator::shared_ptr getAllocator(Rpi::Peripheral *rpi,::Ui::ArgL *argL,Memory::Allocator::shared_ptr def) ;

    Memory::shared_ptr getMemory(Rpi::Peripheral *rpi,::Ui::ArgL *argL) ;

    Memory::shared_ptr read(std::string const &fname,Memory::Allocator *mem) ;
  
} } } }

#endif // HEADER_Rpi_Bus_Memory_h
