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

#ifndef _Rpi_Page_h_
#define _Rpi_Page_h_

#include <Neat/Enum.h>
#include <Posix/MMap.h>
#include <cstdint> // uint32_t
#include <memory> // shared_ptr

namespace Rpi
{
  struct Page
  {
    static const size_t nbytes = 4096 ;
    // ...throws at runtime if not
    static const size_t nwords = nbytes / sizeof(uint32_t) ;
    
    template<unsigned i> uint32_t      & at()       { static_assert(i<nwords,"") ; return front()[i] ; }
    template<unsigned i> uint32_t const& at() const { static_assert(i<nwords,"") ; return front()[i] ; }

    using Index = Neat::Enum<unsigned,nwords-1> ;
    
    uint32_t      & operator[](Index i)       { return front()[i.value()] ; }
    uint32_t const& operator[](Index i) const { return front()[i.value()] ; }

    Page           (Page const&) = delete ;
    Page& operator=(Page const&) = delete ;

  private:

    friend class Peripheral ;

    static std::shared_ptr<Page> load(size_t pno) ; 
    
    std::unique_ptr<Posix::MMap> mmap ;

    Page(std::unique_ptr<Posix::MMap> &&mmap) : mmap(std::move(mmap)) {}

    uint32_t      * front()       { return reinterpret_cast<uint32_t*>(mmap->front()) ; }
    uint32_t const* front() const { return reinterpret_cast<uint32_t*>(mmap->front()) ; }
    
  } ;
}

#endif // _Rpi_Page_h_
