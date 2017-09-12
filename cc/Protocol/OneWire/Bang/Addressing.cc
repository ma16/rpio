// BSD 2-Clause License, see github.com/ma16/rpio

#include "Addressing.h"
#include "Error.h"
#include <cassert>

using namespace Protocol::OneWire::Bang ;

boost::optional<Addressing::Address> Addressing::get()
{
    auto present = this->signaling.init() ;
    if (!present)
	return boost::none ;
    this->signaling.write(Master::ReadRom) ;
    return this->signaling.read<64>() ;
}

boost::optional<Addressing::Address> Addressing::first()
{
    auto present = this->signaling.init() ;
    if (!present)
	return boost::none ;
    this->signaling.write(Master::SearchRom) ;
    // [todo] support Search Alarm 
    Address address ;
    this->traverse(&address,0) ;
    return address ;
} 

boost::optional<Addressing::Address> Addressing::next(Address const &prev)
{
    auto offset = this->descend(prev) ;
    // [todo] support Search Alarm 
    if (offset == 64)
	return boost::none ;
    
    auto present = this->signaling.init() ;
    if (!present)
	throw Error(Error::Type::NotPresent,__LINE__) ;
    this->signaling.write(Master::SearchRom) ;
    // [todo] support Search Alarm 

    this->track(prev,offset) ;
    auto next = this->branch(prev,offset) ;
    this->traverse(&next,offset+1) ;
    
    return next ;
}

void Addressing::traverse(Address *address,size_t offset)
{
    for (auto i=offset ; i<64 ; ++i)
    {
	auto bit = this->signaling.read() ;
	auto inv = this->signaling.read() ;

	if ((bit == 1) && (inv == 1))
	    throw Error(Error::Type::Vanished,__LINE__) ;
	
	// if 0-1: address bit is 1 (for all attached devices)
	// if 1-0: address bit is 0 (for all attached devices)
	// if 0-0: attached devices split into 0 and 1 addresses
	this->signaling.write(bit) ;
	// ...proceed with the lowest address bit (if more than one)
	
	(*address)[i] = bit ;
    }
}

unsigned Addressing::descend(Address const &address)
{
    auto present = this->signaling.init() ;
    if (!present)
	throw Error(Error::Type::Vanished,__LINE__) ;
    this->signaling.write(Master::SearchRom) ;
    // [todo] support Search Alarm 

    auto branch = 64u ; // default: no new branches found
    
    for (auto i=0u ; i<64 ; ++i)
    {
	auto bit = this->signaling.read() ;
	auto inv = this->signaling.read() ;

	if ((bit == 1) && (inv == 1))
	    throw Error(Error::Type::Vanished,__LINE__) ;
	if (bit != inv) // (0,1) and (1,0)
	{
	    if (address[i] != bit)
		throw Error(Error::Type::Vanished,__LINE__) ;
	}
	else // (0,0)
	{
            // attached devices split into 0 and 1 addresses
	    if (address[i] == 0)
	    {
		// address is in the low-address-branch, so we might
		// wanna branch here (unless there are other branches)
		branch = i ;
	    }
	}

	this->signaling.write(address[i]) ;
    }
    return branch ;
}

void Addressing::track(Address const &address,size_t nbits)
{
    for (auto i=0u ; i<nbits ; ++i)
    {
	auto bit = this->signaling.read() ;
	auto inv = this->signaling.read() ;

	if ((bit == 1) && (inv == 1))
	    throw Error(Error::Type::Vanished,__LINE__) ;
	if (bit != inv) // (0,1) and (1,0)
	{
	    if (address[i] != bit)
		throw Error(Error::Type::Vanished,__LINE__) ;
	}
	this->signaling.write(address[i]) ;
    }
}

Addressing::Address Addressing::branch(Address const &address,size_t offset)
{
    auto bit = this->signaling.read() ;
    auto inv = this->signaling.read() ;

    if ((bit == 1) && (inv == 1))
	throw Error(Error::Type::Vanished,__LINE__) ;
    if (bit != inv) // (0,1) and (1,0)
	throw Error(Error::Type::Vanished,__LINE__) ;
    else // (0,0)
    {
	assert(address[offset] == 0) ;
	// otherwise we got a bug in descend()
    }
    this->signaling.write(true) ;

    auto next = address ;
    next[offset] = 1 ;
    return next ;
}
