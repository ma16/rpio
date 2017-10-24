// BSD 2-Clause License, see github.com/ma16/rpio

#include "Page.h"
#include "Error.h"

#include <Neat/safe_int.h>
#include <Posix/base.h> // page_size()

static bool startup_check()
{
  if (Posix::page_size() != Rpi::Page::nbytes)
    throw Rpi::Error("page-size is not 4096 bytes") ;
  return true ;
}

static volatile auto dummy = startup_check() ;

Rpi::Page::shared_ptr Rpi::Page::load(Posix::Fd *fd,size_t pno)
{
  auto bpp = Neat::promote<Posix::Fd::uoff_t::Unsigned>(Page::nbytes) ;
  auto nbytes = Neat::safe_mult(pno,bpp) ;
  auto offset = Posix::Fd::uoff_t::make(nbytes) ;
  auto mmap = Posix::MMap::make(fd,offset,Page::nbytes,Posix::MMap::Prot::RW,false) ;
  return Page::shared_ptr(new Page(mmap)) ;
  // note: the mapping persists even though the file gets closed
}
// ...[todo]
// --open with O_SYNC or not? Does it make a difference for reading from
//   RAM, writing to RAM, accessing peripherals?!
// --does a MAP_SHARED mapping of physical memory make any difference?!
// --provide two variants? One for periherals, another for RAM? at this
//   point it is only used for peripherals.
