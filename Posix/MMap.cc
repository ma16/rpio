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

#include "MMap.h"
#include "base.h"

std::unique_ptr<Posix::MMap> Posix::MMap::make(Fd *fd,Fd::uoff_t offset,size_t length,Prot prot_,bool share)
{
  int prot = std::underlying_type<decltype(prot_)>::type(prot_) ;
  int flags = 0 ;
  if (share)
    flags |= MAP_SHARED ;
  auto p = ::mmap(nullptr,length,prot,MAP_SHARED,fd->i,offset.as_signed()) ;
  if (p == MAP_FAILED) 
    throw Error("mmap():"+strerror(errno)) ;
  return std::unique_ptr<MMap>(new MMap(p,length)) ;
}
    
Posix::MMap::~MMap()
{
  auto result = munmap(this->p,this->n) ;
  if (result != 0)
    throw Error("unmap():"+strerror(errno)) ;
  // [todo] throws in d'tor
}
