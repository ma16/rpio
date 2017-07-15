// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef INCLUDE_Device_Max7219_Parser_h
#define INCLUDE_Device_Max7219_Parser_h

#include <Neat/Error.h>
#include <Ui/strto.h>
#include <deque>
#include <memory> // shared_ptr

namespace Device { namespace Max7219 {

struct Parser
{
    struct Error : Neat::Error
    {
	Error(std::string const &s) : Neat::Error("Device::Max7219::Parser:"+s) {}
    } ;

    struct Command
    {
	using shared_ptr = std::shared_ptr<Command> ;
	virtual ~Command() {}
    } ;

    // there is boost::variant; instead of using inheritance for the
    // parser's tokens. however, just including <boost/variant.hpp>
    // results here in lots of warnings [libboost1.55-dev:armhf]:
    // "warning: conversion to ‘int’ from ‘unsigned int’"

    struct Delay : Command
    {
	double seconds ;
	Delay(double seconds) : seconds(seconds) {}
    } ;
    
    struct Echo : Command
    {
	std::string text ;
	Echo (std::string const &text) : text(text) {}
    } ;
    
    struct Eof : Command {} ;
    
    struct Latch : Command {} ;
    
    struct Shift : Command
    {
	uint16_t data ;
	Shift (uint16_t data) : data (data) {}
    } ;

    Command::shared_ptr parse() ;

    Parser(std::istream *is) : is(is) {}
  
private:

    std::istream *is ; unsigned row=1 ; unsigned col=0 ;

    void throw_error(std::string const &s) ;

    int next_char() ;
  
    std::string scan(char const *set) ;

    void                parse_comment() ;
    Command::shared_ptr parse_delay() ;
    Command::shared_ptr parse_echo() ;
    Command::shared_ptr parse_shift() ;
  
} ; } }

#endif // INCLUDE_Device_Max7219_Parser_h
    
