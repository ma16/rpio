// BSD 2-Clause License, see github.com/ma16/rpio

#include "Fd.h"
#include "base.h"

#include <Neat/cast.h>

#include <cassert>
#include <sstream>

#include <sys/ioctl.h> 
#include <unistd.h> // open(),read(),close()

void Posix::Fd::ioctl(unsigned long request,void *arg)
{
  auto result = ::ioctl(i,request,arg) ;
  if (result == -1) {
    std::ostringstream os ;
    os << "ioctl(" << request << "):" << strerror(errno) ;
    throw Error(os.str()) ;
  }
}

Posix::Fd::shared_ptr Posix::Fd::open(char const path[],Open mode_)
{
  auto mode = std::underlying_type<decltype(mode_)>::type(mode_) ;
  auto i = ::open(path,mode) ; 
  if (i < 0) {
    std::ostringstream os ;
    os << "open(" << path << ',' << mode << "):" << strerror(errno) ;
    throw Error(os.str()) ;
  }
  return shared_ptr(new Fd(i,std::string(path))) ;
}

Posix::Fd::shared_ptr Posix::Fd::create(char const path[])
{
  auto mode = O_CREAT | O_WRONLY | O_TRUNC ;
  auto i = ::open(path,mode,0666) ; 
  if (i < 0) {
    std::ostringstream os ;
    os << "open(" << path << ',' << mode << "):" << strerror(errno) ;
    throw Error(os.str()) ;
  }
  return shared_ptr(new Fd(i,std::string(path))) ;
}

Posix::Fd::ussize_t Posix::Fd::read(void *buf,ussize_t count)
{
  // we use ussize_t since read(2) returns a non-negative ssize_t
  // value on success. read(2) also states: "if count is greater than
  // SSIZE_MAX, the result is unspecified". 
  auto n = ::read(this->i,buf,count.as_unsigned()) ;
  if (n < 0) {
    assert(n == -1) ;
    std::ostringstream os ;
    os << "read(" << this->path << ',' << count.as_unsigned() << "):" << strerror(errno) ;
    throw Error(os.str()) ;
  }
  return ussize_t::make(Neat::as_unsigned(n)) ;
}
    
Posix::Fd::ussize_t Posix::Fd::write(void const *buf,ussize_t count)
{
  // we use ussize_t since write(2) returns a non-negative ssize_t
  // value on success.
  auto n = ::write(this->i,buf,count.as_unsigned()) ;
  if (n < 0) {
    assert(n == -1) ;
    std::ostringstream os ;
    os << "write(" << this->path << ',' << count.as_unsigned() << "):" << strerror(errno) ;
    throw Error(os.str()) ;
  }
  return ussize_t::make(Neat::as_unsigned(n)) ;
}

Posix::Fd::uoff_t Posix::Fd::lseek(off_t ofs,Lseek mode_) 
{
  auto mode = std::underlying_type<decltype(mode_)>::type(mode_) ;
  auto result = ::lseek(this->i,ofs,mode) ;
  if (result < 0) {
    assert(result == -1) ;
    std::ostringstream os ;
    os << "lseek(" << this->path << ',' << ofs << ',' << mode << "):" << strerror(errno) ;
    throw Error(os.str()) ;
  }
  return uoff_t::make(result) ;
}

Posix::Fd::uoff_t Posix::Fd::size() 
{
  auto beg = this->lseek(0,Lseek::Current) ;
  auto end = this->lseek(0,Lseek::End) ;
  auto pos = this->lseek(beg.as_signed(),Lseek::Begin) ;
  assert(pos.as_unsigned() == beg.as_unsigned()) ; (void)pos ;
  return end ;
}

Posix::Fd::~Fd()
{
  auto result = close(i) ;
  (void)result ;
  // [todo] log:"close():"+strerror(errno)
}

