// BSD 2-Clause License, see github.com/ma16/rpio

#include "Dma.h"
#include <iomanip>

void RpiExt::Dma::Control::write(Rpi::Dma::Ti::Word ti,
				 Rpi::Bus::Address src,
				 Rpi::Bus::Address dst,
				 uint32_t nbytes,
				 uint32_t stride)
{
    // [todo] this function should not access DMA control blocks directly
    //   but use some Rpi::Dma::... methods for that
  
    assert(i < mem_->nbytes() / 32) ;
    if (i > 0) {
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

void RpiExt::Dma::Control::repeat(size_t ofs)
{
  assert(ofs < i) ;
  auto p = mem_->as<uint32_t volatile(*)[8]>() + (i-1) ;
  auto to = mem_->phys(32*ofs).first ;
  (*p)[5] = to.value() ;
}

std::string RpiExt::Dma::Control::toStr(Rpi::Bus::Memory *cb)
{
  std::ostringstream os ;
  os << "    ADDR       TI      SRC      DST     SIZE   STRIDE     NEXT\n" ;
  os << std::hex ;
  size_t i = 0 ;
  while (true) {
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

void RpiExt::Dma::write(Control *ctl,Rpi::Dma::Ti::Word ti,
			Rpi::Bus::Address src,
			Rpi::Bus::Memory *dst,uint32_t dofs,
			uint32_t nbytes)
{
    auto section = dst->phys(dofs) ;
    if (section.second < nbytes)
	throw std::runtime_error("PhysMem:access out of range") ;
    ctl->write(ti,src,section.first,nbytes,0) ;
}

void RpiExt::Dma::write(Control *ctl,Rpi::Dma::Ti::Word ti,
			Rpi::Bus::Memory *src,uint32_t sofs,
			Rpi::Bus::Address dst,
			uint32_t nbytes)
{
    auto section = src->phys(sofs) ;
    if (section.second < nbytes)
	throw std::runtime_error("PhysMem:access out of range") ;
    ctl->write(ti,section.first,dst,nbytes,0) ;
}

