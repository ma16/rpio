// BSD 2-Clause License, see github.com/ma16/rpio

#include "Buffer.h"
#include <Neat/cast.h>
#include <Neat/safe_int.h>
#include <Posix/base.h> // page_size
#include <Ui/strto.h>
#include <cstring> // libc:memcpy,memset

struct Plain : public Console::Throughput::Buffer
{
  static Buffer::shared_ptr alloc(size_t nwords)
  {
    auto page_size = static_cast<size_t>(Posix::page_size()) ;
    auto nbytes = Neat::make_safe(page_size-1) + Neat::make_safe(nwords) * sizeof(uint32_t) ;
    auto p = std::unique_ptr<char[]>(new char[nbytes]) ;
    auto i = reinterpret_cast<uintptr_t>(p.get()) + page_size-1 ;
    i -= i % page_size ;
    auto q = reinterpret_cast<uint32_t*>(i) ;
    memset(q,0,nwords*sizeof(uint32_t)) ;
    return Buffer::shared_ptr(new Plain(nwords,std::move(p),q)) ;
  }

  virtual uint32_t* front()       override { return      q ; }
  virtual size_t     size() const override { return nwords ; }

private:

  Plain(size_t nwords,std::unique_ptr<char[]> p, uint32_t *q) : nwords(nwords),p(std::move(p)),q(q) {}

  size_t nwords ; std::unique_ptr<char[]> p ; uint32_t *q ;
} ;

struct Port : public Console::Throughput::Buffer
{
  static Buffer::shared_ptr map(std::shared_ptr<Rpi::Page> page,Rpi::Page::Index ix)
  {
    return Buffer::shared_ptr(new Port(page,ix)) ;
  }

  virtual uint32_t* front() override { return Neat::clip_volatile(&page->at(ix)) ; }
  
  virtual size_t size() const override { return 1 ; }
  
private:
  
  Port(std::shared_ptr<Rpi::Page> page_,Rpi::Page::Index ix_) : page(page_),ix(ix_) {}
  
  std::shared_ptr<Rpi::Page> page ; Rpi::Page::Index ix ;
} ;

Console::Throughput::Buffer::shared_ptr Console::Throughput::locate(Rpi::Peripheral *rpi,size_t nwords,Ui::ArgL *argL)
{
  auto arg = argL->pop() ;
  if (arg == "plain") {
    return Plain::alloc(nwords) ;
  }
  if (arg == "port") {
    if (nwords != 1)
      throw Neat::Error("Console:Buffer:only single word support for peripheral access yet") ;
    auto page_no = Ui::strto(argL->pop(),Rpi::Peripheral::PNo()) ;
    auto page = rpi->page(page_no) ;
    auto page_of = Ui::strto<unsigned>(argL->pop()) ; 
    if (0 != (page_of % sizeof(uint32_t)))
      throw Neat::Error("Console:Buffer:peripheral address must be 32-bit aligned") ;
    auto page_ix = Rpi::Page::Index::make(page_of/4) ;
    auto p = Port::map(page,page_ix) ;
    return p ;
  }
  throw Neat::Error("Console:Buffer:not supported option:<"+arg+'>') ;
}
