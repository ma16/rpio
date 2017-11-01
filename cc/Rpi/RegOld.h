// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef INCLUDE_Rpi_RegOld_h
#define INCLUDE_Rpi_RegOld_h

#include "Bus/Address.h"
#include <Neat/Bit/Word.h>
#include "Page.h"

namespace Rpi { namespace RegOld {

template<uint32_t A> struct Word
{
    static constexpr auto Address = Bus::Address(A) ;

    static constexpr auto Offset = A % Page::nbytes ;

    uint32_t read() const { return (*p) ; }
	
    void write(uint32_t w) { (*p) = w ; }
	
    uint32_t volatile *p ; Word(uint32_t volatile *p) : p(p) {}
} ;
	
template<uint32_t A> constexpr Bus::Address Word<A>::Address ;

template<uint32_t A,uint32_t M> struct Masked
{
    static constexpr auto Address = Bus::Address(A) ;

    static constexpr auto Offset = A % Page::nbytes ;

    static constexpr auto Mask = M ; 

    using Word = Neat::Bit::Word<uint32_t,Mask> ; 
	
    Word read() const { return Word::coset(*p) ; }
	
    void write(Word w) { (*p) = w.value() ; }
	
    uint32_t volatile *p ; Masked(uint32_t volatile *p) : p(p) {}
} ;

template<uint32_t A,uint32_t M> constexpr Bus::Address Masked<A,M>::Address ;

} }

#endif // INCLUDE_Rpi_RegOld_h
