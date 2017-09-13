// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef INCLUDE_Protocol_OneWire_Bang_Signaling_h
#define INCLUDE_Protocol_OneWire_Bang_Signaling_h

#include "Master.h"

namespace Protocol { namespace OneWire { namespace Bang { 

struct Signaling
{
    // reset the bus and return the presence pulse (if any)
    bool init() ;

    // read time-slot
    bool read() { return read(false) ; }

    // for convenience
    template<size_t N> std::bitset<N> read()
    {
	std::bitset<N> set(0) ;
	for (size_t i=0 ; i<N ; ++i)
	    set[i] = this->read() ;
	return set ;
    }
    
    // the same as !read(); however throws Retry not if reset
    bool isBusy() { return !read(true) ; }

    // write time-slot
    void write(bool bit) ;
    
    // for convenience
    template<size_t N> void write(std::bitset<N> const &set) 
    {
	for (size_t i=0 ; i<N ; ++i)
	    this->write(set[i]) ;
    }
    
    Signaling(Master *master) : master(master) {}

private:

    bool read(bool busy) ;

    Master *master ;

} ; } } }

#endif // INCLUDE_Protocol_OneWire_Bang_Signaling_h
