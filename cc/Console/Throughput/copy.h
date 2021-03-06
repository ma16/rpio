// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Console_Throughput_copy_h_
#define _Console_Throughput_copy_h_

#include <Neat/cast.h>

#include <cassert>
#include <cstddef> // size_t
#include <cstdint> // uint32_t
#include <cstdlib> // abort()

namespace Console { namespace Throughput { 

// ------------------------------------------------------------------

using CopyFunc_t = void (*) (uint32_t const *s,uint32_t *d) ;
using PeekFunc_t = void (*) (uint32_t const *s            ) ;
using PokeFunc_t = void (*) (                  uint32_t *d) ;

// ------------------------------------------------------------------

// poke "0:1": N x { n/a -> [d] }
// peek "1:0": N x { [c] -> n/a }
// copy "1:1": N x { [c] -> [d] }

template<size_t N> inline void copy_x01(                  uint32_t *d) { *Neat::as_volatile(d) = 0                     ; copy_x01<N-1>(  d) ; }
template<size_t N> inline void copy_x10(uint32_t const *s            ) { auto x = *Neat::as_volatile(s) ;(void)x       ; copy_x10<N-1>(s  ) ; }
template<size_t N> inline void copy_x11(uint32_t const *s,uint32_t *d) { *Neat::as_volatile(d) = *Neat::as_volatile(s) ; copy_x11<N-1>(s,d) ; }

template<> inline void copy_x01<0>(                uint32_t*) { }
template<> inline void copy_x10<0>(uint32_t const*          ) { }
template<> inline void copy_x11<0>(uint32_t const*,uint32_t*) { }

// Select copy-function at runtime

template<size_t N> inline PokeFunc_t copy_x01_func(size_t n) { if (N == n) return copy_x01<N> ; else return copy_x01_func<N-1>(n) ; }
template<size_t N> inline PeekFunc_t copy_x10_func(size_t n) { if (N == n) return copy_x10<N> ; else return copy_x10_func<N-1>(n) ; }
template<size_t N> inline CopyFunc_t copy_x11_func(size_t n) { if (N == n) return copy_x11<N> ; else return copy_x11_func<N-1>(n) ; }

template<> PokeFunc_t inline copy_x01_func<0>(size_t n) { assert(n==0) ; (void)n ; return copy_x01<0> ; }
template<> PeekFunc_t inline copy_x10_func<0>(size_t n) { assert(n==0) ; (void)n ; return copy_x10<0> ; }
template<> CopyFunc_t inline copy_x11_func<0>(size_t n) { assert(n==0) ; (void)n ; return copy_x11<0> ; }

// ------------------------------------------------------------------

// poke "0:n":       n/a   ->   (d,d+N] 
// copy "1:n": { N x [s] } ->   (d,d+N] 
// peek "n:0":   (s,s+N]   ->       n/a
// copy "n:1":   (s,s+N]   -> { N x [d] }
// copy "n:n":   (s,s+N]   ->   (d,d+N]

template<size_t N> inline void copy_0n(                  uint32_t *d) { *Neat::as_volatile(d) = 0                     ; copy_0n<N-1>(    d+1) ; }
template<size_t N> inline void copy_1n(uint32_t const *s,uint32_t *d) { *Neat::as_volatile(d) = *Neat::as_volatile(s) ; copy_1n<N-1>(s  ,d+1) ; }
template<size_t N> inline void copy_n0(uint32_t const *s            ) { auto x = *Neat::as_volatile(s) ;(void)x       ; copy_n0<N-1>(s+1    ) ; }
template<size_t N> inline void copy_n1(uint32_t const *s,uint32_t *d) { *Neat::as_volatile(d) = *Neat::as_volatile(s) ; copy_n1<N-1>(s+1,d  ) ; }
template<size_t N> inline void copy_nn(uint32_t const *s,uint32_t *d) { *Neat::as_volatile(d) = *Neat::as_volatile(s) ; copy_nn<N-1>(s+1,d+1) ; }

template<> inline void copy_0n<0>(                uint32_t*) { }
template<> inline void copy_1n<0>(uint32_t const*,uint32_t*) { }
template<> inline void copy_n0<0>(uint32_t const*          ) { }
template<> inline void copy_n1<0>(uint32_t const*,uint32_t*) { }
template<> inline void copy_nn<0>(uint32_t const*,uint32_t*) { }

// Select copy-function at runtime

template<size_t N> inline PokeFunc_t copy_0n_func(size_t n) { if (N == n) return copy_0n<N> ; else return copy_0n_func<N-1>(n) ; }
template<size_t N> inline CopyFunc_t copy_1n_func(size_t n) { if (N == n) return copy_1n<N> ; else return copy_1n_func<N-1>(n) ; }
template<size_t N> inline PeekFunc_t copy_n0_func(size_t n) { if (N == n) return copy_n0<N> ; else return copy_n0_func<N-1>(n) ; }
template<size_t N> inline CopyFunc_t copy_n1_func(size_t n) { if (N == n) return copy_n1<N> ; else return copy_n1_func<N-1>(n) ; }
template<size_t N> inline CopyFunc_t copy_nn_func(size_t n) { if (N == n) return copy_nn<N> ; else return copy_nn_func<N-1>(n) ; }

template<> PokeFunc_t inline copy_0n_func<0>(size_t n) { assert(n==0) ; (void)n ; return copy_0n<0> ; }
template<> CopyFunc_t inline copy_1n_func<0>(size_t n) { assert(n==0) ; (void)n ; return copy_1n<0> ; }
template<> PeekFunc_t inline copy_n0_func<0>(size_t n) { assert(n==0) ; (void)n ; return copy_n0<0> ; }
template<> CopyFunc_t inline copy_n1_func<0>(size_t n) { assert(n==0) ; (void)n ; return copy_n1<0> ; }
template<> CopyFunc_t inline copy_nn_func<0>(size_t n) { assert(n==0) ; (void)n ; return copy_nn<0> ; }

// ------------------------------------------------------------------

} /* Throughput */ } /* Console */
      
#endif // _Console_Throughput_copy_h_
