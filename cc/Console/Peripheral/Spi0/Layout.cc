// BSD 2-Clause License, see github.com/ma16/rpio

#include "Layout.h"
#include <Rpi/Spi0.h>
#include <Rpi/Timer.h>
#include <deque>

// ----[ actual allocation ]-------------------------------------------

static Rpi::Bus::Alloc::Chunk alloc_reset_data(Rpi::Bus::Alloc *alloc,bool adcs)
{
    uint32_t flags = Rpi::Spi0::ClearRx | Rpi::Spi0::ClearTx | Rpi::Spi0::Dmaen ;
    if (adcs)
	flags |= Rpi::Spi0::Adcs ;
    return alloc->seize<uint32_t>(flags) ;
    // to set up spi's control register
}

static Rpi::Bus::Alloc::Chunk alloc_tx_data(Rpi::Bus::Alloc *alloc,Console::Spi0::Mosi const &mosi,uint32_t flags)
{
    auto c = alloc->seize(sizeof(uint32_t) + mosi.nbytes()) ;
    // control word
    auto p = c.as<uint32_t*>() ;
    (*p) = (mosi.nbytes() << 16) | Rpi::Spi0::Ta | flags ;
    // mosi bytes
    auto q = c.as<uint8_t*>() + sizeof(uint32_t) ;
    std::copy(mosi.begin(),mosi.end(),q) ;
    return c ;
    // to be placed into spi's tx-fifo dat
}

// ----[ generic DMA CB setup ]----------------------------------------

static Rpi::Bus::Alloc::Chunk alloc_cb(Rpi::Bus::Alloc *alloc,Rpi::Dma::Ti::Word ti,Rpi::Bus::Address src,Rpi::Bus::Address dst,uint32_t nbytes)
{
    auto c = alloc->seize(8 * sizeof(uint32_t),8 * sizeof(uint32_t)) ;

    auto p = c.as<uint32_t*>() ;

    p[0] = ti.value() ;
    p[1] = src.value() ;
    p[2] = dst.value() ;
    p[3] = nbytes ;
    p[4] = 0 ;
    p[5] = 0 ;
    p[6] = 0 ;
    p[7] = 0 ;

    return c ;
}

static void link_cb(std::deque<Rpi::Bus::Alloc::Chunk> const &q,bool loop)
{
    auto i = q.begin() ;
    auto p = i->as<uint32_t*>() ;
    while (++i != q.end())
    {
	p[5] = i->address().value() ;
	p = i->as<uint32_t*>() ;
    }
    if (loop)
	p[5] = q.front().address().value() ;
}

// ----[ DMA Transfer Information ]------------------------------------

// transfer a single word, no pacing
static Rpi::Dma::Ti::Word make_1x1()
{
    using namespace Rpi::Dma::Ti ;
    
    Word w ;
    w %= Inten       ::make<0>() ; 
    w %= Tdmode      ::make<0>() ; 
    w %= WaitResp    ::make<1>() ; // play it safe
    
    w %= SrcInc      ::make<0>() ; 
    w %= SrcWidth    ::make<0>() ; // play it safe 
    w %= SrcDreq     ::make<0>() ; 
    w %= SrcIgnore   ::make<0>() ;
    
    w %= DestInc     ::make<0>() ; 
    w %= DestWidth   ::make<0>() ; // play it safe
    w %= DestDreq    ::make<0>() ; 
    w %= DestIgnore  ::make<0>() ;
    
    w %= BurstLength ::make<0>() ; 
    w %= Permap      ::make<0>() ;
    w %= Waits       ::make<0>() ; 
    w %= NoWideBursts::make<1>() ; // play it safe

    return w ;
}

// transfer buffer to peripheral register, no pacing
static Rpi::Dma::Ti::Word make_Nx1()
{
    using namespace Rpi::Dma::Ti ;
    
    Word w ;
    w %= Inten       ::make<0>() ; 
    w %= Tdmode      ::make<0>() ; 
    w %= WaitResp    ::make<1>() ; // play it safe
    
    w %= SrcInc      ::make<1>() ; // buffer gets incremented
    w %= SrcWidth    ::make<0>() ; // play it safe 
    w %= SrcDreq     ::make<0>() ; 
    w %= SrcIgnore   ::make<0>() ;
    
    w %= DestInc     ::make<0>() ; 
    w %= DestWidth   ::make<0>() ; // play it safe
    w %= DestDreq    ::make<0>() ; 
    w %= DestIgnore  ::make<0>() ;
    
    w %= BurstLength ::make<0>() ; 
    w %= Permap      ::make<0>() ;
    w %= Waits       ::make<0>() ; 
    w %= NoWideBursts::make<1>() ; // play it safe

    return w ;
}

// transfer peripheral register (fifo) to buffer, with pacing
static Rpi::Dma::Ti::Word make_1xN(Rpi::Dma::Ti::Permap permap)
{
    using namespace Rpi::Dma::Ti ;
    
    Word w ;
    w %= Inten       ::make<0>() ; 
    w %= Tdmode      ::make<0>() ; 
    w %= WaitResp    ::make<1>() ; // play it safe
    
    w %= SrcInc      ::make<0>() ; 
    w %= SrcWidth    ::make<0>() ; // only 32-bit words
    w %= SrcDreq     ::make<1>() ; // paced by peripheral
    w %= SrcIgnore   ::make<0>() ;
    
    w %= DestInc     ::make<1>() ; // buffer gets incremented
    w %= DestWidth   ::make<0>() ; // play it safe
    w %= DestDreq    ::make<0>() ; 
    w %= DestIgnore  ::make<0>() ;
    
    w %= BurstLength ::make<0>() ; 
    w %=                  permap ;
    w %= Waits       ::make<0>() ; 
    w %= NoWideBursts::make<1>() ; // play it safe

    return w ;
}

// ----[ specific DMA CB setup ]---------------------------------------

static Rpi::Bus::Alloc::Chunk alloc_reset_cb(Rpi::Bus::Alloc *alloc,Rpi::Bus::Alloc::Chunk const &data)
{
    return alloc_cb(alloc,make_1x1(),data.address(),Rpi::Spi0::ctrl_addr(),data.nbytes()) ;
}

static Rpi::Bus::Alloc::Chunk alloc_tx_cb(Rpi::Bus::Alloc *alloc,Rpi::Bus::Alloc::Chunk const &data)
{
    return alloc_cb(alloc,make_Nx1(),data.address(),Rpi::Spi0::fifo_addr(),data.nbytes()) ;
}

static Rpi::Bus::Alloc::Chunk alloc_ts_cb(Rpi::Bus::Alloc *alloc,Rpi::Bus::Alloc::Chunk const &data)
{
    return alloc_cb(alloc,make_1x1(),Rpi::Timer::Address,data.address(),data.nbytes()) ;
}

static Rpi::Bus::Alloc::Chunk alloc_rx_cb(Rpi::Bus::Alloc *alloc,Rpi::Bus::Alloc::Chunk const &data)
{
    return alloc_cb(alloc,make_1xN(Rpi::Dma::Ti::Permap::make<7>()),Rpi::Spi0::fifo_addr(),data.address(),data.nbytes()) ;
}

// ----[ DMA memory layout ]-------------------------------------------

size_t Console::Spi0::Layout::index(Rpi::Dma::Channel *channel) const
{
    auto i = channel->getCb().value() ;
    return (i == 0) ? i : (i - cb_start.value()) / cb_rec_nby ;
}
    
size_t Console::Spi0::Layout::nbytes(Console::Spi0::Mosi::Sequence const &mosiV,size_t nrecords)
{
    // reset
    auto n = sizeof(uint32_t) ;
    // for each MOSI: (control + MOSI) 
    n += mosiV.nitems() * sizeof(uint32_t) + mosiV.nbytes() ;
    // NRECORDS x for each MISO: (time-stamp + MISO)
    n += nrecords * (mosiV.nitems() * sizeof(uint32_t) + mosiV.nbytes()) ;
    // align to 32-byte boundary
    n = (n + 0x1fu) & ~0x1fu ;
    // NRECORDS x (4 x |MOSI|) * 32
    n += nrecords * mosiV.nitems() * 4 * 8 *sizeof(uint32_t) ;
    return n ;
}

Console::Spi0::Layout Console::Spi0::Layout::setup(
    Mosi::Sequence const &mosiV,
    size_t             nrecords,
    uint32_t              flags,
    bool                   adcs,
    bool                   loop)
{
    auto alloc = Rpi::Bus::Alloc::reserve(nbytes(mosiV,nrecords)) ;
    
    auto reset_data = alloc_reset_data(&alloc,adcs) ;

    std::vector<Rpi::Bus::Alloc::Chunk> tx_data ;
    for (auto const &mosi: mosiV)
    {
	tx_data.push_back(alloc_tx_data(&alloc,mosi,flags)) ;
    }

    std::vector<std::vector<Rpi::Bus::Alloc::Chunk>> ts_data ;
    std::vector<std::vector<Rpi::Bus::Alloc::Chunk>> rx_data ;
    ts_data.reserve(nrecords * mosiV.nitems()) ;
    rx_data.reserve(nrecords * mosiV.nitems()) ;
    
    for (decltype(nrecords) i=0 ; i<nrecords ; ++i)
    {
	ts_data.push_back(std::vector<Rpi::Bus::Alloc::Chunk>()) ;
	rx_data.push_back(std::vector<Rpi::Bus::Alloc::Chunk>()) ;
	for (auto const &mosi: mosiV)
	{
	    ts_data.back().push_back(alloc.seize(sizeof(uint32_t))) ;
	    rx_data.back().push_back(alloc.seize(   mosi.nbytes())) ;
	}
	assert(ts_data[i].size() == mosiV.nitems()) ;
	assert(rx_data[i].size() == mosiV.nitems()) ;
    }
    assert(ts_data.size() == nrecords) ;
    assert(rx_data.size() == nrecords) ;

    // list of DMA control blocks
    
    std::deque<Rpi::Bus::Alloc::Chunk> q ;

    for (decltype(nrecords) i=0 ; i<nrecords ; ++i)
    {
	for (decltype(mosiV.nitems()) j=0 ; j<mosiV.nitems() ; ++j)
	{
	    q.push_back( alloc_reset_cb(&alloc,reset_data)    ) ;

	    q.push_back( alloc_tx_cb   (&alloc,tx_data   [j]) ) ;

	    q.push_back( alloc_ts_cb   (&alloc,ts_data[i][j]) ) ;

	    q.push_back( alloc_rx_cb   (&alloc,rx_data[i][j]) ) ;
	}
    }

    link_cb(q,loop) ;

    auto cb_rec_nby = mosiV.nitems() * 4 * 0x20 ;
    auto rx_rec_nby = mosiV.nitems() * sizeof(uint32_t) + mosiV.nbytes() ;

    return Layout(alloc,
		  q.front().address()                    ,cb_rec_nby,
		  ts_data.front().front().as<uint32_t*>(),rx_rec_nby) ;
}
