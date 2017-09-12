// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef INCLUDE_Device_Ds18x20_Bang_OneWire_Address_h
#define INCLUDE_Device_Ds18x20_Bang_OneWire_Address_h

#include "Master.h"

#include <bitset>
#include <boost/optional.hpp>

namespace Device { namespace Ds18x20 { namespace Bang { namespace OneWire {

struct Addressing
{
    using Address = std::bitset<64> ; // the "ROM code"

    // get device address (single drop bus only)
    boost::optional<Address> get() ;

    // get address of first device
    boost::optional<Address> first() ;

    // get address of next device
    boost::optional<Address> next(Address const&) ;

    Addressing(Master *master) : master(master) {}
    
private:    

    Master *master ;
    
    // fill the address beginning at offset by following the low-
    // address-branch (throws on problems)
    void traverse(Address*,size_t offset) ;
    // a search ROM command must be executed before

    // execute a search ROM command and follow the given address
    // return the offset of lowest branch where address[offset]=0
    unsigned descend(Address const&) ;
    // returns 64 if no such offset exists

    // track the given address for the first nbits
    void track(Address const&,size_t nbits) ;
    // a search ROM command must be executed before

    // a search ROM command must have been executed and the address
    // must have been tracked till the offset.
    // -> read the next address bit
    // this bit must be a branch (determined by descend)
    // -> follow address bit 1
    // this is then completed by following the low-address-branch 
    Address branch(Address const&,size_t offset) ;
    
    // [todo] make offset a number in the range 0..63
    

} ; } } } }

#endif // INCLUDE_Device_Ds18x20_Bang_OneWire_Address_h
