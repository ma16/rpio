// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Neat_U32_h_
#define _Neat_U32_h_

namespace Neat
{

namespace U32
{
    // bit-count after dropping leading zeros, e.g. 17 for 0x0001ff00
    template<uint32_t M> constexpr size_t Tail()
    {
	return 1+Tail< (M >> 1) >() ;
    }
    template<> constexpr size_t Tail<0>() { return 0 ; }

    // bit-count after dropping trailing zeros, e.g. 24 for 0x0001ff00
    template<uint32_t M> constexpr size_t Head()
    {
	return 1+Head< (M << 1) >() ;
    }
    template<> constexpr size_t Head<0>() { return 0 ; }

    // bit-count of trailing zeros, e.g. 8 for 0x0001ff00
    template<uint32_t M> constexpr size_t AlignRight()
    {
	static_assert(M != 0,"") ;
	return 32 - Head<M>() ;
    }

    // bit-count after dropping leading and trailing zeros, e.g. 15 for 0x0001ff00
    template<uint32_t M> constexpr size_t Sig()
    {
	return Tail<M>() - AlignRight<M>() ;
    }

    // create bit mask of size N
    template<size_t N> constexpr uint32_t Mask()
    {
	return (Mask<N-1>() << 1) | 1 ;
    }
    template<> constexpr uint32_t Mask<1> () { return 1 ; }
    template<> constexpr uint32_t Mask<0> () { return 0 ; }

    // return true if contiguous (exactly one cluster of 1-digits)
    template<uint32_t M> constexpr bool Contiguous()
    {
	return M == ( Mask< Sig<M>() >() << AlignRight<M>()) ;
    }
}

}

#endif // _Neat_U32_h_
