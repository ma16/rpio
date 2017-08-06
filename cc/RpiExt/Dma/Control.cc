// BSD 2-Clause License, see github.com/ma16/rpio

#include "Control.h"
#include <iomanip>

void RpiExt::Dma::Control::add(Rpi::Dma::Ti::Word ti,
			       Rpi::Bus::Address src,
			       Rpi::Bus::Memory *dst)
{
    auto section = dst->phys(0) ;
    auto dst_addr = section.first ;
    auto nbytes = section.second ;
    this->add(ti,src,dst_addr,nbytes,0) ;
}

void RpiExt::Dma::Control::add(Rpi::Dma::Ti::Word ti,
			       Rpi::Bus::Memory *src,
			       Rpi::Bus::Address dst)
{
    auto section = src->phys(0) ;
    auto src_addr = section.first ;
    auto nbytes = section.second ;
    this->add(ti,src_addr,dst,nbytes,0) ;
}

void RpiExt::Dma::Control::add(Rpi::Dma::Ti::Word ti,
			       Rpi::Bus::Address src,
			       Rpi::Bus::Address dst,
			       uint32_t       nbytes,
			       uint32_t       stride)
{
    auto block = this->allocator->allocate(32) ; 

    auto p = block->as<uint32_t volatile(*)[8]>() ;
    (*p)[0] = ti.value() ;
    (*p)[1] = src.value() ;
    (*p)[2] = dst.value() ;
    (*p)[3] = nbytes ;
    (*p)[4] = stride ;
    (*p)[5] = 0 ; // next block
    (*p)[6] = 0 ; // Reserved - set to zero.
    (*p)[7] = 0 ; // Reserved - set to zero.
    
    // link previous block (if any)
    if (!this->blocks.empty())
    {
	auto p = this->blocks.back()->as<uint32_t volatile(*)[8]>() ;
	(*p)[5] = block->phys(0).first.value() ;
    }

    blocks.push_back(block) ;
    
    // [todo] access DMA control blocks by Rpi::Dma
}
