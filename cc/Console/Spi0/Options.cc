// BSD 2-Clause License, see github.com/ma16/rpio

#include "Options.h"
#include <Ui/strto.h>
#include <fstream>
#include <iostream>

bool Console::Spi0::Options::diagnostics(Ui::ArgL *argL)
{
    if (!argL->empty() && (argL->peek() != "help"))
	return false ;
    std::cout << "arguments: COMMANDS NRECORDS CHANNEL [-f FLAGS] [--adcs] [--loop] [-o FILE] [-b BRECORDS] [-q BUFFER] [-p PROGRESS] [-s SLEEP]\n"
	      << "\n"
	      << "COMMANDS : BYTES (\"+\" BYTES)* \".\"\n"
	      << "\n"
	      << "   BYTES  # A sequence of space separated values\n"
	      << " NRECORDS # The number of samples to take\n"
	      << "  CHANNEL # The DMA channel number to use\n"
	      << "    FLAGS # SPI control flags as 16-bit value\n"
	      << "   --adcs # Enable ADCS control flag\n"
	      << "   --loop # Start over when all samples were taken\n"
	      << "     FILE # file to write data to\n"
	      << " BRECORDS # number of records to collect before writing\n"
	      << "   BUFFER # Queue buffer\n"
	      << " PROGRESS # seconds for progress bar\n"
	      << "    SLEEP # seconds to sleep if there aren't enough records\n"
	      << std::flush ;
    return true ;
}
    
Console::Spi0::Options Console::Spi0::Options::parse(Ui::ArgL *argL)
{
    auto mosiV = Console::Spi0::Mosi::Sequence::make(argL) ;
    auto nrecords = Ui::strto<unsigned>(argL->pop()) ;
    auto cno = Ui::strto(argL->pop(),Rpi::Dma::Ctrl::Index()) ;
    auto flags = Ui::strto<uint16_t>(argL->option("-f","0x0")) ;
    auto adcs = argL->pop_if("--adcs") ;
    auto loop = argL->pop_if("--loop") ;
    std::unique_ptr<std::ostream> os ;
    if (argL->pop_if("-o"))
    {
	os.reset(new std::ofstream(argL->pop())) ;
	if (!os->good())
	    throw std::runtime_error("cannot open file") ;
    }
    auto brecords = Ui::strto<unsigned>(argL->option("-b","0x1000")) ;
    auto qbuffer = Ui::strto<size_t>(argL->option("-q","0x100000")) ;
    auto progress = Ui::strto<float>(argL->option("-p","1.0")) ;
    auto sleep = Ui::strto<float>(argL->option("-s","1e-3")) ;
    argL->finalize() ;
    return Options(mosiV,nrecords,cno,flags,adcs,loop,std::move(os),brecords,qbuffer,progress,sleep) ;
}    

