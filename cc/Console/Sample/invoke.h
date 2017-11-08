// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef INCLUDE_Console_Sample_invoke_h
#define INCLUDE_Console_Sample_invoke_h

#include <Rpi/Peripheral.h>
#include <Ui/ArgL.h>

namespace Console { namespace Sample
{

void invoke_event(Rpi::Peripheral *rpi,Ui::ArgL *argL) ;
void invoke_level(Rpi::Peripheral *rpi,Ui::ArgL *argL) ;

} }

#endif // INCLUDE_Console_Sample_invoke_h
