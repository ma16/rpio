// BSD 2-Clause License, see github.com/ma16/rpio

#include "Cm.h"

constexpr Rpi::Peripheral::PNo Rpi::Cm::PNo ;

void Rpi::Cm::enable(Alias alias) 
{
    auto c = this->ctl(alias).read() ;
    c %= Ctl::Enab::make(1) ;
    c %= Ctl::Kill::make(0) ;
    this->ctl(alias).write(c) ;
}

void Rpi::Cm::disable(Alias alias) 
{
    auto c = this->ctl(alias).read() ;
    c %= Ctl::Enab::make(0) ;
    this->ctl(alias).write(c) ;
    // the generator might still be busy (for a while)
}

void Rpi::Cm::kill(Alias alias) 
{ 
    auto c = this->ctl(alias).read() ;
    c %= Ctl::Kill::make(1) ;
    this->ctl(alias).write(c) ;
    while (busy(alias))
	;
    c %= Ctl::Kill::make(0) ;
    this->ctl(alias).write(c) ;
} 

void Rpi::Cm::set(Alias      alias,
		  Ctl::Src     src,
		  Div::Intgr intgr,
		  Div::Fract fract,
		  Ctl::Mash   mash)
{
    auto c = this->ctl(alias).read() ;
    // disable the generator if currently enabled
    auto e = c.test(Ctl::Enab::Digit) ;
    if (e)
    {
	c %= Ctl::Enab::make(0) ;
	this->ctl(alias).write(c) ;
	while (busy(alias))
	    ;
    }
    // set new value w/o enabling
    c %= src ;
    c %= mash ;
    this->ctl(alias).write(c) ;
    auto d = div(alias).read() ;
    d %= intgr ;
    d %= fract ;
    div(alias).write(d) ;
    // enable the generator if it was enabled in the first place
    if (e)
    {
	c %= Ctl::Enab::make(1) ;
	this->ctl(alias).write(c) ;
    }
}
