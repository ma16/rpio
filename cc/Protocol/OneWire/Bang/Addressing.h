// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef INCLUDE_Protocol_OneWire_Bang_Addressing_h
#define INCLUDE_Protocol_OneWire_Bang_Addressing_h

#include "Address.h"
#include "Signaling.h"

#include <bitset>
#include <boost/optional.hpp>

namespace Protocol { namespace OneWire { namespace Bang { 

struct Addressing
{
    // get device address (single drop bus only)
    boost::optional<Address> get() ;
    // ...issues a Read-ROM command

    // get address of first device
    boost::optional<Address> first(bool alarm=false) ;
    // ...issues an Alarm/Search-ROM command

    // get address of next device
    boost::optional<Address> next(Address const&,bool alarm=false) ;
    // ...issues an Alarm/Search-ROM command

    Addressing(Master *master) : signaling(master) {}
    
private:    

    Signaling signaling ;
    
    // execute a Search-ROM command and keep track of all branches
    // while tracing the given address; return the lowest index where
    // address[index]=0 (another address starts at this branch)
    unsigned descend(Address const&,bool alarm=false) ;
    // returns 64 if no such index exists

    // track the given address for the first nbits
    void track(Address const&,size_t nbits) ;
    // a Search-ROM command must have been executed before

    // fill the address beginning at offset by tracing the lowest-
    // address-branch
    void traverse(Address*,size_t offset) ;
    // a Search-ROM command must have been executed before

} ; } } }

#endif // INCLUDE_Protocol_OneWire_Bang_Addressing_h
