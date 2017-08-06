// BSD 2-Clause License, see github.com/ma16/rpio

#include "Coherency.h"
#include "Memory.h"
#include <fstream>
#include <Neat/stream.h>
#include <Rpi/ArmMem.h>
#include <Rpi/GpuMem.h>
#include <Ui/strto.h>

Rpi::Bus::Memory::Allocator::shared_ptr Rpi::Ui::Bus::Memory::
getAllocator(Rpi::Peripheral *rpi,::Ui::ArgL *argL) 
{
    auto type = argL->pop({"arm","gpu"}) ;
    switch (type)
    {
    case 0:
    {
	auto co = Coherency::get(argL) ;
	auto stick = argL->pop_if("-s") ;
	return Rpi::ArmMem::Allocator::shared_ptr(new Rpi::ArmMem::Allocator(co,stick)) ;
    }
    case 1:
    {
	auto align = ::Ui::strto<uint32_t>(argL->option("-a","0x1000")) ;
	auto u32 = ::Ui::strto<uint32_t>(argL->option("-m","0x4")) ;
	auto mode = Rpi::Mbox::Property::Memory::Mode::deserialize(u32) ;
	Rpi::Mbox::Interface::shared_ptr iface ;
	if (argL->pop_if("-d")) {
	    auto co = Coherency::get(argL) ;
	    iface = Rpi::Mbox::Interface::make(Rpi::Mbox::Queue(rpi),co) ;
	}
	else {
	    auto vcio = Rpi::Mbox::Vcio::shared_ptr(new Rpi::Mbox::Vcio()) ;
	    iface = Rpi::Mbox::Interface::make(vcio) ;
	}
	return Rpi::GpuMem::Allocator::shared_ptr(new Rpi::GpuMem::Allocator(iface,align,mode)) ;
    }
    }
    abort() ;
}

Rpi::Bus::Memory::Allocator::shared_ptr Rpi::Ui::Bus::Memory::
getAllocator(Rpi::Peripheral *rpi,::Ui::ArgL *argL,Memory::Allocator::shared_ptr def) 
{
    if (argL->pop_if("--memf"))
	return getAllocator(rpi,argL) ;
    return def ;
}

static std::shared_ptr<std::istream> getIn(Ui::ArgL *argL)
{
    std::shared_ptr<std::istream> is ;
    if (argL->pop_if("-i"))
    {
	auto fname = argL->pop() ;
	is.reset(new std::ifstream(fname)) ;
	if (!is)
	    throw std::runtime_error("cannot open input file <"+fname+'>') ;
    }
    return is ;
}
    
static void readIn(std::istream *is,Rpi::Bus::Memory *mem)
{
    if (is != nullptr)
    {
	auto p = mem->as<char*>() ;
	auto n = Neat::to_signed(mem->nbytes()) ;
	is->read(p,n) ;
	if (!is->good())
	    throw std::runtime_error("cannot read input file") ;
    }
}

Rpi::Bus::Memory::shared_ptr Rpi::Ui::Bus::Memory::
read(std::string const &fname,Rpi::Bus::Memory::Allocator *mem)
{
    std::ifstream is ; Neat::open(&is,fname) ;
    auto nbytes = Neat::demote<size_t>(Neat::size(&is).as_unsigned()) ; 
    auto nwords = nbytes / 4 ;
    if (nwords * 4 != nbytes)
	throw std::runtime_error("file-size must be a multiple of 4 bytes") ;
    auto data = mem->allocate(nbytes) ;
    Neat::read(&is,data->virt(),Neat::ustreamsize::make(nbytes)) ;
    return data ;
}

static Rpi::Bus::Memory::shared_ptr make_arm(Ui::ArgL *argL)
{
    auto co = Rpi::Ui::Bus::Coherency::get(argL) ;
    auto stick = argL->pop_if("-s") ;
    auto nbytes = ::Ui::strto<uint32_t>(argL->pop()) ;
    auto is = getIn(argL) ;
    auto mem = Rpi::ArmMem::allocate(nbytes,co,stick) ;
    readIn(is.get(),mem.get()) ;
    return mem ;
}
  
static Rpi::Bus::Memory::shared_ptr make_gpu(Rpi::Peripheral *rpi,Ui::ArgL *argL) 
{
    auto align = ::Ui::strto<uint32_t>(argL->option("-a","0x1000")) ;
    auto u32 = ::Ui::strto<uint32_t>(argL->option("-m","0x4")) ;
    auto mode = Rpi::Mbox::Property::Memory::Mode::deserialize(u32) ;
    Rpi::Mbox::Interface::shared_ptr iface ;
    if (argL->pop_if("-d"))
    {
	auto co = Rpi::Ui::Bus::Coherency::get(argL) ;
	iface = Rpi::Mbox::Interface::make(Rpi::Mbox::Queue(rpi),co) ;
    }
    else
    {
	auto vcio = Rpi::Mbox::Vcio::shared_ptr(new Rpi::Mbox::Vcio()) ;
	iface = Rpi::Mbox::Interface::make(vcio) ;
    }
    auto nbytes = ::Ui::strto<uint32_t>(argL->pop()) ;
    auto is = getIn(argL) ;
    auto mem = Rpi::GpuMem::allocate(iface,nbytes,align,mode) ;
    readIn(is.get(),mem.get()) ;
    return mem ;
}

struct VcRange : Rpi::Bus::Memory
{
    static Rpi::Bus::Memory::shared_ptr make(Ui::ArgL *argL)
    {
	auto addr = ::Ui::strto<uint32_t>(argL->pop()) ;
	auto nbytes = ::Ui::strto<uint32_t>(argL->pop()) ;
	return Rpi::Bus::Memory::shared_ptr(new VcRange(addr,nbytes)) ;
    }
  
    virtual void* virt() override { assert(false) ; }

    virtual std::pair<Rpi::Bus::Address,size_t> phys(size_t ofs) override
    {
	assert(ofs < nbytes()) ;
	return std::make_pair(Rpi::Bus::Address(addr_),nbytes()-ofs) ;
    }
  
    virtual size_t nbytes() const override { return nbytes_ ; }
  
    VcRange(uintptr_t addr,uint32_t nbytes) : addr_(addr),nbytes_(nbytes) {}
  
    uintptr_t addr_ ; uint32_t nbytes_ ;
} ;

Rpi::Bus::Memory::shared_ptr Rpi::Ui::Bus::Memory::
getMemory(Rpi::Peripheral *rpi,::Ui::ArgL *argL) 
{
    auto type = argL->pop({"arm","bus","gpu"}) ;
    switch (type)
    {
    case 0: return make_arm(argL) ;
    case 1: return VcRange::make(argL) ;
    case 2: return make_gpu(rpi,argL) ;
	// ...{rpi} required for Mbox/Queue not for /dev/vcio
    }
    abort() ;
}

