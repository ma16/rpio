// BSD 2-Clause License, see github.com/ma16/rpio

// --------------------------------------------------------------------
// An interface for bit-banging (that is supposed to be generic and
// hide as many details of the Raspberry Pi as possible).
//
// Levels: get pin level
// Mode:   pin function mode
// Recent: get last recorded timer tick
// Reset:  set GPIO pin level to Low
// Set:    set GPIO pin level to High
// Sleep:  sleep for a number of ticks based on current time
// Time:   query time
// Wait:   sleep for a number of ticks based on given time
//
// The ARM counter is used as time-reference.
//
// The client is responsible for setup (i.e. ARM counter and
// GPIO input/output mode).
// --------------------------------------------------------------------

#ifndef INCLUDE_RpiExt_Bang_h
#define INCLUDE_RpiExt_Bang_h

#include <assert.h>
#include <deque>
#include <vector>
#include <Rpi/Counter.h>
#include <Rpi/Gpio.h>

namespace RpiExt {

struct Bang
{
    struct Command
    {
	enum class Choice { Levels,Mode,Recent,Reset,Set,Sleep,Time,Wait } ;
	
	Choice choice ;
	
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
	    Rpi::Gpio::Mode mode ;
	    Mode(Rpi::Pin pin,Rpi::Gpio::Mode mode) : pin(pin), mode(mode) {}
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
	    
	union Value
	{
	private:
	    
	    friend Bang ;
	    
	    Levels levels ;
	    Mode     mode ;
	    Recent recent ;
	    Reset   reset ;
	    Set       set ;
	    Sleep   sleep ;
	    Time     time ;
	    Wait     wait ;
	    
	    Value(Levels const& levels) : levels(levels) {}
	    Value(Mode   const&   mode) :   mode  (mode) {}
	    Value(Recent const& recent) : recent(recent) {}
	    Value(Reset  const&  reset) : reset  (reset) {}
	    Value(Set    const&    set) : set      (set) {}
	    Value(Sleep  const&  sleep) : sleep  (sleep) {}
	    Value(Time   const&   time) : time    (time) {}
	    Value(Wait   const&   wait) : wait    (wait) {}
	} ;
	
	Value value ;

	Command(Choice choice,Value value)
	    : choice(choice),value(value) {}

	static Command levels(uint32_t *pins)
	{
	    return Command(Choice::Levels,Levels(pins)) ;
	}

	static Command mode(Rpi::Pin pin,Rpi::Gpio::Mode mode)
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
    } ;

    Bang(Rpi::Peripheral *rpi) :
      counter(Rpi::Counter(rpi)),
      gpio(Rpi::Gpio(rpi)),
      t(counter.clock())
    {}

    void execute(Command const &c)
    {
	switch (c.choice)
	{
	case Command::Choice::Levels : levels(c.value.levels) ; break ;
	case Command::Choice::Mode   :   mode(c.value.  mode) ; break ;
	case Command::Choice::Recent : recent(c.value.recent) ; break ;
	case Command::Choice::Reset  :  reset(c.value. reset) ; break ;
	case Command::Choice::Set    :    set(c.value.   set) ; break ;
	case Command::Choice::Sleep  :  sleep(c.value. sleep) ; break ;
	case Command::Choice::Time   :   time(c.value.  time) ; break ;
	case Command::Choice::Wait   :   wait(c.value.  wait) ; break ;
	default: assert(false) ; abort() ;
	}
    }
    
    template<typename InputIt> void execute(InputIt first, InputIt recent)
    {
	for (InputIt i=first ; i!=recent ; ++i)
	    this->execute(*i) ;
    }
    
    struct Enqueue // helper for convenience
    {
	std::deque<Command> q ;

	std::vector<Command> vector() const
	{
	    return std::vector<Command>(q.begin(),q.end()) ;
	}
	
	void levels(uint32_t *pins)
	{
	    q.push_back(Command::levels(pins)) ;
	}

	void low(Rpi::Pin pin)
	{
	    q.push_back(Command::reset(1u<<pin.value())) ; // [todo] drop
	    q.push_back(Command::mode(pin,Rpi::Gpio::Mode::Out)) ;
	}

	void mode(Rpi::Pin pin,Rpi::Gpio::Mode mode)
	{
	    q.push_back(Command::mode(pin,mode)) ;
	}

	void off(Rpi::Pin pin)
	{
	    q.push_back(Command::mode(pin,Rpi::Gpio::Mode::In)) ;
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
    } ;
  
private:

    Rpi::Counter counter ; Rpi::Gpio gpio ; uint32_t t ;

    void levels(Command::Levels const &c)
    {
	(*c.pins) = this->gpio.getLevels() ;
    }

    void mode(Command::Mode const &c)
    {
	this->gpio.setMode(c.pin,c.mode) ;
    }

    void reset(Command::Reset const &c)
    {
	this->gpio.setOutput<Rpi::Gpio::Output::Lo>(c.pins) ;
    }
    
    void set(Command::Set const &c)
    {
	this->gpio.setOutput<Rpi::Gpio::Output::Hi>(c.pins) ;
    }

    void time(Command::Time const &c)
    {
	(*c.ticks) = this->t = this->counter.clock() ;
    }

    void recent(Command::Recent const &c)
    {
	(*c.ticks) = this->t ;
    }

    void wait(Command::Wait const &c)
    {
	while (this->t - (*c.t0) < c.span) 
	    this->t = this->counter.clock() ;
    }
    
    void sleep(Command::Sleep const &c)
    {
	auto t0 = this->t = this->counter.clock() ;
	while (this->t - t0 < c.span)
	    this->t = this->counter.clock() ;
    }

} ; }
	
#endif // INCLUDE_RpiExt_Bang_h
