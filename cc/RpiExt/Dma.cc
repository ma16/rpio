// BSD 2-Clause License, see github.com/ma16/rpio

#include "Dma.h"
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
    // [todo] this function should not access DMA control blocks directly
    //   but use some Rpi::Dma::... methods for that
  
    if (i >= mem_->nbytes() / 32)
	throw Error("not enough control-blocks allocated") ;
    if (i > 0)
    {
	// let previous block link here
	auto p = mem_->as<uint32_t volatile(*)[8]>() + (i-1) ;
	auto self = mem_->phys(32*i).first ;
	(*p)[5] = self.value() ;
    }
    auto p = mem_->as<uint32_t volatile(*)[8]>() + i ;
    (*p)[0] = ti.value() ;
    (*p)[1] = src.value() ;
    (*p)[2] = dst.value() ;
    (*p)[3] = nbytes ;
    (*p)[4] = stride ;
    (*p)[5] = 0 ;
    (*p)[6] = 0 ; // Reserved - set to zero.
    (*p)[7] = 0 ; // Reserved - set to zero.
    ++i ;
}

std::string RpiExt::Dma::Control::toStr(Rpi::Bus::Memory *cb)
{
    std::ostringstream os ;
    os << "    ADDR       TI      SRC      DST     SIZE   STRIDE     NEXT\n" ;
    os << std::hex ;
    size_t i = 0 ;
    while (true)
    {
	if ((i+1) * 32 > cb->nbytes())
	    break ;
	auto p = cb->as<uint32_t volatile(*)[8]>() + i ;
	os << std::setw(8) << cb->phys(32*i).first.value()
	   << std::setw(9) << (*p)[0]
	   << std::setw(9) << (*p)[1]
	   << std::setw(9) << (*p)[2]
	   << std::setw(9) << (*p)[3]
	   << std::setw(9) << (*p)[4]
	   << std::setw(9) << (*p)[5] << '\n' ;
	if (p[5] == 0)
	    break ;
	++i ;
    }
    return os.str() ;
}

