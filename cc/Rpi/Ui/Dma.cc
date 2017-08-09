// BSD 2-Clause License, see github.com/ma16/rpio

#include "Dma.h"
#include <Ui/strto.h>
#include <iomanip> // setw

Rpi::Dma::Cs Rpi::Ui::Dma::getCs(::Ui::ArgL *argL,Rpi::Dma::Cs cs)
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
	{
	    argL->pop() ;
	    cs.panic() = ::Ui::strto(argL->peek(),Rpi::Dma::Cs::Panic::Uint()) ;
	}
	else if (arg == "--cs=priority")
	{
	    argL->pop() ;
	    cs.priority() = ::Ui::strto(argL->peek(),Rpi::Dma::Cs::Panic::Uint()) ;
	}
    
	else break ;
	argL->pop() ;
    }
    return cs ;
}

Rpi::Dma::Ti::Word Rpi::Ui::Dma::getTi(::Ui::ArgL *argL,Rpi::Dma::Ti::Word ti)
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
	{
	    argL->pop() ;
	    ti = Ti::Waits::make(::Ui::strto<Ti::Word::Unsigned>(argL->peek())) ;
	}
	else if (arg == "--ti=permap")
	{
	    argL->pop() ;
	    ti = Ti::Permap::make(::Ui::strto<Ti::Word::Unsigned>(argL->peek())) ;
	}							 
	else if (arg == "--ti=burst-length")
	{
	    argL->pop() ;
	    ti = Ti::BurstLength::make(::Ui::strto<Ti::Word::Unsigned>(argL->peek())) ;
	}
	else break ;
	argL->pop() ;
    }
    return ti ;
}
