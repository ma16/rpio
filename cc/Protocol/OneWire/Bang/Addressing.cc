// BSD 2-Clause License, see github.com/ma16/rpio

#include "Addressing.h"
#include "Error.h"
#include <cassert>

using namespace Protocol::OneWire::Bang ;

unsigned Addressing::descend(Address const &address,bool alarm)
{
    auto present = this->signaling.init() ;
    if (!present)
	throw Error(Error::Type::Vanished,__LINE__) ;
    alarm
      ? this->signaling.write(Master::AlarmSearch) 
      : this->signaling.write(Master::  SearchRom) ;

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

boost::optional<Address> Addressing::first(bool alarm)
{
    auto present = this->signaling.init() ;
    if (!present)
	return boost::none ;
    alarm
      ? this->signaling.write(Master::AlarmSearch) 
      : this->signaling.write(Master::  SearchRom) ;
    Address address ;
    try
    {
	this->traverse(&address,0) ;
    }
    catch (Error &error)
    {
	if (!alarm)
	    throw ;
	if (Error::Type::Vanished != error.type())
	    throw ;
	return boost::none ;
    }
    return address ;
} 

boost::optional<Address> Addressing::next(Address const &prev,bool alarm)
{
    auto offset = this->descend(prev,alarm) ;
    if (offset == 64)
	return boost::none ;
    
    auto present = this->signaling.init() ;
    if (!present)
	throw Error(Error::Type::NotPresent,__LINE__) ;
    alarm
      ? this->signaling.write(Master::AlarmSearch) 
      : this->signaling.write(Master::  SearchRom) ;

    // head: trace address till branch
    this->track(prev,offset) ;

    // the branch index
    auto bit = this->signaling.read() ;
    auto inv = this->signaling.read() ;
    if ((bit != 0) || (inv != 0))
	throw Error(Error::Type::Vanished,__LINE__) ;
    assert(prev[offset] == 0) ;
    // ...otherwise we got a bug in descend()
    this->signaling.write(true) ;
    // copy address so far
    auto next = prev ; next[offset] = 1 ;

    // tail: trace the lowest-address-branch
    this->traverse(&next,offset+1) ;
    return next ;
}

boost::optional<Address> Addressing::rom()
{
    auto present = this->signaling.init() ;
    if (!present)
	return boost::none ;
    this->signaling.write(Master::ReadRom) ;
    return this->signaling.read<64>() ;
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
