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

#include "Fd.h"
#include "base.h"

#include <Neat/cast.h>

#include <cassert>
#include <sstream>

#include <unistd.h> // open(),read(),close()

std::unique_ptr<Posix::Fd> Posix::Fd::open(char const path[],Open mode_)
{
  int mode = std::underlying_type<decltype(mode_)>::type(mode_) ;
  auto i = ::open(path,mode) ; 
  if (i < 0) {
    std::ostringstream os ;
    os << "open(" << path << ',' << mode << "):" << strerror(errno) ;
    throw Error(os.str()) ;
  }
  return std::unique_ptr<Fd>(new Fd(i,std::string(path))) ;
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
    
Posix::Fd::~Fd()
{
  auto result = close(i) ;
  if (result != 0)
    throw Error("unmap():"+strerror(errno)) ;
  // [todo] throws in d'tor
}

