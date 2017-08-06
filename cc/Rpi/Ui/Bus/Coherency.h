// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef HEADER_Rpi_Ui_Bus_Coherency_h
#define HEADER_Rpi_Ui_Bus_Coherency_h

#include <Rpi/Bus/Coherency.h>
#include <Ui/ArgL.h>

namespace Rpi { namespace Ui { namespace Bus {

namespace Coherency
{
    Rpi::Bus::Coherency get(::Ui::ArgL *argL) ;

    static inline std::string synopsis()
    {
	return "[--co-[0|4|8|c]] (default --co-4)\n" ;
    }
    
} } } }

#endif // HEADER_Rpi_Bus_Coherency_h
