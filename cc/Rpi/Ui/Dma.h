// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef HEADER_Rpi_Ui_Dma_h
#define HEADER_Rpi_Ui_Dma_h

// --------------------------------------------------------------------
// command line arguments
// --------------------------------------------------------------------

#include <Rpi/Dma.h>
#include <Ui/ArgL.h>

namespace Rpi { namespace Ui {

namespace Dma
{
    Rpi::Dma::Cs getCs(::Ui::ArgL *argL,Rpi::Dma::Cs cs) ;

    static inline std::string csSynopsis()
    {
	return 
	    "--cs[+-]disdebug\n"
	    "--cs=panic-priority 0..15\n"
	    "--cs=priority 0..15\n" 
	    "--cs[+-]wait-for-outstanding-writes\n"
	    ;
    }

    Rpi::Dma::Ti::Word getTi(::Ui::ArgL *argL,Rpi::Dma::Ti::Word ti) ;

    static inline std::string tiSynopsis()
    {
	return 
	    "--ti=burst-length 0..16\n"
	    "--ti[+-]dest-dreq\n"
	    "--ti[+-]dest-inc\n"
	    "--ti[+-]dest-ignore\n"
	    "--ti[+-]dest-width\n"
	    "--ti[+-]inten\n"
	    "--ti[+-]no-wide-bursts\n"
	    "--ti=permap 0..31\n"
	    "--ti[+-]src-dreq\n"
	    "--ti[+-]src-inc\n"
	    "--ti[+-]src-ignore\n"
	    "--ti[+-]src-width\n"
	    "--ti[+-]tdmode\n"
	    "--ti[+-]wait-resp\n"
	    "--ti=waits 0..31\n"
	    ;
    }
    
} } } 

#endif // HEADER_Rpi_Ui_Dma_h
