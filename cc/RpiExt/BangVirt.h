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
    Rpi::Counter counter ; Rpi::Gpio gpio ; uint32_t t ;

    Bang(Rpi::Peripheral *rpi) :
	counter(Rpi::Counter(rpi)),
	gpio      (Rpi::Gpio(rpi)),
	t        (counter.clock())
	{}

    struct Command
    {
	virtual void invoke(Bang *bang) = 0 ;
	virtual ~Command() { /* void */ }
	using Ptr = std::unique_ptr<Command> ;
    } ;
    
    struct Levels : Command
    {
	uint32_t *pins ;
	Levels(uint32_t *pins) : pins(pins) {}
	void invoke(Bang *bang) override
	{
	    (*this->pins) = bang->gpio.getLevels() ;
	}
    } ;

    struct Mode : Command
    {
	Rpi::Pin pin ;
	Rpi::Gpio::Mode mode ;
	Mode(Rpi::Pin pin,Rpi::Gpio::Mode mode) : pin(pin), mode(mode) {}
	void invoke(Bang *bang) override
	{
	    bang->gpio.setMode(this->pin,this->mode) ;
	}
    } ;
	    
    struct Recent : Command
    {
	uint32_t *ticks ;
	Recent(uint32_t *ticks) : ticks(ticks) {}
	void invoke(Bang *bang) override
	{
	    (*this->ticks) = bang->t ;
	} ;
    } ;
	    
    struct Reset : Command
    {
	uint32_t pins ;
	Reset(uint32_t pins) : pins(pins) {}
	void invoke(Bang *bang) override
	{
	    bang->gpio.setOutput<Rpi::Gpio::Output::Lo>(this->pins) ;
	}
    } ;
	    
    struct Set : Command
    {
	uint32_t pins ;
	Set(uint32_t pins) : pins(pins) {}
	void invoke(Bang *bang) override
	{
	    bang->gpio.setOutput<Rpi::Gpio::Output::Hi>(this->pins) ;
	}
    } ;
	    
    struct Sleep : Command
    {
	uint32_t span ;
	Sleep(uint32_t span) : span(span) {}
	void invoke(Bang *bang) override
	{
	    if (this->span > 0)
	    {
		auto t0 = bang->t = bang->counter.clock() ;
		while (bang->t - t0 < this->span)
		    bang->t = bang->counter.clock() ;
	    }
	}
    } ;
	    
    struct Time : Command
    {
	uint32_t *ticks ;
	Time(uint32_t *ticks) : ticks(ticks) {}
	void invoke(Bang *bang) override
	{
	    (*this->ticks) = bang->t = bang->counter.clock() ;
	}
    } ;
	    
    struct Wait : Command
    {
	uint32_t const *t0 ;
	uint32_t span ;
	Wait(uint32_t const *t0,uint32_t span) : t0(t0),span(span) {}
	void invoke(Bang *bang) override
	{
	    while (bang->t - (*this->t0) < this->span) 
		bang->t = bang->counter.clock() ;
	}
    } ;

    void execute(std::vector<Command::Ptr> const &v)
    {
	for (auto &c : v)
	    c->invoke(this) ;
    }
    
    struct Enqueue // helper for convenience
    {
	std::deque<Command::Ptr> q ;

	std::vector<Command::Ptr> vector() 
	{
	    std::vector<Command::Ptr> v ;
	    v.reserve(q.size()) ;
	    for (auto &c : q)
		v.push_back(std::move(c)) ;
	    q.clear() ;
	    return v ;
	}

	void levels(uint32_t *pins)
	{
	    this->push<Levels>(pins) ;
	}

	void low(Rpi::Pin pin)
	{
	    this->push<Mode>(pin,Rpi::Gpio::Mode::Out) ;
	}

	void mode(Rpi::Pin pin,Rpi::Gpio::Mode mode)
	{
	    this->push<Mode>(pin,mode) ;
	}

	void off(Rpi::Pin pin)
	{
	    this->push<Mode>(pin,Rpi::Gpio::Mode::In) ;
	}

	void recent(uint32_t *ticks)
	{
	    this->push<Recent>(ticks) ;
	}

	void reset(uint32_t pins) 
	{
	    this->push<Reset>(pins) ;
	}

	void set(uint32_t pins)
	{
	    this->push<Set>(pins) ;
	}

	void sleep(uint32_t span)
	{
	    this->push<Sleep>(span) ;
	}
    
	void time(uint32_t *ticks)
	{
	    this->push<Time>(ticks) ;
	}
    
	void wait(uint32_t const *t0,uint32_t span)
	{
	    this->push<Wait>(t0,span) ;
	}
	
    private:
	
	template<typename T,typename... Args>
	void push(Args&&... args)
	{
	    auto p = new T(std::forward<Args>(args)...) ;
	    this->q.push_back(std::unique_ptr<T>(p)) ;
	}
    } ;
    
} ; }
	
#endif // INCLUDE_RpiExt_Bang_h
