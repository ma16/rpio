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

#include "Peripheral.h"
#include <Neat/safe_int.h>
#include <Posix/Fd.h>
#include <Posix/base.h> 

#include <fstream>
#include <sstream>
#include <stdexcept>

Posix::Fd::uoff_t Rpi::Peripheral::by_devtree()
{
  auto fd = Posix::Fd::open("/proc/device-tree/soc/ranges",Posix::Fd::Open::RO) ;
  unsigned char buffer[8] ;
  auto n = fd->read(buffer,Posix::Fd::ussize_t::make<sizeof(buffer)>()) ;
  if (n.as_unsigned() != sizeof(buffer))
    throw Error("Peripheral:read(/proc/device-tree/soc/ranges,8):premature end-of-file") ;
  uint32_t address = buffer[4] ;
  address <<= 8 ; address |= buffer[5] ;
  address <<= 8 ; address |= buffer[6] ;
  address <<= 8 ; address |= buffer[7] ;
  // ...see https://www.raspberrypi.org/forums/viewtopic.php?f=33&t=98740
  return Posix::Fd::uoff_t::make(address) ;
}

Posix::Fd::uoff_t Rpi::Peripheral::by_cpuinfo()
{
  std::ifstream is("/proc/cpuinfo") ;
  if (!is)
    throw Error("Peripheral:ifstream(/proc/cpuinfo):"+Posix::strerror(errno)) ;
  std::string line ;
  while (std::getline(is,line)) {
    if (line.find("ARMv6") != line.npos) return Posix::Fd::uoff_t::make<0x20000000>() ;
    if (line.find("ARMv7") != line.npos) return Posix::Fd::uoff_t::make<0x3f000000>() ;
    if (line.find("ARMv8") != line.npos) return Posix::Fd::uoff_t::make<0x3f000000>() ;
  }
  if (!is.eof())
    throw Error("Peripheral:ifstream(/proc/cpuinfo):"+Posix::strerror(errno)) ;
  throw Error("Peripheral:no ARM found in /proc/cpuinfo") ;
}

std::shared_ptr<Rpi::Peripheral> Rpi::Peripheral::make(Posix::Fd::uoff_t addr)
{
  static auto exists = false ;
  if (exists)
    throw Error("Peripheral:multiple initialisations") ;
  if (0 != (addr.as_unsigned() % Page::nbytes))
    throw Error("Peripheral:base address is not page aligned") ;
  auto base_page = addr.as_unsigned() / Page::nbytes ;
  auto self = std::shared_ptr<Peripheral>(new Peripheral(base_page,Map())) ;
  exists = true ;
  return self ;
}

std::shared_ptr<Rpi::Page const> Rpi::Peripheral::page(PNo no) const
{
  auto i = this->map.find(no.value()) ;
  if (i == this->map.end()) {
    auto page = Page::load(Neat::safe_add(this->base_page,no.value())) ;
    i = this->map.insert(std::make_pair(no.value(),page)).first ;
  }
  return i->second ;
}

std::shared_ptr<Rpi::Page> Rpi::Peripheral::page(PNo no)
{
  return Neat::clip_const(Neat::as_const(this)->page(no)) ;
}

uint32_t volatile const & Rpi::Peripheral::at(size_t i) const
{
  if (0 != (i % 4))
    throw Error("Peripheral:address is not page aligned") ;
  i /= 4 ;
  auto ofs = Rpi::Page::Index::make(i % 0x400) ;
  i /= 0x400 ;
  auto pno = Rpi::Peripheral::PNo::make(i) ;
  return this->page(pno)->at(ofs) ; 
}

uint32_t volatile & Rpi::Peripheral::at(size_t i)
{
  return Neat::clip_const(Neat::as_const(this)->at(i)) ;
}
    
