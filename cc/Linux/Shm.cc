// BSD 2-Clause License, see github.com/ma16/rpio

#include "Shm.h"
#include <cstring> // memset

Linux::Shm::shared_ptr Linux::Shm::allocate(size_t nbytes)
{
  auto id = Posix::shm::create(0,nbytes,Posix::shm::ugo_t::make<0777>()) ;
  auto p = Posix::shm::attach(id) ;
  auto phys = Linux::PhysMem::map(p,nbytes) ;
  memset(reinterpret_cast<char*>(p),0xff,nbytes) ;
  return shared_ptr(new Shm(id,phys)) ;
}

Linux::Shm::~Shm()
{
  auto p = phys_->virt() ;
  phys_.reset() ;
  Posix::shm::detach(p) ;
  Posix::shm::destroy(id) ;
}
  
