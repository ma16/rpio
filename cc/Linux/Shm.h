// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Linux_Shm_h_
#define _Linux_Shm_h_

// --------------------------------------------------------------------
// RAII for Posix::shm::id_t (survives process crash on purpose)
// --------------------------------------------------------------------

#include "PhysMem.h"
#include <Posix/shm.h>

namespace Linux { struct Shm : Neat::PhysMem
{
  using shared_ptr = std::shared_ptr<Shm> ;
  
  static shared_ptr allocate(size_t nbytes) ;

  virtual void* virt() override { return phys_->virt() ; }
  
  virtual std::pair<uintptr_t,size_t> phys(size_t ofs) override { return phys_->phys(ofs) ; }

  size_t nbytes() const { return phys_->nbytes() ; }

  ~Shm() override ;
  
private:
  
  Posix::shm::id_t id ; PhysMem::shared_ptr phys_ ;
  
  Shm(Posix::shm::id_t id,PhysMem::shared_ptr phys) : id(id),phys_(phys) {}

} ; /* Shm */ } /* Linux */ 

#endif // _Linux_Shm_h_
