// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Console_Dma_Lib_h_
#define _Console_Dma_Lib_h_

// --------------------------------------------------------------------
// command line arguments
// --------------------------------------------------------------------

#include <Neat/cast.h>
#include <Rpi/Bus/Memory.h>
#include <Rpi/Dma.h>
#include <Rpi/Peripheral.h>
#include <Ui/ArgL.h>

namespace Console { namespace Dma {

namespace Lib
{
    Rpi::Dma::Cs optCs(Ui::ArgL *argL,Rpi::Dma::Cs cs) ;

    Rpi::Dma::Ti::Word optTi(Ui::ArgL *argL,Rpi::Dma::Ti::Word ti) ;
} } } 

#endif // _Console_Dma_Lib_h_
