// BSD 2-Clause License, see github.com/ma16/rpio

#include "Peripheral.h"
#include <Neat/cast.h>
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
    using T = Posix::Fd::uoff_t ;
    return T::make(Neat::promote<T::Unsigned>(address)) ;
}

Posix::Fd::uoff_t Rpi::Peripheral::by_cpuinfo()
{
    std::ifstream is("/proc/cpuinfo") ;
    if (!is)
	throw Error("Peripheral:ifstream(/proc/cpuinfo):"+Posix::strerror(errno)) ;
    std::string line ;
    while (std::getline(is,line))
    {
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
    auto mem = Posix::Fd::open("/dev/mem",Posix::Fd::Open::RW) ;
    auto self = new Peripheral(base_page,mem,Map()) ;
    exists = true ;
    return std::shared_ptr<Peripheral>(self) ;
}

std::shared_ptr<Rpi::Page const> Rpi::Peripheral::page(PNo no) const
{
    auto i = this->map.find(no.value()) ;
    if (i == this->map.end())
    {
	auto ofs = Neat::make_safe(this->base_page)+no.value() ;
	auto page = Page::load(this->mem.get(),ofs) ;
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
	throw Error("Peripheral:address is not word aligned") ;
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
    
