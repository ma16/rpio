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

#ifndef _Posix_MMap_h_
#define _Posix_MMap_h_

#include "Fd.h"

#include <sys/mman.h> // PROT_READ...

namespace Posix
{
  struct MMap
  {
    enum class Prot { RO=PROT_READ,WO=PROT_WRITE,RW=PROT_READ|PROT_WRITE } ;
    
    static std::unique_ptr<MMap> make(Fd *fd,Fd::uoff_t offset,size_t length,Prot prot,bool share) ; 
    // ...note: offset must be a multiple of the page-size
    
    void* front() const { return p ; }
    size_t size() const { return n ; }
      
    ~MMap() ;
      
  private:
      
    void *p ; size_t n ; MMap(void *p,size_t n) : p(p),n(n) {}

    // notes:
    // --mmap can be either on regular file, device file or anonymous
    // --a mapping that is not covered by (enough) physical memory
    //   may cause a SIGSEGV on access (zero page fault). An initial
    //   memset(3)+signal catcher and/or mlock(2) might help to verify
    //   beforehand.
  } ;
} 

#endif // _Posix_MMap_h_
