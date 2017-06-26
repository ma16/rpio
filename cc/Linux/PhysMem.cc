// BSD 2-Clause License, see github.com/ma16/rpio

#include "PhysMem.h"
#include "base.h"
#include <Posix/base.h>
//#include <algorithm> // std::min
#include <cassert>

static size_t adjust(size_t nbytes)
{
  return (nbytes + Posix::page_size() - 1) & ~(Posix::page_size()-1) ;
}

Linux::PhysMem::shared_ptr Linux::PhysMem::allocate(size_t nbytes) 
{
  assert(nbytes + Posix::page_size() > nbytes) ;
  auto size = adjust(nbytes) ;
  auto p = Posix::memalign(size,Posix::page_size()) ;
  // ...[future] make p exception safe
  Posix::mlock(p,size) ;

  auto vpno = reinterpret_cast<uintptr_t>(p) / Posix::page_size() ;
  auto npages = size / Posix::page_size() ;
  std::vector<uintptr_t> pfnV ; pfnV.reserve(npages) ;
  for (size_t i=0 ; i<npages ; ++i)
    pfnV.push_back(Linux::phys_page(vpno+i)) ;

  return shared_ptr(new PhysMem(p,true,nbytes,std::move(pfnV))) ;
}

Linux::PhysMem::shared_ptr Linux::PhysMem::map(void *p,size_t nbytes) 
{
  assert(nbytes + Posix::page_size() > nbytes) ;
  assert(0 == (reinterpret_cast<uintptr_t>(p) % Posix::page_size())) ;
  auto size = adjust(nbytes) ;
  Posix::mlock(p,size) ;

  auto vpno = reinterpret_cast<uintptr_t>(p) / Posix::page_size() ;
  auto npages = size / Posix::page_size() ;
  std::vector<uintptr_t> pfnV ; pfnV.reserve(npages) ;
  for (size_t i=0 ; i<npages ; ++i)
    pfnV.push_back(Linux::phys_page(vpno+i)) ;

  return shared_ptr(new PhysMem(p,false,nbytes,std::move(pfnV))) ;
}

Linux::PhysMem::~PhysMem() 
{
  auto size = adjust(this->nbytes()) ;
  Posix::munlock(this->p,size) ;
  if (this->doRelease)
    free(this->p) ;
}

std::pair<uintptr_t,size_t> Linux::PhysMem::phys(size_t ofs)
{
  assert(ofs <= nbytes()) ;
  auto pfn = this->pfnV.at(ofs / Posix::page_size()) ;
  auto addr = pfn * Posix::page_size() + (ofs % Posix::page_size()) ;
  auto left = Neat::demote<size_t>(Posix::page_size()-(ofs%Posix::page_size())) ;
  left = std::min(left,nbytes()-ofs) ;
  return std::make_pair(addr,left) ;
}
