// BSD 2-Clause License, see github.com/ma16/rpio

#include "Mosi.h"
#include <Ui/strto.h>

Console::Spi0::Mosi::Sequence::V::size_type Console::Spi0::Mosi::Sequence::count(V const &v)
{
    Mosi::V::size_type nbytes = 0 ;
    for (auto mosi : v)
	nbytes += (mosi.nbytes() + 3u) & ~3u ;
    return nbytes ;
}

Console::Spi0::Mosi::Sequence Console::Spi0::Mosi::Sequence::make(Ui::ArgL *argL)
{
    Mosi::Sequence::V mosiV ;
    do
    {
	Mosi::V mosi ;
	mosi.push_back(Ui::strto<uint8_t>(argL->pop())) ;
	while ((argL->peek() != "+") && (argL->peek() != "."))
	    mosi.push_back(Ui::strto<uint8_t>(argL->pop())) ;
	mosiV.push_back(std::move(mosi)) ;
    }
    while (argL->pop_if("+")) ;
    argL->pop(".") ;
    return Mosi::Sequence(std::move(mosiV)) ;
}
    
