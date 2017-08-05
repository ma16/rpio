// BSD 2-Clause License, see github.com/ma16/rpio

// --------------------------------------------------------------------
// This is a very limited implementation to manage DMA control-blocks.
// 
// It is based on blocks of Rpi::Bus::Memory. Such a block is allocated
// for each DMA-control-block. Also, such a block is required to define
// a buffer that shall be transferred to or from a peripheral.
//
// Since Rpi::Bus::Memory comes in size of pages, there will be quite
// some waste due to the low payload ratio.
// --------------------------------------------------------------------

#ifndef INCLUDE_RpiExt_Dma_Control_h
#define INCLUDE_RpiExt_Dma_Control_h

#include <deque>
#include <Rpi/Bus/Memory.h>
#include <Rpi/Dma.h>

namespace RpiExt { namespace Dma {

struct Control 
{
    struct Error : Neat::Error
    {
	Error(std::string const &s) : Neat::Error("RpiExt:Dma:Control:" + s) {}
    } ;

    using Factory = Rpi::Bus::Memory::Factory ;
	
    Control(Factory::shared_ptr factory) : factory(factory) {}
    // ...the factory is used to allocate DMA control blocks
  
    // read buffer from peripheral (register)
    void add(Rpi::Dma::Ti::Word ti,
	     Rpi::Bus::Address src, 
	     Rpi::Bus::Memory *dst) ;

    // write buffer to peripheral (register)
    void add(Rpi::Dma::Ti::Word ti,
	     Rpi::Bus::Memory *src,
	     Rpi::Bus::Address dst) ;

    // return the address of the first DMA control block
    Rpi::Bus::Address address()
    {
	if (blocks.empty())
	    throw Error("empty list of control-blocks") ;
	return blocks.front()->phys(0).first ;
    }
  
private:
  
    Factory::shared_ptr factory ;

    std::deque<Rpi::Bus::Memory::shared_ptr> blocks ;

    void add(Rpi::Dma::Ti::Word ti,
	     Rpi::Bus::Address src,
	     Rpi::Bus::Address dst,
	     uint32_t       nbytes,
	     uint32_t       stride) ;
} ;
    
} }

#endif // INCLUDE_RpiExt_Dma_Control_h
