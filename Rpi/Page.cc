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

std::shared_ptr<Rpi::Page> Rpi::Page::load(size_t pno)
{
  auto fd = Posix::Fd::open("/dev/mem",Posix::Fd::Open::RW) ;
  auto offset = Posix::Fd::uoff_t::make(Neat::safe_mult(pno,Page::nbytes)) ;
  auto mmap = Posix::MMap::make(fd.get(),offset,Page::nbytes,Posix::MMap::Prot::RW,false) ;
  return std::shared_ptr<Page>(new Page(std::move(mmap))) ;
  // note: the mapping persists even though the file gets closed
}
// ...[todo]
// --open with O_SYNC or not? Does it make a difference for reading from
//   RAM, writing to RAM, accessing peripherals?!
// --does a MAP_SHARED mapping of physical memory make any difference?!
// --provide two variants? One for periherals, another for RAM? at this
//   point it is only used for peripherals.
