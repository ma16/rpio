// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef INCLUDE_Device_Ds18x20_Bang_OneWire_Error_h
#define INCLUDE_Device_Ds18x20_Bang_OneWire_Error_h

#include <Neat/Error.h>
#include <Neat/Numerator.h>

namespace Device { namespace Ds18x20 { namespace Bang { namespace OneWire {

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
} ; } } } }

#endif // INCLUDE_Device_Ds18x20_Bang_OneWire_Error_h
