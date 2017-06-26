// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Rpi_Page_h_
#define _Rpi_Page_h_

#include <Neat/Enum.h>
#include <Posix/MMap.h>
#include <cstdint> // uint32_t
#include <memory> // shared_ptr

namespace Rpi
{
  struct Page
  {
    using shared_ptr = std::shared_ptr<Page> ;
    
    static const size_t nbytes = 4096 ;
    // ...throws at runtime if not
    static const size_t nwords = nbytes / sizeof(uint32_t) ;
    
    using Index = Neat::Enum<unsigned,nwords-1> ;

    volatile uint32_t      & at(Index i)       { return front()[i.value()] ; }
    volatile uint32_t const& at(Index i) const { return front()[i.value()] ; }

    template<unsigned i> volatile uint32_t      & at()       { static_assert(i<nwords,"") ; return front()[i] ; }
    template<unsigned i> volatile uint32_t const& at() const { static_assert(i<nwords,"") ; return front()[i] ; }

    Page           (Page const&) = delete ;
    Page& operator=(Page const&) = delete ;

  private:

    friend class Peripheral ;

    static Page::shared_ptr load(size_t pno) ; 
    
    Posix::MMap::shared_ptr mmap ; Page(Posix::MMap::shared_ptr mmap) : mmap(mmap) {}

    volatile uint32_t      * front()       { return reinterpret_cast<volatile uint32_t*>(mmap->front()) ; }
    volatile uint32_t const* front() const { return reinterpret_cast<volatile uint32_t*>(mmap->front()) ; }
  } ;
}

#endif // _Rpi_Page_h_
