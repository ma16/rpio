// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Linux_PhysMem_h_
#define _Linux_PhysMem_h_

// A page-aligned block of memory that is physically locked
//
// We use page-aligned memory since the atomic unit for mlock(2) and
// munlock(2) is a single page. If we wouldn't use page-aligned
// memory, an unlock() may hit a page that is still supposed to be
// locked (if the page is shared by two PhysMem instances).

#include <Neat/PhysMem.h>
#include <vector>

namespace Linux { struct PhysMem : Neat::PhysMem
{
  using shared_ptr = std::shared_ptr<PhysMem> ;
  
  static shared_ptr allocate(size_t nbytes) ;

  static shared_ptr map(void *p,size_t nbytes) ;

  virtual void* virt() override { return p ; }
  
  virtual std::pair<uintptr_t,size_t> phys(size_t ofs) override ;

  size_t nbytes() const { return nbytes_ ; }

  virtual ~PhysMem() override ;

  size_t npages() const { return pfnV.size() ; } // [todo]

private:

  void *p ; bool doRelease ; size_t nbytes_ ; std::vector<uintptr_t> pfnV ;

  PhysMem(void *p,bool doRelease,size_t nbytes,std::vector<uintptr_t> &&pfnV) : p(p),doRelease(doRelease),nbytes_(nbytes),pfnV(std::move(pfnV)) {}

} ; /* PhysMem */ } /* Linux */

#endif // _Linux_PhysMem_h_
