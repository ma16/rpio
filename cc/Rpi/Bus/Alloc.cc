// BSD 2-Clause License, see github.com/ma16/rpio

#include "Alloc.h"
#include <Posix/base.h>

Rpi::Bus::Alloc Rpi::Bus::Alloc::reserve(size_t nbytes)
{
    auto vcio = std::make_shared<Mbox::Vcio>() ;
    auto iface = Mbox::Interface::make(vcio) ;
    Mbox::Property::Memory::Mode mode ; mode.direct = 1 ; mode.permaLock = 1 ; // [todo]
    static uint32_t const align = Posix::page_size() ;
    auto p = Mbox::Memory::allocate(iface,nbytes,align,mode) ;
    return Alloc(p,0) ;
} 

Rpi::Bus::Alloc::Chunk Rpi::Bus::Alloc::seize(size_t nbytes,size_t align)
{
    // [todo] introduce alignment class as a power of 2
    if (align == 0)
	throw Error("Bus::Alloc:zero-alignment not permitted") ;
    if (nbytes == 0)
	throw Error("Bus::Alloc:zero-allocation not permitted") ;
    this->ofs = ((this->ofs+align-1) / align) * align ;
    if (this->ofs + nbytes > this->p->size())
    {
	std::ostringstream os ;
	os << "Bus::Alloc:" << this->ofs << " + " << nbytes << " exceeds " << this->p->size() ;
	throw std::runtime_error(os.str()) ;
    }
    auto p = static_cast<char*>(this->p->front()) + this->ofs ;
    auto addr = Address(this->p->address().value() + this->ofs) ;
    this->ofs += nbytes ;
    return Chunk(p,addr,nbytes) ;
}
