// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef Console_Spi0_Mosi_h
#define Console_Spi0_Mosi_h

#include <Ui/ArgL.h>
#include <vector>

namespace Console { namespace Spi0 {

struct Mosi
{
    using V = std::vector<uint8_t> ;
    
    Mosi(V const &v) : v(v) {}

    V::size_type nbytes() const { return v.size() ; }

    V::const_iterator begin() const { return v.begin() ; }
    V::const_iterator   end() const { return v.  end() ; }

    struct Sequence
    {
	using V = std::vector<Mosi> ;
    
	static Sequence make(Ui::ArgL *argL) ;
    
	V::const_iterator begin() const { return v.begin() ; }
	V::const_iterator   end() const { return v.  end() ; }
    
	V::size_type nitems() const { return v.size() ; }
	V::size_type nbytes() const { return  nbytes_ ; } 
	// ...sum of: each item length rounded up to a multiple of 4

    private:

	static size_t count(V const &v) ;

	Sequence(V const &v) : v(v),nbytes_(count(v)) {}
    
	V v ; size_t nbytes_ ;
    } ;

private:
    
    V v ;
} ;

} }

#endif // Console_Spi0_Mosi_h
