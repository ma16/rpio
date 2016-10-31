# Copyright (c) 2016, "ma16". All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions 
# are met:
#
# * Redistributions of source code must retain the above copyright 
#   notice, this list of conditions and the following disclaimer.
# * Redistributions in binary form must reproduce the above copyright 
#   notice, this list of conditions and the following disclaimer in 
#   the documentation and/or other materials provided with the 
#   distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
# HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
# OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED 
# AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
# WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
# POSSIBILITY OF SUCH DAMAGE.
#
# This project is hosted at https://github.com/ma16/rpio

# ---------------------------------------------------------------------
# Draw a character on a Canvas
#
# load(): font object construction
# draw(): single character on canvas (clipped) 
#
# Supports fonts in Bitmap Distribution Format (BDF)
#
# Note that Portable Compiled Format (PCS) fonts (that come with X11) 
# can be converted to BDF with pcf2bdf -- which is available as Raspbian 
# package. E.g.
#
# $ pcf2bdf -o 6x9-ISO8859-15.bdf /usr/share/fonts/X11/misc/6x9-ISO8859-15.pcf.gz
#
# Note: this is just a little light demo. It's barely tested and might 
#   not work properly.
# ---------------------------------------------------------------------

package Font ;

use strict ;
use warnings ;

use Canvas ;
use Parser ;

my $pkg = __PACKAGE__ ;

# create and return object by parsing the given font file (name)
sub load
{
    my ($class,$fname) = @_ ;
    die("open <$fname> failed:$!")
	unless open(my $fh,'<',$fname) ;
    my $self = { fname=>$fname,char=>{} } ;
    my $p = Parser->make($fh) ;
    while (1) {
	my $s = $p->next() ;
	last
	    unless defined($s) ;
	next
	    unless $s eq 'STARTCHAR' ;
	my $name = $p->next() ;
	$p->expect('ENCODING') ; my $code = $p->next() ;
	$p->expect('SWIDTH') ; $p->next() ; $p->next() ;
	$p->expect('DWIDTH') ; $p->next() ; $p->next() ;
	$p->expect('BBX') ; my $width = $p->next() ; my $height = $p->next() ; $p->next() ; $p->next() ;
	$p->expect('BITMAP') ;
	my @L ;
	for my $i (1..$height) {
	    use integer ;
	    my $line = $p->next() ; # scan line, hex, msb first
	    my $v = hex($line) ;
	    my $n = 4 * length($line) ;
	    push (@L,$v>>($n-$width)) ; 
	}
	$p->expect('ENDCHAR') ;
	$self->{char}->{$code} = [$name,int($width),int($height),@L] ;
    }
    return bless $self,$class ;
}

# the character to draw is given by its code number (see ord()). returns 
# the new x coordinate after drawing
sub draw
{
    my ($self,$canvas,$x0,$y0,$code) = @_ ;
    my $r = $self->{char}->{$code} ;
    return $x0
	unless defined($r) ;
    my (undef,$w,$h) = @$r ;
    use integer ;
    my $msb = 1<<($w-1) ;
    for my $y (0..$h-1) {
	my $mask = $r->[3+$y] ;
	for my $x (0..$w-1) {
	    $canvas->plot($x0+$x,$y0+($h-1-$y),1) 
		if $mask & $msb ;
	    $mask <<= 1 ;
	}
    }
    return $x0 + $w ;
}
