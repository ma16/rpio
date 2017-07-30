// BSD 2-Clause License, see github.com/ma16/rpio

// "join<a,b,c,...>" does exactly the same as "a | b | c | ..."
// however, join() aborts if there is an overlap of the values

#ifndef INCLUDE_Neat_join_h
#define INCLUDE_Neat_join_h

namespace Neat
{
    template<uint32_t head,uint32_t... tail> struct join_Helper
    {
	static constexpr uint32_t invoke()
	{
	    static_assert((head & join_Helper<tail...>::invoke()) == 0,"") ;
	    return head | join_Helper<tail...>::invoke() ;
	}
    } ;

    template<uint32_t tail> struct join_Helper<tail>
    {
	static constexpr uint32_t invoke()
	{
	    return tail ;
	}
    } ;

    template<uint32_t... args> static constexpr uint32_t join()
    {
	return join_Helper<args...>::invoke() ;
    }
}

#endif // INCLUDE_Neat_join_h
