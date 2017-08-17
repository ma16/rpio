// BSD 2-Clause License, see github.com/ma16/rpio

// --------------------------------------------------------------------
// read/write directly from/into memory
// --------------------------------------------------------------------

#include "../rpio.h"
#include <Posix/base.h>
#include <Posix/MMap.h>
#include <Rpi/Peripheral.h>
#include <Ui/strto.h>
#include <iostream>

static Posix::MMap::shared_ptr map(Posix::Fd::uoff_t offset,size_t nbytes)
{
  auto mem = Posix::Fd::open("/dev/mem",Posix::Fd::Open::RW) ;
  auto mmap = Posix::MMap::make(mem.get(),offset,nbytes,Posix::MMap::Prot::RW,false) ;
  return mmap ;
}

static void get(Posix::Fd::uoff_t base,unsigned ofs,Ui::ArgL *argL)
{
  argL->finalize() ;
  auto mmap = map(base,ofs+4) ;
  auto p = mmap->as<uint32_t const*>() ;
  std::cout << std::hex << p[ofs/4] << std::endl ;
}

static void read(Posix::Fd::uoff_t base,unsigned ofs,Ui::ArgL *argL)
{
  auto nbytes = Ui::strto(argL->pop(),Posix::Fd::ussize_t()) ;
  auto fd = Posix::Fd::create(argL->pop().c_str()) ;
  argL->finalize() ;
  auto mmap = map(base,ofs+nbytes.as_unsigned()) ;
  
  void const *p = mmap->as<char const*>() + ofs ;
  auto nwritten = fd->write(p,nbytes) ;
  if (nwritten.as_unsigned() != nbytes.as_unsigned())
    throw std::runtime_error("not enough data") ;
}
  
static void write(Posix::Fd::uoff_t base,unsigned ofs,Ui::ArgL *argL)
{
  auto fd = Posix::Fd::open(argL->pop().c_str(),Posix::Fd::Open::RO) ;
  argL->finalize() ;
  auto nbytes = fd->size().as_unsigned() ;
  auto mmap = map(base,ofs+nbytes) ;

  void *p = mmap->as<char*>() + ofs ;
  auto nbytes_ = Posix::Fd::ussize_t::make(Neat::demote<Posix::Fd::ussize_t::Unsigned>(nbytes)) ;
  auto nread = fd->read(p,nbytes_) ;
  if (nbytes != nread.as_unsigned())
    throw std::runtime_error("not enough data") ;
}
  
static void set(Posix::Fd::uoff_t base,unsigned ofs,Ui::ArgL *argL)
{
  auto word = Ui::strto<uint32_t>(argL->pop()) ;
  argL->finalize() ;
  auto mmap = map(base,ofs+4) ;
  auto p = mmap->as<uint32_t*>() ;
  p[ofs/4] = word ;
  std::cout << std::hex << p[ofs/4] << std::endl ;
}

void Console::Poke::invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
  if (argL->empty() || argL->peek() == "help") { 
    std::cout << "arguments: [-p] ADDR MODE\n"
	      << '\n'
	      << "MODE :   get\n"
	      << "     |  read NBYTES FILE\n"
	      << "     |   set WORD\n"
	      << "     | write FILE \n"
	      << '\n'
	      << "ADDR = virtual memory address\n"
	      << "-p   = ADDR is relative to peripheral base address\n"
	      << "WORD = 32-bit integer number\n"
	      << '\n'
	      << "for example: <-p 0x200034 get> displays GPIO levels\n"
	      << std::flush ;
    return ;
  }

  auto p = argL->pop_if("-p") ;
  auto addr = Ui::strto(argL->pop(),Posix::Fd::uoff_t()) ;
  if (p)  
    addr = decltype(addr)::make(addr.as_unsigned() + rpi->base_addr()) ; // [todo] watch overflow
  static auto const page_size = Posix::page_size() ;
  auto bas = Posix::Fd::uoff_t::make((addr.as_unsigned() / page_size)*page_size) ;
  auto ofs =                          addr.as_unsigned() % page_size ;

  std::string mode = argL->pop() ;
  if      (mode ==   "get")   get(bas,ofs,argL) ;
  else if (mode ==  "read")  read(bas,ofs,argL) ;
  else if (mode ==   "set")   set(bas,ofs,argL) ;
  else if (mode == "write") write(bas,ofs,argL) ;
  
  else throw std::runtime_error("not supported option:<"+mode+'>') ;
}
