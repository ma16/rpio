// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Ui_ArgL_h_
#define _Ui_ArgL_h_

#include "Error.h"

#include <list>
#include <map>
#include <sstream>
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

    // pop and return list's front (throws if empty)
    void pop(char const* arg)
    {
      auto success = pop_if(arg) ;
      if (!success)
	throw Error(std::string("expect:<")+arg+">") ;
    }

    // return true and pop if {front} eq {arg}
    bool pop_if(char const* arg) ;

    // return arg's position and pop if {front} in {args}
    boost::optional<size_t> pop_if(std::initializer_list<char const*> const &args) ;
    // [opt] make template for N arguments and return Enum<N-1>
    
    // same as above. throws however if {front} is not in {args}
    size_t pop(std::initializer_list<char const*> const &args) ;
    
    template<typename T> T* pop(std::map<std::string,T*> const &map)
    {
	auto arg = this->pop() ;
	auto i = map.find(arg) ;
	if (i != map.end())
	    return i->second ;
	std::ostringstream os ;
	os << "unexpected argument:<" << arg << '>' << " valid arguments are:" ;
	for (auto &pair: map) 
	    os << " <" << pair.first << '>' ;
	throw Error(os.str()) ;
    }
    
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
