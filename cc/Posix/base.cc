// BSD 2-Clause License, see github.com/ma16/rpio

#include "base.h"
#include "Neat/cast.h"

#include <cassert>
#include <cmath> // floor
#include <cstring> // strerror
#include <limits> // numeric_limits
#include <sys/mman.h> // mlock() et al.
#include <sys/types.h> // uid_t
#include <sstream>
#include <unistd.h> // sysconf(_SC_PAGE_SIZE)

static unsigned long get_page_size()
{
  // sysconf(3): "The values obtained from these functions are system 
  // configuration constants. They do not change during the lifetime of 
  // a process." So the caller may make it a static const.
  auto result = sysconf(_SC_PAGE_SIZE) ; 
  if (result == -1)
    throw Posix::Error("sysconf(_SC_PAGESIZE):"+Posix::strerror(errno)) ;
  assert(result >= 0) ;
  auto n = Neat::as_unsigned(result) ; 
  // sysconf(3) say's it must not be zero 
  if (n == 0)
    throw Posix::Error("sysconf(_SC_PAGE_SIZE):zero") ;
  // actually, the page-size must be a power-of-two since the MMU
  // splits an address into HI=page-no + LO=offset. Besides
  // posix_memalign(3) requires also a power-of-two.
  if (0 != (n & (n - 1)))
    throw Posix::Error("sysconf(_SC_PAGE_SIZE):no power-of-two") ;
  // posix_memalign(3) requires also a multiple of sizeof(void*)
  if (0 != (n % sizeof(void*)))
    throw Posix::Error("sysconf(_SC_PAGE_SIZE):no multiple of (void*)") ;
  // return: sizeof(void*) <= n <= signed-long::max
  return n ;
  // note: the page-size is almost always 0x1000.
}

void* Posix::memalign(size_t nbytes,size_t alignment)
{
  void *p ;
  auto result = ::posix_memalign(&p,alignment,nbytes) ;
  if (result != 0) {
    std::ostringstream os ;
    os << "posix_memalign(" << alignment << ',' << nbytes << "):" << Posix::strerror(result) ;
    throw Posix::Error(os.str()) ;
  }
  return p ;
}

void Posix::mlock(void const *p,size_t nbytes)
{
  // [todo] mlock() says: "the POSIX.1 specification of mlock() and
  // munlock() allows an implementation to require that addr is page
  // aligned, so portable applications should ensure this."
  // [note] actually it would be best to permit only page-aligned
  // {nbytes} too 
  auto result = ::mlock(p,nbytes) ;
  if (result != 0) {
    std::ostringstream os ;
    os << std::hex << "mlock(" << p << ',' << nbytes << "):" << Posix::strerror(errno) ;
    throw std::runtime_error(os.str()) ;
  }
}

void Posix::munlock(void const *p,size_t nbytes)
{
  auto result = ::munlock(p,nbytes) ;
  if (result != 0) {
    std::ostringstream os ;
    os << std::hex << "munlock(" << p << ',' << nbytes << "):" << Posix::strerror(errno) ;
    throw std::runtime_error(os.str()) ;
    // e.g. ENOMEM if free() was called before
    // besides mlock() says: "The memory lock on an address range is
    // automatically removed if the address range is unmapped via munmap(2)."
  }
}

void Posix::nanosleep(double ns)
{
  if (ns < 0.0) {
    std::ostringstream os ;
    os << "nanosleep(" << ns << "):negative duration" ;
    throw Posix::Error(os.str()) ;
  }
  timespec t ;
  auto i = floor(ns/1E9) ;
  if (i > std::numeric_limits<decltype(t.tv_sec)>::max()) {
    std::ostringstream os ;
    os << "nanosleep(" << ns << "):duration exceeds maximum value" ;
    throw Posix::Error(os.str()) ;
  }
  t.tv_sec = static_cast<decltype(t.tv_sec)>(i) ;
  t.tv_nsec = static_cast<decltype(t.tv_nsec)>(ns-1E9*i+0.5) ;
  if (t.tv_nsec >= 1000000000) {
    if (t.tv_sec == std::numeric_limits<decltype(t.tv_sec)>::max()) {
      std::ostringstream os ;
      os << "nanosleep(" << ns << "):duration exceeds maximum value" ;
      throw Posix::Error(os.str()) ;
    }
    ++t.tv_sec ;
    t.tv_nsec = 0 ;
  }
  auto result = ::nanosleep(&t,nullptr) ;
  if (result != 0) {
    std::ostringstream os ;
    os << "nanosleep(" << ns << "):" << Posix::strerror(errno) ;
    throw Posix::Error(os.str()) ;
  }
}

unsigned long Posix::page_size()
{
  static auto const i = get_page_size() ; return i ;
} 

std::string Posix::strerror(int no)
{
  char buffer[0x1000] ;
  std::ostringstream os ;
  auto result = strerror_r(no,buffer,sizeof(buffer)) ;
#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600) && ! _GNU_SOURCE
  // POSIX's strerror() version
  if (result == 0) {
    os << buffer ;
  }
  else {
    os << "error-number not resolved" ; // either invalid number or buffer too small
  }
#else
  // g++ uses GNU's char* strerror() instead of POSIX's int strerror()
  os << result ;
  // [note] call __xpg_strerror_r() to force POSIX on g++
#endif
  os << " (" << no << ')' ;
  return os.str() ;
}

rusage Posix::getrusage()
{
    struct rusage u ;
    auto result = getrusage(RUSAGE_THREAD,&u) ;
    if (result != 0)
	throw Posix::Error("rusage(RUSAGE_THREAD):"+Posix::strerror(errno)) ;
    return u ;
}

void Posix::reset_uid()
{
    auto uid = getuid() ;
    auto result = setuid(uid) ;
    if (result != 0)
    {
	std::ostringstream os ;
	os << "setuid(" << uid << "):" << strerror(errno) ;
	throw Error(os.str()) ;
    }
}
