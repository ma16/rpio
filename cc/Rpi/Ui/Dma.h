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
    Rpi::Dma::Cs optCs(::Ui::ArgL *argL,Rpi::Dma::Cs cs) ;

    Rpi::Dma::Ti::Word optTi(::Ui::ArgL *argL,Rpi::Dma::Ti::Word ti) ;
    
} } } 

#endif // HEADER_Rpi_Ui_Dma_h
