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

#include "Buffer.h"
#include <Neat/safe_int.h>
#include <Posix/base.h> // page_size
#include <Ui/strto.h>
#include <cstring> // libc:memcpy,memset

struct Plain : public Main::Throughput::Buffer
{
  static std::unique_ptr<Buffer> alloc(size_t nwords)
  {
    auto page_size = static_cast<size_t>(Posix::page_size()) ;
    auto nbytes = Neat::safe_add(page_size-1,Neat::safe_mult(nwords,sizeof(uint32_t))) ;
    auto p = std::unique_ptr<char[]>(new char[nbytes]) ;
    auto i = reinterpret_cast<uintptr_t>(p.get()) + page_size-1 ;
    i -= i % page_size ;
    auto q = reinterpret_cast<uint32_t*>(i) ;
    memset(q,0,nwords*sizeof(uint32_t)) ;
    return std::unique_ptr<Plain>(new Plain(nwords,std::move(p),q)) ;
  }

  virtual uint32_t* front()       override { return      q ; }
  virtual size_t     size() const override { return nwords ; }

private:

  Plain(size_t nwords_,std::unique_ptr<char[]> &&p_, uint32_t *q_) : nwords(nwords_),p(std::move(p_)),q(q_) {}

  size_t nwords ; std::unique_ptr<char[]> p ; uint32_t *q ;
} ;

struct Port : public Main::Throughput::Buffer
{
  static std::unique_ptr<Buffer> map(std::shared_ptr<Rpi::Page> page,Rpi::Page::Index ix)
  {
    return std::unique_ptr<Port>(new Port(page,ix)) ;
  }

  virtual uint32_t* front() override { return Neat::clip_volatile(&page->at(ix)) ; }
  
  virtual size_t size() const override { return 1 ; }
  
private:
  
  Port(std::shared_ptr<Rpi::Page> page_,Rpi::Page::Index ix_) : page(page_),ix(ix_) {}
  
  std::shared_ptr<Rpi::Page> page ; Rpi::Page::Index ix ;
} ;

std::unique_ptr<Main::Throughput::Buffer> Main::Throughput::locate(Rpi::Peripheral *rpi,size_t nwords,Ui::ArgL *argL)
{
  auto arg = argL->pop() ;
  if (arg == "plain") {
    return Plain::alloc(nwords) ;
  }
  if (arg == "port") {
    if (nwords != 1)
      throw Neat::Error("Main:Buffer:only single word support for peripheral access yet") ;
    auto page_no = Ui::strto(argL->pop(),Rpi::Peripheral::PNo()) ;
    auto page = rpi->page(page_no) ;
    auto page_of = Ui::strto<unsigned>(argL->pop()) ; 
    if (0 != (page_of % sizeof(uint32_t)))
      throw Neat::Error("Main:Buffer:peripheral address must be 32-bit aligned") ;
    auto page_ix = Rpi::Page::Index::make(page_of/4) ;
    auto p = Port::map(page,page_ix) ;
    return p ;
  }
  throw Neat::Error("Main:Buffer:not supported option:<"+arg+'>') ;
}
