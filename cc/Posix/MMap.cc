// BSD 2-Clause License, see github.com/ma16/rpio

#include "MMap.h"
#include "base.h"
#include <sstream>

Posix::MMap::shared_ptr Posix::MMap::make(Fd *fd,Fd::uoff_t offset,size_t length,Prot prot_,bool share)
{
  int prot = std::underlying_type<decltype(prot_)>::type(prot_) ;
  int flags = 0 ;
  if (share)
    flags |= MAP_SHARED ;
  // ...[todo] forgot to use it?
  auto p = ::mmap(nullptr,length,prot,MAP_SHARED,fd->i,offset.as_signed()) ;
  if (p == MAP_FAILED) {
    std::ostringstream os ;
    os << "mmap(" << length << ',' << prot << ',' << offset.as_signed() << "):" << strerror(errno) ;
    throw Error(os.str()) ;
  }
  return shared_ptr(new MMap(p,length)) ;
}
    
Posix::MMap::~MMap()
{
  auto result = munmap(this->p,this->n) ;
  if (result != 0)
    throw Error("unmap():"+strerror(errno)) ;
  // [todo] throws in d'tor
}
