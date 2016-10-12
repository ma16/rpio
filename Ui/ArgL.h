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

#ifndef _Ui_ArgL_h_
#define _Ui_ArgL_h_

#include "Error.h"

#include <list>
#include <string>
#include <boost/optional.hpp>

namespace Ui
{
  struct ArgL
  {
    struct Error : Ui::Error
    {
      Error(std::string const &s) : Ui::Error("ArgL:" + s) {}
    } ;

    static ArgL make(int argc,char *argv[]) ;
    // ...[opt] return smart pointer instead

    // return true if empty
    bool empty() const ;

    // throw if list is not empty to finalize argument processing
    void finalize() const ;

    // return list's front (throws if empty)
    std::string const& peek() const ;

    // pop and return list's front (throws if empty)
    std::string pop() ;

    // return true and pop if {front} eq {arg}
    bool pop_if(char const* arg) ;

    // return arg's position and pop if {front} in {args}
    boost::optional<size_t> pop_if(std::initializer_list<char const*> const &args) ;
    // [opt] make template for N arguments and return Enum<N-1>
    
    // same as above. throws however if {front} is not in {args}
    size_t pop(std::initializer_list<char const*> const &args) ;
    
    // if {arg} == {front} return the next value and pop both (throws if no next value)
    boost::optional<std::string> option(std::string const &arg) ;

    // same as above. however, return {def} if {arg} != {front}
    std::string option(std::string const &arg,std::string const &def) ;

    // if {arg} == {front} return {F(next)} and pop both (throws if no next value)
    template<typename T,T (*F)(std::string const&)> boost::optional<T> option(std::string const &arg) 
    {
      auto o = option(arg) ;
      if (!o)
	return boost::none ;
      return F(*o) ;
    }

  private:

    ArgL(std::list<std::string> &&list) : list(std::move(list)) {}

    std::list<std::string> list ;
    // ...[opt] use deque

  } ;
}

#endif // _Ui_ArgL_h_
