// BSD 2-Clause License, see github.com/ma16/rpio

#include "base.h"

#include <Neat/cast.h>
#include <Posix/base.h>

#include <cassert>
#include <fcntl.h> // O_RDWR
#include <sstream>
#include <stdexcept>
#include <unistd.h> // open(),read(),close()

static uint64_t phys_info(uint64_t vpno)
{
  // see proc(5) or http://www.mjmwired.net/kernel/Documentation/vm/pagemap.txt
  int fd = open("/proc/self/pagemap",O_RDONLY) ; 
  // ...fails with -EPERM unless user with CAP_SYS_ADMIN capability
  // ...[todo] fd is vulnerable to leak
  if (fd < 0) 
    throw std::runtime_error("open(/proc/self/pagemap,O_RDONLY):"+Posix::strerror(errno)) ;
  uint64_t info ; 
  auto fofs = static_cast<off_t>(vpno*sizeof(info)) ; // [todo] overflow
  auto result = lseek(fd,fofs,SEEK_SET) ;
  if (result < 0) 
    throw std::runtime_error("lseek(/proc/self/pagemap,SEEK_SET):"+Posix::strerror(errno)) ;
  if (result != fofs) 
    throw std::runtime_error("lseek(/proc/self/pagemap,SEEK_SET) < offset") ;
  result = read(fd,&info,sizeof(info)) ;
  if (result < 0) 
    throw std::runtime_error("read(/proc/self/pagemap,8):"+Posix::strerror(errno)) ;
  if (result != sizeof(info))
    throw std::runtime_error("read(/proc/self/pagemap,8) != 8") ;
  // ...[note] assumes machine's endianess in /proc/self/pagemap
  result = close(fd) ;
  if (result != 0) 
    throw std::runtime_error("close(/proc/self/pagemap):"+Posix::strerror(errno)) ;
  return info ;
}

uint32_t Linux::phys_page(uint64_t vpno) 
{
  auto info = phys_info(vpno) ; 
  static uint64_t const xpct = (0x2ull << 62) ; // page present but not swapped
  static uint64_t const mask = (0x3ull << 62) ; // the two most significant bits
  if (xpct != (info & mask))
    throw std::runtime_error("page not present or currently swapped") ;
  return static_cast<uint32_t>(info & 0x007fffffffffffffull) ; // [todo] overflow  
}
