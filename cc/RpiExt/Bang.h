// BSD 2-Clause License, see github.com/ma16/rpio

// --------------------------------------------------------------------
// An interface for bit-banging (that is supposed to be generic and
// hide as many details of the Raspberry Pi as possible). Actually,
// this is more like a "script" generator for bit-banging.
//
// The Free Running ARM Counter is used as time-reference.
//
// The client is responsible for setup (i.e. ARM counter and
// possibly GPIO input/output mode).
// --------------------------------------------------------------------

#ifndef INCLUDE_RpiExt_Bang_h
#define INCLUDE_RpiExt_Bang_h

#include <assert.h>
#include <deque>
#include <vector>
#include <Rpi/ArmTimer.h>
#include <Rpi/GpioOld.h>
#include <Rpi/Gpio/Function.h>
#include <Rpi/Gpio/Input.h>
#include <Rpi/Gpio/Output.h>

namespace RpiExt {

struct Bang
{
    struct Command
    {
	enum class Choice
	{
	    Assume,     // stop if condition isn't true
	    Compare,    // execute comparison
	    Duration,   // save the difference of two time-stamps
	    Levels,     // get pin level
	    Mode,       // set pin function mode
	    Recent,     // get last recorded timer tick
	    Reset,      // set GPIO pin level to Low
	    Set,        // set GPIO pin level to High
	    Sleep,      // sleep for a number of ticks based on current time
	    Time,       // query time
	    Wait,       // sleep for a number of ticks based on given time
	    WaitFor,    // wait for a signal level
        } ;
	
	Choice choice ;
	
	enum class Op { Eq,Ge,Gt,Le,Lt,Ne } ;
	
	class Assume
	{
	    friend Command ;
	    friend Bang ;
	    uint32_t const *x ;
	    Op op ;
	    uint32_t y ;
	    unsigned error ;
	    Assume(uint32_t const*x,Op op,uint32_t y,unsigned error)
		: x(x),op(op),y(y),error(error) {}
	} ;
	    
	class Compare
	{
	    friend Command ;
	    friend Bang ;
	    uint32_t const *x ;
	    Op op ;
	    uint32_t y ;
	    bool *success ;
	    Compare(uint32_t const*x,Op op,uint32_t y,bool *success)
		: x(x),op(op),y(y),success(success) {}
	} ;
	    
	class Duration
	{
	    friend Command ;
	    friend Bang ;
	    uint32_t const *t0 ;
	    uint32_t const *t1 ;
	    uint32_t     *span ;
	    Duration(uint32_t *t0,uint32_t *t1,uint32_t *span)
		: t0(t0),t1(t1),span(span) {}
	} ;
	    
	class Levels
	{
	    friend Command ;
	    friend Bang ;
	    uint32_t *pins ;
	    Levels(uint32_t *pins) : pins(pins) {}
	} ;
	    
	class Mode
	{
	    friend Command ;
	    friend Bang ;
	    Rpi::Pin pin ;
	    Rpi::Gpio::Function::Mode mode ;
	  Mode(Rpi::Pin pin,Rpi::Gpio::Function::Mode mode) : pin(pin), mode(mode) {}
	} ;
	    
	class Recent
	{
	    friend Command ;
	    friend Bang ;
	    uint32_t *ticks ;
	    Recent(uint32_t *ticks) : ticks(ticks) {}
	} ;
	    
	class Reset
	{
	    friend Command ;
	    friend Bang ;
	    uint32_t pins ;
	    Reset(uint32_t pins) : pins(pins) {}
	} ;
	    
	class Set
	{
	    friend Command ;
	    friend Bang ;
	    uint32_t pins ;
	    Set(uint32_t pins) : pins(pins) {}
	} ;
	    
	class Sleep
	{
	    friend Command ;
	    friend Bang ;
	    uint32_t span ;
	    Sleep(uint32_t span) : span(span) {}
	} ;
	    
	class Time
	{
	    friend Command ;
	    friend Bang ;
	    uint32_t *ticks ;
	    Time(uint32_t *ticks) : ticks(ticks) {}
	} ;
	    
	class Wait
	{
	    friend Command ;
	    friend Bang ;
	    uint32_t const *t0 ;
	    uint32_t span ;
	    Wait(uint32_t const *t0,uint32_t span) : t0(t0),span(span) {}
	} ;
	    
	class WaitFor
	{
	    friend Command ;
	    friend Bang ;
	    uint32_t const *t0 ;
	    uint32_t span ;
	    uint32_t mask ;
	    uint32_t cond ;
	    uint32_t *t1 ;
	    WaitFor(uint32_t const *t0,
		    uint32_t      span,
		    uint32_t      mask,
		    uint32_t      cond,
		    uint32_t       *t1)
		: t0(t0),span(span),mask(mask),cond(cond),t1(t1) {}
	} ;
	    
	union Value
	{
	private:
	    
	    friend Bang ;
	    
	    Assume         assume ;
	    Compare       compare ;
	    Duration     duration ;
	    Levels         levels ;
	    Mode             mode ;
	    Recent         recent ;
	    Reset           reset ;
	    Set               set ;
	    Sleep           sleep ;
	    Time             time ;
	    Wait             wait ;
	    WaitFor       waitFor ;
	    
	    Value(Assume     const&     assume) : assume        (assume) {}
	    Value(Compare    const&    compare) : compare      (compare) {}
	    Value(Duration   const&   duration) : duration    (duration) {}
	    Value(Levels     const&     levels) : levels        (levels) {}
	    Value(Mode       const&       mode) : mode            (mode) {}
	    Value(Recent     const&     recent) : recent        (recent) {}
	    Value(Reset      const&      reset) : reset          (reset) {}
	    Value(Set        const&        set) : set              (set) {}
	    Value(Sleep      const&      sleep) : sleep          (sleep) {}
	    Value(Time       const&       time) : time            (time) {}
	    Value(Wait       const&       wait) : wait            (wait) {}
	    Value(WaitFor    const&    waitFor) : waitFor      (waitFor) {}
	} ;
	
	Value value ;

	Command(Choice choice,Value value)
	    : choice(choice),value(value) {}

	static Command assume(uint32_t const* x,
			      Command::Op    op,
			      uint32_t        y,
			      unsigned    error)
	{
	    return Command(Choice::Assume,Assume(x,op,y,error)) ;
	}

	static Command compare(uint32_t const* x,
			       Command::Op    op,
			       uint32_t        y,
			       bool     *success)
	{
	    return Command(Choice::Compare,Compare(x,op,y,success)) ;
	}

	static Command duration(uint32_t *t0,uint32_t *t1,uint32_t *span)
	{
	    return Command(Choice::Duration,Duration(t0,t1,span)) ;
	}

	static Command levels(uint32_t *pins)
	{
	    return Command(Choice::Levels,Levels(pins)) ;
	}

	static Command mode(Rpi::Pin pin,Rpi::Gpio::Function::Mode mode)
	{
	    return Command(Choice::Mode,Mode(pin,mode)) ;
	}

	static Command recent(uint32_t *ticks)
	{
	    return Command(Choice::Recent,Recent(ticks)) ;
	}
    
	static Command reset(uint32_t pins) 
	{
	    return Command(Choice::Reset,Reset(pins)) ;
	}

	static Command set(uint32_t pins)
	{
	    return Command(Choice::Set,Set(pins)) ;
	}

	static Command sleep(uint32_t span)
	{
	    return Command(Choice::Sleep,Sleep(span)) ;
	}

	static Command time(uint32_t *ticks)
	{
	    return Command(Choice::Time,Time(ticks)) ;
	}
    
	static Command wait(uint32_t const *t0,uint32_t span)
	{
	    return Command(Choice::Wait,Wait(t0,span)) ;
	}

	static Command waitFor(uint32_t const *t0,
			       uint32_t      span,
			       uint32_t      mask,
			       uint32_t      cond,
			       uint32_t       *t1)
	{
	    return Command(Choice::WaitFor,WaitFor(t0,span,mask,cond,t1)) ;
	}
	
    } ;

    Bang(Rpi::Peripheral *rpi)
    : timer         (Rpi::ArmTimer(rpi))
    , gpio           (Rpi::GpioOld(rpi))
    , function(Rpi::Gpio::Function(rpi))
    , input   (Rpi::Gpio::   Input(rpi))
    , output  (Rpi::Gpio::  Output(rpi))
    , t         (timer.counter().read())
    {}

    void execute(Command const &c)
    {
	using Choice = Command::Choice ;
	switch (c.choice)
	{
	case Choice::Assume     :     assume(c.value.    assume) ; break ;
	case Choice::Compare    :    compare(c.value.   compare) ; break ;
	case Choice::Duration   :   duration(c.value.  duration) ; break ;
	case Choice::Levels     :     levels(c.value.    levels) ; break ;
	case Choice::Mode       :       mode(c.value.      mode) ; break ;
	case Choice::Recent     :     recent(c.value.    recent) ; break ;
	case Choice::Reset      :      reset(c.value.     reset) ; break ;
	case Choice::Set        :        set(c.value.       set) ; break ;
	case Choice::Sleep      :      sleep(c.value.     sleep) ; break ;
	case Choice::Time       :       time(c.value.      time) ; break ;
	case Choice::Wait       :       wait(c.value.      wait) ; break ;
	case Choice::WaitFor    :    waitFor(c.value.   waitFor) ; break ;
	default: assert(false) ; abort() ;
	}
    }
    
    unsigned execute(std::vector<Command> const &v)
    {
	this->error = 0 ;
	for (auto const &c: v)
	{
	    this->execute(c) ;
	    if (this->error != 0)
		return this->error ;
	}
	return 0 ;
    }

    struct Stack
    {
	Stack(size_t n) : ss(n),sp(0),stack(new uint32_t[n]) {} 
	
	size_t ss,sp ; std::unique_ptr<uint32_t[]> stack ; 

	uint32_t* push()
	{
	    assert(sp < ss) ;
	    return stack.get() + sp++ ;
	} 

	void pop(size_t n=1)
	{
	    assert(sp >= n) ;
	    sp -= n ;
	}

	uint32_t save() const
	{
	    return sp ;
	}

	void recover(uint32_t sp)
	{
	    this->sp = sp ;
	}
    } ;
    
    struct Enqueue // helper for convenience
    {
	std::deque<Command> q ;

	std::vector<Command> vector() const
	{
	    return std::vector<Command>(q.begin(),q.end()) ;
	}
	
	void assume(uint32_t const *x,
		    Command::Op    op,
		    uint32_t        y,
		    unsigned    error)
	{
	    q.push_back(Command::assume(x,op,y,error)) ;
	}

	void compare(uint32_t const *x,
		     Command::Op    op,
		     uint32_t        y,
		     bool     *success)
	{
	    q.push_back(Command::compare(x,op,y,success)) ;
	}

	void duration(uint32_t *t0,uint32_t *t1,uint32_t *span)
	{
	    q.push_back(Command::duration(t0,t1,span)) ;
	}

	void levels(uint32_t *pins)
	{
	    q.push_back(Command::levels(pins)) ;
	}

	void low(Rpi::Pin pin)
	{
	    q.push_back(Command::reset(1u<<pin.value())) ; // [todo] drop
	    q.push_back(Command::mode(pin,Rpi::Gpio::Function::Mode::Out)) ;
	}

	void mode(Rpi::Pin pin,Rpi::Gpio::Function::Mode mode)
	{
	    q.push_back(Command::mode(pin,mode)) ;
	}

	void off(Rpi::Pin pin)
	{
	    q.push_back(Command::mode(pin,Rpi::Gpio::Function::Mode::In)) ;
	}

	void recent(uint32_t *ticks)
	{
	    q.push_back(Command::recent(ticks)) ;
	}

	void reset(uint32_t pins) 
	{
	    q.push_back(Command::reset(pins)) ;
	}

	void set(uint32_t pins)
	{
	    q.push_back(Command::set(pins)) ;
	}

	void sleep(uint32_t span)
	{
	    q.push_back(Command::sleep(span)) ;
	}
    
	void time(uint32_t *ticks)
	{
	    q.push_back(Command::time(ticks)) ;
	}
    
	void wait(uint32_t const *t0,uint32_t span)
	{
	    q.push_back(Command::wait(t0,span)) ;
	}

	void waitFor(uint32_t const *t0,
		     uint32_t      span,
		     Rpi::Pin       pin,
		     bool          high,
		     uint32_t       *t1)
	{
	    auto mask = 1u<<pin.value() ;
	    auto cond = high ? mask : 0u ;
	    q.push_back(Command::waitFor(t0,span,mask,cond,t1)) ;
	}
	
	void waitFor(uint32_t const *t0,
		     uint32_t      span,
		     uint32_t      mask,
		     uint32_t      cond,
		     uint32_t       *t1)
	{
	    q.push_back(Command::waitFor(t0,span,mask,cond,t1)) ;
	}
    } ;
  
private:

    Rpi::ArmTimer timer ; Rpi::GpioOld gpio ;

    Rpi::Gpio::Function function ;
    Rpi::Gpio::Input       input ;
    Rpi::Gpio::Output     output ;

    uint32_t t ; // last read time-stamp
    uint32_t l ; // last read GPIO level

    unsigned error ; // todo: provide script line & error message

    void assume(Command::Assume const &c)
    {
	switch (c.op)
	{
	case Command::Op::Eq: if ((*c.x) == c.y) return ; break ;
	case Command::Op::Ge: if ((*c.x) >= c.y) return ; break ;
	case Command::Op::Gt: if ((*c.x) >  c.y) return ; break ;
	case Command::Op::Le: if ((*c.x) <= c.y) return ; break ;
	case Command::Op::Lt: if ((*c.x) <  c.y) return ; break ;
	case Command::Op::Ne: if ((*c.x) != c.y) return ; break ;
	}
	this->error = c.error ;
    }

    void compare(Command::Compare const &c)
    {
	switch (c.op)
	{
	case Command::Op::Eq: (*c.success) = (*c.x) == c.y ; break ;
	case Command::Op::Ge: (*c.success) = (*c.x) >= c.y ; break ;
	case Command::Op::Gt: (*c.success) = (*c.x) >  c.y ; break ;
	case Command::Op::Le: (*c.success) = (*c.x) <= c.y ; break ;
	case Command::Op::Lt: (*c.success) = (*c.x) <  c.y ; break ;
	case Command::Op::Ne: (*c.success) = (*c.x) != c.y ; break ;
	}
    }

    void duration(Command::Duration const &c)
    {
	(*c.span) = (*c.t1) - (*c.t0) ;
    }

    void levels(Command::Levels const &c)
    {
	(*c.pins) = this->input.bank0().read() ;
    }

    void mode(Command::Mode const &c)
    {
	this->function.set(c.pin,c.mode) ;
    }

    void reset(Command::Reset const &c)
    {
	this->output.clear().write(c.pins) ;
    }
    
    void set(Command::Set const &c)
    {
	this->output.raise().write(c.pins) ;
    }

    void time(Command::Time const &c)
    {
	(*c.ticks) = this->t = this->timer.counter().read() ;
    }

    void recent(Command::Recent const &c)
    {
	(*c.ticks) = this->t ;
    }

    void sleep(Command::Sleep const &c)
    {
	if (c.span > 0)
	{
	    auto t0 = this->t = this->timer.counter().read() ;
	    while (this->t - t0 < c.span)
		this->t = this->timer.counter().read() ;
	}
    }

    void wait(Command::Wait const &c)
    {
	while (this->t - (*c.t0) < c.span) 
	    this->t = this->timer.counter().read() ;
    }
    
    void waitFor(Command::WaitFor const &c)
    {
	do
	{
	    this->l = this->input.bank0().read() ;
	    if (c.cond == (this->l & c.mask))
	    {
		(*c.t1) = this->t ;
		this->t = this->timer.counter().read() ;
		return ;
	    }
	    this->t = this->timer.counter().read() ;
	}
	while (this->t - (*c.t0) <= c.span) ;
    }
    
} ; }
	
#endif // INCLUDE_RpiExt_Bang_h
