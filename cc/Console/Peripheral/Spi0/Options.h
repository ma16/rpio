// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef Console_Spi0_Options_h
#define Console_Spi0_Options_h

#include "Mosi.h"

#include <Ui/ArgL.h>
#include <Rpi/Dma/Ctrl.h>

namespace Console { namespace Spi0 {

struct Options
{
    static Options    parse(Ui::ArgL *argL) ;
    static bool diagnostics(Ui::ArgL *argL) ;

    Console::Spi0::Mosi::Sequence  mosiV ;
    unsigned                 nrecords ;
    Rpi::Dma::Ctrl::Index         cno ;
    uint16_t                    flags ;
    bool                         adcs ;
    bool                         loop ;
    std::unique_ptr<std::ostream>  os ;
    unsigned                 brecords ;
    size_t                    qbuffer ;
    float                    progress ;
    float                       sleep ;

private:
  
    Options(Console::Spi0::Mosi::Sequence   mosiV,
	    unsigned                  nrecords,
	    Rpi::Dma::Ctrl::Index          cno,
	    uint16_t                     flags,
	    bool                          adcs,
	    bool                          loop,
	    std::unique_ptr<std::ostream> &&os,
	    unsigned                  brecords,
	    size_t                     qbuffer,
	    float                     progress,
	    float                        sleep)
	:
	mosiV      (mosiV),
	nrecords(nrecords),
	cno          (cno),
	flags      (flags),
	adcs        (adcs),
	loop        (loop),
	os (std::move(os)),
	brecords(brecords),
	qbuffer  (qbuffer),
	progress(progress),
	sleep      (sleep)
	{ }
} ;

} }

#endif // Console_Spi0_Options_h
