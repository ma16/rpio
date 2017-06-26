// BSD 2-Clause License, see github.com/ma16/rpio

#include "ArgL.h"
#include <sstream>

bool Ui::ArgL::empty() const
{
  return this->list.empty() ;
}

void Ui::ArgL::finalize() const
{
  if (!this->empty()) {
    std::ostringstream os ;
    os << "unexpected arguments:" ;
    for (auto &s: this->list)
      os << " <" << s << '>' ;
    throw Error(os.str()) ;
  }
}

Ui::ArgL Ui::ArgL::make(int argc,char *argv[])
{
  std::list<std::string> l ;
  for (int i=0 ; i<argc ; ++i) {
    l.push_back(argv[i]) ;
  }
  return Ui::ArgL(std::move(l)) ;
}

boost::optional<std::string> Ui::ArgL::option(std::string const& arg)
{
  if (this->empty())
    return boost::none ;
  if (this->peek() != arg)
    return boost::none ;
  this->pop() ;
  if (this->empty())
    throw Error("missing argument for option:<" + arg + '>') ;
  return this->pop() ;
}

std::string Ui::ArgL::option(std::string const& arg,std::string const &def) 
{
  auto option = this->option(arg) ;
  return option ? (*option) : def ;
}

std::string const& Ui::ArgL::peek() const
{
  if (this->list.empty())
    throw Error("missing argument") ;
  return this->list.front() ;
}

std::string Ui::ArgL::pop()
{
  if (this->list.empty())
    throw Error("missing argument") ;
  std::string arg = this->list.front() ;
  this->list.pop_front() ;
  return arg ;
}

size_t Ui::ArgL::pop(std::initializer_list<char const*> const &args) 
{
  auto n = this->pop_if(args) ;
  if (n) return (*n) ;
  std::ostringstream os ;
  if (this->empty())
    os << "missing argument" ;
  else
    os << "unexpected argument:<" + this->peek() << '>' ;
  os <<  " valid arguments are: " ;
  for (auto &arg: args) {
    os << " <" << arg << '>' ;
  }
  throw Error(os.str()) ;
}

bool Ui::ArgL::pop_if(char const* arg) 
{
  if (this->empty())
    return false ;
  if (this->peek() != arg)
    return false ;
  this->pop() ;
  return true ;
}

boost::optional<size_t> Ui::ArgL::pop_if(std::initializer_list<char const*> const &args) 
{
  if (this->empty())
    return boost::none ;
  auto &front = this->peek() ;
  size_t i = 0 ;
  for (auto &arg: args) {
    if (front == arg) {
      this->pop() ;
      return i ;
    }
    ++i ;
  }
  return boost::none ;
}
