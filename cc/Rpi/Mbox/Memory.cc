// BSD 2-Clause License, see github.com/ma16/rpio

#include "../Error.h"
#include "Memory.h"
#include "Vcio.h"
#include <Posix/base.h>
#include <Posix/Fd.h>
#include <cassert>

using   Fd = Posix::  Fd ;
using MMap = Posix::MMap ;

Rpi::Mbox::Memory::shared_ptr Rpi::Mbox::Memory::allocate(Interface::shared_ptr iface,uint32_t nbytes,uint32_t align,Property::Memory::Mode mode)
{
    assert(nbytes + Posix::page_size() > nbytes) ;
    auto h = Property::Memory::allocate(iface.get(),nbytes,align,mode) ;
    if (h == 0)
	throw Error("Mbox::Memory:allocate(" + std::to_string(nbytes) + ") failed") ;
    auto addr = Property::Memory::lock(iface.get(),h) ;
    if (addr == 0)
	// ...[todo] release memory
	throw Error("Mbox::Memory:lock failed") ;
    auto fd = Fd::open("/dev/mem",Fd::Open::RW) ; // [todo] Linux specific 
    auto offset = Fd::uoff_t::make(addr & ~(0x3ul<<30)) ;
    auto mmap = MMap::make(fd.get(),offset,nbytes,MMap::Prot::RW,false) ;
    return shared_ptr(new Memory(iface,h,Rpi::Bus::Address(addr),mmap)) ;
}

Rpi::Mbox::Memory::~Memory()
{
    this->mmap.reset() ;
    Property::Memory::unlock(this->iface.get(),this->h) ;
    Property::Memory::release(this->iface.get(),this->h) ;
    // ...[todo] these may throw; which shouldn't happen in a d'tor
}

