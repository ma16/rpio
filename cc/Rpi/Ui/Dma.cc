// BSD 2-Clause License, see github.com/ma16/rpio

#include "Dma.h"
#include <Ui/strto.h>
#include <iomanip> // setw

Rpi::Dma::Cs Rpi::Ui::Dma::optCs(::Ui::ArgL *argL,Rpi::Dma::Cs cs)
{
    while (!argL->empty())
    {
	auto arg = argL->peek() ;
	if (false) ;
    
	else if (arg == "--cs-disdebug") cs.disdebug() = false ;
	else if (arg == "--cs+disdebug") cs.disdebug() =  true ;
    
	else if (arg == "--cs-wait-for-outstanding-writes") cs.wait() = false ;
	else if (arg == "--cs+wait-for-outstanding-writes") cs.wait() =  true ;
    
	else if (arg == "--cs=panic-priority")
	    cs.panic() = ::Ui::strto(argL->pop(),Rpi::Dma::Cs::Panic::Uint()) ;
	else if (arg == "--cs=priority")
	    cs.priority() = ::Ui::strto(argL->pop(),Rpi::Dma::Cs::Panic::Uint()) ;
    
	else break ;
	argL->pop() ;
    }
    return cs ;
}

Rpi::Dma::Ti::Word Rpi::Ui::Dma::optTi(::Ui::ArgL *argL,Rpi::Dma::Ti::Word ti)
{
    using namespace Rpi::Dma ;
  
    while (!argL->empty())
    {
	auto arg = argL->peek() ;
	if (false) ;
    
	else if (arg == "--ti-no-wide-bursts") ti = Ti::NoWideBursts::make<0>() ;
	else if (arg == "--ti+no-wide-bursts") ti = Ti::NoWideBursts::make<1>() ;
    
	else if (arg == "--ti-src-ignore") ti = Ti::SrcIgnore::make<0>() ;
	else if (arg == "--ti+src-ignore") ti = Ti::SrcIgnore::make<1>() ;
    
	else if (arg == "--ti-src-dreq") ti = Ti::SrcDreq::make<0>() ;
	else if (arg == "--ti+src-dreq") ti = Ti::SrcDreq::make<1>() ;

	else if (arg == "--ti-src-width") ti = Ti::SrcWidth::make<0>() ;
	else if (arg == "--ti+src-width") ti = Ti::SrcWidth::make<1>() ;

	else if (arg == "--ti-src-inc") ti = Ti::SrcInc::make<0>() ;
	else if (arg == "--ti+src-inc") ti = Ti::SrcInc::make<1>() ;

	else if (arg == "--ti-dest-ignore") ti = Ti::DestIgnore::make<0>() ;
	else if (arg == "--ti+dest-ignore") ti = Ti::DestIgnore::make<1>() ;

	else if (arg == "--ti-dest-dreq") ti = Ti::DestDreq::make<0>() ;
	else if (arg == "--ti+dest-dreq") ti = Ti::DestDreq::make<1>() ;

	else if (arg == "--ti-dest-width") ti = Ti::DestWidth::make<0>() ;
	else if (arg == "--ti+dest-width") ti = Ti::DestWidth::make<1>() ;

	else if (arg == "--ti-dest-inc") ti = Ti::DestInc::make<0>() ;
	else if (arg == "--ti+dest-inc") ti = Ti::DestInc::make<1>() ;

	else if (arg == "--ti-wait-resp") ti = Ti::WaitResp::make<0>() ;
	else if (arg == "--ti+wait-resp") ti = Ti::WaitResp::make<1>() ;

	else if (arg == "--ti-tdmode") ti = Ti::Tdmode::make<0>() ;
	else if (arg == "--ti+tdmode") ti = Ti::Tdmode::make<1>() ;

	else if (arg == "--ti-inten") ti = Ti::Inten::make<0>() ;
	else if (arg == "--ti+inten") ti = Ti::Inten::make<1>() ;
    
	else if (arg == "--ti=waits")
	    ti = Ti::Waits::make(::Ui::strto<Ti::Word::Unsigned>(argL->pop())) ;
								
	else if (arg == "--ti=permap")
	    ti = Ti::Permap::make(::Ui::strto<Ti::Word::Unsigned>(argL->pop())) ;
								 
	else if (arg == "--ti=burst-length")
	    ti = Ti::BurstLength::make(::Ui::strto<Ti::Word::Unsigned>(argL->pop())) ;

	else break ;
	argL->pop() ;
    }
    return ti ;
}
