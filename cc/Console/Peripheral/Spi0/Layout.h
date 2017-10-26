// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef Console_Spi0_Layout_h
#define Console_Spi0_Layout_h

// Work in progress (allocates memory for DMA transfer)

#include "Mosi.h"

#include <Rpi/Bus/Alloc.h>
#include <Rpi/Dma.h>

namespace Console { namespace Spi0 {

struct Layout
{
    static Layout setup(
	Console::Spi0::Mosi::Sequence const &mosiV, // spi data 
	size_t                         nrecords, // number of repetitions
	uint32_t                          flags, // spi flags (11 LSB bits)
	bool                               adcs, // another spi flag
	bool                               loop) // chain last dma block to first one
	;

    // enter first DMA control block 
    void enter(Rpi::Dma::Channel *channel,Rpi::Dma::Cs cs)
    { channel->setup(cb_start,cs) ; }
    
    // read DMA control block register and return related record index
    size_t index(Rpi::Dma::Channel *channel) const ;

    // pointer to the array of incoming data records
    void* front() const { return rx_start ; }
    
    // size of an incoming spi data record for the given MOSI sequence
    size_t record_size() const { return rx_rec_nby ; }

private:
    
    Rpi::Bus::Alloc alloc ;

    uint32_t *rx_start ;
  
    Console::Spi0::Mosi::Sequence::V::size_type rx_rec_nby ;
    
    Rpi::Bus::Address cb_start ;
  
    Console::Spi0::Mosi::Sequence::V::size_type cb_rec_nby ;

    Layout(Rpi::Bus::Alloc alloc,
	   Rpi::Bus::Address cb_start,size_t  cb_rec_nby,
	   uint32_t         *rx_start,size_t  rx_rec_nby)
	:
	alloc(alloc),
	rx_start(rx_start),rx_rec_nby(rx_rec_nby),
	cb_start(cb_start),cb_rec_nby(cb_rec_nby)
    {
    }

    static size_t nbytes(Console::Spi0::Mosi::Sequence const &mosiV,size_t nrecords) ;
} ;

} }

#endif // Console_Spi0_Layout_h
