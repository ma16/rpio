// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef INCLUDE_Protocol_OneWire_Bang_Error_h
#define INCLUDE_Protocol_OneWire_Bang_Error_h

#include <Neat/Error.h>
#include <Neat/Numerator.h>

namespace Protocol { namespace OneWire { namespace Bang { 

struct Error : Neat::Error
{
    enum class Type : unsigned
    {
	NotPresent = 0, // no device found on the bus
	Retry      = 1, // timing exceeded due to process suspension 
	Timing     = 2, // timing exceeded by remote device
	Vanished   = 3, // device vanished while Search-ROM operation
    } ;
  
    using TypeN = Neat::Numerator<Type,Type::Vanished> ;
	
    Error(Type type,int line) ;
	
    Type type() const { return type_ ; }
	
private:
	
    Type type_ ;
    
} ; } } }

#endif // INCLUDE_Protocol_OneWire_Bang_Error_h
