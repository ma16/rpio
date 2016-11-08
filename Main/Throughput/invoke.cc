// Copyright (c) 2016, "ma16". All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions 
// are met:
//
// * Redistributions of source code must retain the above copyright 
//   notice, this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright 
//   notice, this list of conditions and the following disclaimer in 
//   the documentation and/or other materials provided with the 
//   distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
// HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
// OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED 
// AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
// WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
// POSSIBILITY OF SUCH DAMAGE.
//
// This project is hosted at https://github.com/ma16/rpio

#include "../rpio.h"

#include "Buffer.h"
#include "copy.h"

#include <Neat/cast.h>
#include <Neat/safe_int.h>
#include <Rpi/Peripheral.h>
#include <Ui/strto.h>

#include <chrono>
#include <cstring> // memcpy
#include <iostream>
#include <sstream>

namespace Main { namespace Throughput {

// --------------------------------------------------------------------

using rep_t = size_t ;
// [note] the results are quite different with a 64-bit counter
    
static std::chrono::time_point<std::chrono::steady_clock> now()
{
  return std::chrono::steady_clock::now() ;
}

static void report(rep_t c,size_t nwords,std::chrono::time_point<std::chrono::steady_clock> t0)
{
  auto dt = std::chrono::duration<double>(now()-t0).count() ;
  std::cout.setf(std::ios::scientific) ;
  std::cout.precision(2) ;
  std::cout << static_cast<double>(c) * static_cast<double>(nwords)/dt << "/s" << std::endl ;
}

// --------------------------------------------------------------------

static size_t const MAX = 256 ;
// ...maximum block size for fixed-size copy operations
    
// --------------------------------------------------------------------

static void blck_0n(rep_t rep,size_t nwords,uint32_t *d,Ui::ArgL *argL)
{
  auto m = Ui::strto<size_t>(argL->pop()) ;
  if (m < 1 || MAX < m)
    throw Neat::Error("Main:Throughput:invoke:" + std::to_string(m) + " not in range") ;
  argL->finalize() ;
  auto copyM = copy_0n_func<MAX>(       m) ;
  auto copyR = copy_0n_func<MAX>(nwords%m) ;
  auto t0 = now() ;
  if (m > nwords) {
    for (decltype(rep) i=0 ; i<rep ; ++i) {
      copyR(d) ;
    }
  }
  else if (m == nwords) {
    for (decltype(rep) i=0 ; i<rep ; ++i) {
      copyM(d) ;
    }
  }
  else if (0 == (nwords % m)) {
    for (decltype(rep) i=0 ; i<rep ; ++i) {
      decltype(nwords) j = 0 ;
      while (m <= nwords - j) { 
	copyM(d+j) ;
	j += m ;
      }
    }
  }
  else {
    for (decltype(rep) i=0 ; i<rep ; ++i) {
      decltype(nwords) j = 0 ;
      while (m <= nwords - j) { 
	copyM(d+j) ;
	j += m ;
      }
      copyR(d+j) ;
    }
  }
  report(rep,nwords,t0) ;
}

static void blck_1n(rep_t rep,size_t nwords,uint32_t const *s,uint32_t *d,Ui::ArgL *argL)
{
  auto m = Ui::strto<size_t>(argL->pop()) ;
  if (m < 1 || MAX < m)
    throw Neat::Error("Main:Throughput:invoke:" + std::to_string(m) + " not in range") ;
  argL->finalize() ;
  auto copyM = copy_1n_func<MAX>(       m) ;
  auto copyR = copy_1n_func<MAX>(nwords%m) ;
  auto t0 = now() ;
  if (m > nwords) {
    for (decltype(rep) i=0 ; i<rep ; ++i) {
      copyR(s,d) ;
    }
  }
  else if (m == nwords) {
    for (decltype(rep) i=0 ; i<rep ; ++i) {
      copyM(s,d) ;
    }
  }
  else if (0 == (nwords % m)) {
    for (decltype(rep) i=0 ; i<rep ; ++i) {
      decltype(nwords) j = 0 ;
      while (m <= nwords - j) { 
	copyM(s,d+j) ;
	j += m ;
      }
    }
  }
  else {
    for (decltype(rep) i=0 ; i<rep ; ++i) {
      decltype(nwords) j = 0 ;
      while (m <= nwords - j) { 
	copyM(s,d+j) ;
	j += m ;
      }
      copyR(s,d+j) ;
    }
  }
  report(rep,nwords,t0) ;
}

static void blck_n0(rep_t rep,size_t nwords,uint32_t const *s,Ui::ArgL *argL)
{
  auto m = Ui::strto<size_t>(argL->pop()) ;
  if (m < 1 || MAX < m)
    throw Neat::Error("Main:Throughput:invoke:" + std::to_string(m) + " not in range") ;
  argL->finalize() ;
  auto copyM = copy_n0_func<MAX>(       m) ;
  auto copyR = copy_n0_func<MAX>(nwords%m) ;
  auto t0 = now() ;
  if (m > nwords) {
    for (decltype(rep) i=0 ; i<rep ; ++i) {
      copyR(s) ;
    }
  }
  else if (m == nwords) {
    for (decltype(rep) i=0 ; i<rep ; ++i) {
      copyM(s) ;
    }
  }
  else if (0 == (nwords % m)) {
    for (decltype(rep) i=0 ; i<rep ; ++i) {
      decltype(nwords) j = 0 ;
      while (m <= nwords - j) { 
	copyM(s+j) ;
	j += m ;
      }
    }
  }
  else {
    for (decltype(rep) i=0 ; i<rep ; ++i) {
      decltype(nwords) j = 0 ;
      while (m <= nwords - j) { 
	copyM(s+j) ;
	j += m ;
      }
      copyR(s+j) ;
    }
  }
  report(rep,nwords,t0) ;
}

static void blck_n1(rep_t rep,size_t nwords,uint32_t const *s,uint32_t *d,Ui::ArgL *argL)
{
  auto m = Ui::strto<size_t>(argL->pop()) ;
  if (m < 1 || MAX < m)
    throw Neat::Error("Main:Throughput:invoke:" + std::to_string(m) + " not in range") ;
  argL->finalize() ;
  auto copyM = copy_n1_func<MAX>(       m) ;
  auto copyR = copy_n1_func<MAX>(nwords%m) ;
  auto t0 = now() ;
  if (m > nwords) {
    for (decltype(rep) i=0 ; i<rep ; ++i) {
      copyR(s,d) ;
    }
  }
  else if (m == nwords) {
    for (decltype(rep) i=0 ; i<rep ; ++i) {
      copyM(s,d) ;
    }
  }
  else if (0 == (nwords % m)) {
    for (decltype(rep) i=0 ; i<rep ; ++i) {
      decltype(nwords) j = 0 ;
      while (m <= nwords - j) { 
	copyM(s+j,d) ;
	j += m ;
      }
    }
  }
  else {
    for (decltype(rep) i=0 ; i<rep ; ++i) {
      decltype(nwords) j = 0 ;
      while (m <= nwords - j) { 
	copyM(s+j,d) ;
	j += m ;
      }
      copyR(s+j,d) ;
    }
  }
  report(rep,nwords,t0) ;
}

static void blck_nn(rep_t rep,size_t nwords,uint32_t const *s,uint32_t *d,Ui::ArgL *argL)
{
  auto m = Ui::strto<size_t>(argL->pop()) ;
  if (m < 1 || MAX < m)
    throw Neat::Error("Main:Throughput:invoke:" + std::to_string(m) + " not in range") ;
  argL->finalize() ;
  auto copyM = copy_nn_func<MAX>(       m) ;
  auto copyR = copy_nn_func<MAX>(nwords%m) ;
  auto t0 = now() ;
  if (m > nwords) {
    for (decltype(rep) i=0 ; i<rep ; ++i) {
      copyR(s,d) ;
    }
  }
  else if (m == nwords) {
    for (decltype(rep) i=0 ; i<rep ; ++i) {
      copyM(s,d) ;
    }
  }
  else if (0 == (nwords % m)) {
    for (decltype(rep) i=0 ; i<rep ; ++i) {
      decltype(nwords) j = 0 ;
      while (m <= nwords - j) { 
	copyM(s+j,d+j) ;
	j += m ;
      }
    }
  }
  else {
    for (decltype(rep) i=0 ; i<rep ; ++i) {
      decltype(nwords) j = 0 ;
      while (m <= nwords - j) { 
	copyM(s+j,d+j) ;
	j += m ;
      }
      copyR(s+j,d+j) ;
    }
  }
  report(rep,nwords,t0) ;
}

// --------------------------------------------------------------------

static void iter_0n(rep_t rep,size_t nwords,uint32_t *d_,Ui::ArgL *argL)
{
  argL->finalize() ;
  auto d = Neat::as_volatile(d_) ;
  auto t0 = now() ;
  for (decltype(rep) i=0 ; i<rep ; ++i)
    for (decltype(nwords) j=0 ; j<nwords ; ++j)
      d[j] = 0 ;
  report(rep,nwords,t0) ;
}

static void iter_1n(rep_t rep,size_t nwords,uint32_t const *s_,uint32_t *d_,Ui::ArgL *argL)
{
  argL->finalize() ;
  auto s = Neat::as_volatile(s_) ;
  auto d = Neat::as_volatile(d_) ;
  auto t0 = now() ;
  for (decltype(rep) i=0 ; i<rep ; ++i)
    for (decltype(nwords) j=0 ; j<nwords ; ++j)
      d[j] = s[0] ;
  report(rep,nwords,t0) ;
}

static void iter_n0(rep_t rep,size_t nwords,uint32_t const *s_,Ui::ArgL *argL)
{
  argL->finalize() ;
  auto s = Neat::as_volatile(s_) ;
  auto t0 = now() ;
  for (decltype(rep) i=0 ; i<rep ; ++i)
    for (decltype(nwords) j=0 ; j<nwords ; ++j) {
      auto x = s[j] ; (void)x ;
    }
  report(rep,nwords,t0) ;
}

static void iter_n1(rep_t rep,size_t nwords,uint32_t const *s_,uint32_t *d_,Ui::ArgL *argL)
{
  argL->finalize() ;
  auto s = Neat::as_volatile(s_) ;
  auto d = Neat::as_volatile(d_) ;
  auto t0 = now() ;
  for (decltype(rep) i=0 ; i<rep ; ++i)
    for (decltype(nwords) j=0 ; j<nwords ; ++j)
      d[0] = s[j] ;
  report(rep,nwords,t0) ;
}

static void iter_nn(rep_t rep,size_t nwords,uint32_t const *s_,uint32_t *d_,Ui::ArgL *argL)
{
  argL->finalize() ;
  auto s = Neat::as_volatile(s_) ;
  auto d = Neat::as_volatile(d_) ;
  auto t0 = now() ;
  for (decltype(rep) i=0 ; i<rep ; ++i)
    for (decltype(nwords) j=0 ; j<nwords ; ++j)
      d[j] = s[j] ;
  report(rep,nwords,t0) ;
}

// [opt] optimize 0:1 1:0 1:1 version (for better comparability)

// --------------------------------------------------------------------

static void libc_0n(rep_t rep,size_t nwords,uint32_t *d,Ui::ArgL *argL)
{
  argL->finalize() ;
  Neat::safe_mult(nwords,sizeof(uint32_t)) ;
  auto t0 = now() ;
  for (decltype(rep) i=0 ; i<rep ; ++i)
    memset(d,0,nwords*sizeof(uint32_t)) ;
  report(rep,nwords,t0) ;
}

static void libc_nn(rep_t rep,size_t nwords,uint32_t const *s,uint32_t *d,Ui::ArgL *argL)
{
  argL->finalize() ;
  Neat::safe_mult(nwords,sizeof(uint32_t)) ;
  auto t0 = now() ;
  for (decltype(rep) i=0 ; i<rep ; ++i)
    memcpy(d,s,nwords*sizeof(uint32_t)) ;
  report(rep,nwords,t0) ;
}

// --------------------------------------------------------------------

static void pool_01(rep_t rep,uint32_t *d,Ui::ArgL *argL)
{
  auto m = Ui::strto<size_t>(argL->pop()) ;
  if (m < 1 || MAX < m)
    throw Neat::Error("Main:Throughput:invoke:" + std::to_string(m) + " not in range") ;
  argL->finalize() ;
  auto copyM = copy_x01_func<MAX+1>(                             m ) ;
  auto copyR = copy_x01_func<MAX+1>(static_cast<decltype(m)>(rep%m)) ;
  auto left = rep ; 
  auto t0 = now() ;
  while (m <= left) {
    copyM(d) ;
    left -= m ; 
  }
  copyR(d) ;
  report(rep,1,t0) ;
}

static void pool_10(rep_t rep,uint32_t const *s,Ui::ArgL *argL)
{
  auto m = Ui::strto<size_t>(argL->pop()) ;
  if (m < 1 || MAX < m)
    throw Neat::Error("Main:Throughput:invoke:" + std::to_string(m) + " not in range") ;
  argL->finalize() ;
  auto copyM = copy_x10_func<MAX+1>(                             m ) ;
  auto copyR = copy_x10_func<MAX+1>(static_cast<decltype(m)>(rep%m)) ;
  auto left = rep ; 
  auto t0 = now() ;
  while (m <= left) {
    copyM(s) ;
    left -= m ; 
  }
  copyR(s) ;
  report(rep,1,t0) ;
}

static void pool_11(rep_t rep,uint32_t const *s,uint32_t *d,Ui::ArgL *argL)
{
  auto m = Ui::strto<size_t>(argL->pop()) ;
  if (m < 1 || MAX < m)
    throw Neat::Error("Main:Throughput:invoke:" + std::to_string(m) + " not in range") ;
  argL->finalize() ;
  auto copyM = copy_x11_func<MAX+1>(                             m ) ;
  auto copyR = copy_x11_func<MAX+1>(static_cast<decltype(m)>(rep%m)) ;
  auto left = rep ; 
  auto t0 = now() ;
  while (m <= left) {
    copyM(s,d) ;
    left -= m ; 
  }
  copyR(s,d) ;
  report(rep,1,t0) ;
}

// [opt] use two 32-bit word for left instead of one 64-bit word

// --------------------------------------------------------------------

static void invoke_01(Rpi::Peripheral *rpi,rep_t rep,Ui::ArgL *argL)
{
  auto d = locate(rpi,1,argL) ;
  auto arg = argL->pop() ;
  if      (arg == "blck") blck_0n(rep,1,d->front(),argL) ;
  else if (arg == "iter") iter_0n(rep,1,d->front(),argL) ;
  else if (arg == "pool") pool_01(rep  ,d->front(),argL) ;
  else throw std::runtime_error("not supported option:<"+arg+'>') ;
}

static void invoke_0n(Rpi::Peripheral *rpi,rep_t rep,Ui::ArgL *argL)
{
  auto nwords = Ui::strto<size_t>(argL->pop()) ;
  if (nwords < 2)
    throw Neat::Error("Main:Throughput:invoke:at least two words required") ;
  auto d = locate(rpi,nwords,argL) ;
  auto arg = argL->pop() ;
  if      (arg == "blck") blck_0n(rep,nwords,d->front(),argL) ;
  else if (arg == "iter") iter_0n(rep,nwords,d->front(),argL) ;
  else if (arg == "libc") libc_0n(rep,nwords,d->front(),argL) ;
  else throw std::runtime_error("not supported option:<"+arg+'>') ;
}

static void invoke_10(Rpi::Peripheral *rpi,rep_t rep,Ui::ArgL *argL)
{
  auto s = locate(rpi,1,argL) ;
  auto arg = argL->pop() ;
  if      (arg == "blck") blck_n0(rep,1,s->front(),argL) ;
  else if (arg == "iter") iter_n0(rep,1,s->front(),argL) ;
  else if (arg == "pool") pool_10(rep  ,s->front(),argL) ;
  else throw std::runtime_error("not supported option:<"+arg+'>') ;
}

static void invoke_11(Rpi::Peripheral *rpi,rep_t rep,Ui::ArgL *argL)
{
  auto s = locate(rpi,1,argL) ; 
  auto d = locate(rpi,1,argL) ;
  auto arg = argL->pop() ;
  if      (arg == "blck") blck_nn(rep,1,s->front(),d->front(),argL) ;
  else if (arg == "iter") iter_nn(rep,1,s->front(),d->front(),argL) ;
  else if (arg == "pool") pool_11(rep  ,s->front(),d->front(),argL) ;
  else throw std::runtime_error("not supported option:<"+arg+'>') ;
}

static void invoke_1n(Rpi::Peripheral *rpi,rep_t rep,Ui::ArgL *argL)
{
  auto nwords = Ui::strto<size_t>(argL->pop()) ;
  if (nwords < 2)
    throw Neat::Error("Main:Throughput:invoke:at least two words required") ;
  auto s = locate(rpi,     1,argL) ; 
  auto d = locate(rpi,nwords,argL) ;
  auto arg = argL->pop() ;
  if      (arg == "blck") blck_1n(rep,nwords,s->front(),d->front(),argL) ;
  else if (arg == "iter") iter_1n(rep,nwords,s->front(),d->front(),argL) ;
  else throw std::runtime_error("not supported option:<"+arg+'>') ;
}

static void invoke_n0(Rpi::Peripheral *rpi,rep_t rep,Ui::ArgL *argL)
{
  auto nwords = Ui::strto<size_t>(argL->pop()) ;
  if (nwords < 2)
    throw Neat::Error("Main:Throughput:invoke:at least two words required") ;
  auto d = locate(rpi,nwords,argL) ;
  auto arg = argL->pop() ;
  if      (arg == "blck") blck_n0(rep,nwords,d->front(),argL) ;
  else if (arg == "iter") iter_n0(rep,nwords,d->front(),argL) ;
  else throw std::runtime_error("not supported option:<"+arg+'>') ;
}

static void invoke_n1(Rpi::Peripheral *rpi,rep_t rep,Ui::ArgL *argL)
{
  auto nwords = Ui::strto<size_t>(argL->pop()) ;
  if (nwords < 2)
    throw Neat::Error("Main:Throughput:invoke:at least two words required") ;
  auto s = locate(rpi,nwords,argL) ; 
  auto d = locate(rpi,     1,argL) ;
  auto arg = argL->pop() ;
  if      (arg == "blck") blck_n1(rep,nwords,s->front(),d->front(),argL) ;
  else if (arg == "iter") iter_n1(rep,nwords,s->front(),d->front(),argL) ;
  else throw std::runtime_error("not supported option:<"+arg+'>') ;
}

static void invoke_nn(Rpi::Peripheral *rpi,rep_t rep,Ui::ArgL *argL)
{
  auto nwords = Ui::strto<size_t>(argL->pop()) ;
  if (nwords < 2)
    throw Neat::Error("Main:Throughput:invoke:at least two words required") ;
  auto s = locate(rpi,nwords,argL) ; 
  auto d = locate(rpi,nwords,argL) ;
  auto arg = argL->pop() ;
  if      (arg == "blck") blck_nn(rep,nwords,s->front(),d->front(),argL) ;
  else if (arg == "iter") iter_nn(rep,nwords,s->front(),d->front(),argL) ;
  else if (arg == "libc") libc_nn(rep,nwords,s->front(),d->front(),argL) ;
  else throw std::runtime_error("not supported option:<"+arg+'>') ;
}

// --------------------------------------------------------------------

void invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
  if (argL->empty() || argL->peek() == "help") { 
    std::cout << "arguments : REP MODE\n"
	      << '\n'
	      << "     MODE : 0:1        DST1 ( blck M | iter | pool M )\n"
	      << "          | 0:n N      DSTN ( blck M | iter | libc   )\n"
	      << "          | 1:0   SRC1      ( blck M | iter | pool M )\n"
	      << "          | 1:1   SRC1 DST1 ( blck M | iter | pool M )\n"
	      << "          | 1:n N SRC1 DSTN ( blck M | iter |        )\n"
	      << "          | n:0 N SRCN      ( blck M | iter |        )\n"
	      << "          | n:1 N SRCN DST1 ( blck M | iter |        )\n"
	      << "          | n:n N SRCN DSTN ( blck M | iter | libc   )\n"
	      << '\n'
	      << "      REP : number of repetitions\n"
	      << "        N : number of 32-bit words to transfer\n"
	      << "   blck M : use buffer of M 32-bit words\n"
	      << "   pool M : perform M copy/read/write operations at once\n"
	      << "     libc : Lib-C's memset (0:n) or memcpy (n:n)\n"
	      << '\n'
	      << "SRC1,DST1 : LOCATION1\n"
	      << "SRCN,DSTN : LOCATIONN\n"
	      << '\n'
	      << "LOCATION1 : plain                 # use virtual memory\n"
	      << "          | port PAGE-NO PAGE-IX  # use peripheral address\n"
	      << '\n'
	      << "LOCATIONN : plain                 # use virtual memory\n"
	      << '\n'
	      << "  PAGE-NO : peripheral page number as offset 0..FFF (e.g. 0x200 for GPIO)\n"
	      << "  PAGE-IX : offset within page 0..FFC (e.g. 0x34 for GPIO input levels)\n"
	      << std::flush ;
    return ;
  }

  auto rep = Ui::strto<rep_t>(argL->pop()) ;

  std::string arg = argL->pop() ;
  
  if      (arg == "0:1") invoke_01(rpi,rep,argL) ;
  else if (arg == "0:n") invoke_0n(rpi,rep,argL) ;
  else if (arg == "1:0") invoke_10(rpi,rep,argL) ;
  else if (arg == "1:1") invoke_11(rpi,rep,argL) ;
  else if (arg == "1:n") invoke_1n(rpi,rep,argL) ;
  else if (arg == "n:0") invoke_n0(rpi,rep,argL) ;
  else if (arg == "n:1") invoke_n1(rpi,rep,argL) ;
  else if (arg == "n:n") invoke_nn(rpi,rep,argL) ;
  
  else throw std::runtime_error("not supported option:<"+arg+'>') ; 
}

} /* Throughput */ } /* Main */
