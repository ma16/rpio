// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef Console_Spi0_Watch_h
#define Console_Spi0_Watch_h

#include <cstddef> // size_t
#include <cstdint> // uint8_t

// --------------------------------------------------------------------
// A buffer (front,nrecords,rsize) is going to be monitored.
//     front    = start address of buffer
//     nrecords = number of records in buffer
//     rsize    = size of a single record in bytes
//
// A record in the buffer is an arbitrary sequence of bytes with one
// exception: The first 32-bit word of the record must hold a time-
// stamp (or more general: some kind of unique id).
//
// The buffer is written to asynchronously, e.g. by DMA. It can serve
// as a ring-buffer: new write operations may overwrite previous ones.
//
// The client keeps somehow track of the last written record index
// (e.g. by monitoring the related DMA register) and depending on this
// the client queries a number of records to be fetched.
//
// This implementation keeps track of the current record index (i0) and
// of the time-stamp of the (current) record (t0).
//
// When a client requests N records to be fetched, this implementation
// does:
//   -- compute the new index i1 := i0 + N
//   -- save the related time-stamp t1 := record[i1].time_stamp
//   -- copy records (i0..i1] into the client's buffer
//   -- return false if record i0 was overwritten, i.e. if
//      t0 != record[i0].time_stamp
//   -- update index i0 : = i1 and time stamp t0 := t1 
// The operations shown above consider the nature of a ring-buffer.
//
// When constructed, the indexed record must be valid, i.e. contain a
// valid time-stamp. A fetch request will copy records (i0,i0+n], so
// it does not include record i0.
//
// This implementation does only verify if there was a wrap around that
// overwrote a not-yet-fetched record. This implementation can not
// verify whether the N records to be fetched have already been written
// to (e.g. by DMA).
// --------------------------------------------------------------------

namespace Console { namespace Spi0 {

struct Watch
{
    Watch(void const *front,size_t nrecords,size_t rsize,size_t i) ;

    void reset(size_t i) ;

    size_t avail(size_t i) const ;
    
    bool overrun() const ;
    
    bool fetch(size_t n,void *buffer) ;

private:
    
    uint8_t const *front ; // first record
    size_t      nrecords ; // number of records
    size_t         rsize ; // record size in bytes
    size_t            i0 ; // index of last saved record
    uint32_t          t0 ; // time-stamp in indexed record
} ;

} }
    
#endif // Console_Spi0_Watch_h
