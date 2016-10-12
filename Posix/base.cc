// Copyright (c) 2016, "ma16". All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions 
// are met:
//
// * Redistributions of source code must retain the above copyright 
//   notice, this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright 
//   notice, this list of conditions and the following disclaimer in 
//   the documentation and/or other materials provided with the 
//   distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
// HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
// OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED 
// AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
// WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
// POSSIBILITY OF SUCH DAMAGE.
//
// This project is hosted at https://github.com/ma16/rpio

#include "base.h"
#include "Neat/cast.h"

#include <cassert>
#include <cstring> // strerror
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
