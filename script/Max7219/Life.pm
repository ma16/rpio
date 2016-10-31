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

# Note: this is just a little light demo. It's barely tested and might 
#   not work properly.

package Life ;

use strict ;
use warnings ;

my $pkg = __PACKAGE__ ;

sub make
{
    my ($class,$w,$h,$seed) = @_ ;
    $seed = 0
	unless defined($seed) ;
    srand($seed) ;
    my @Map = map { [(map {rand()<0.5} 1..$h)] } 1..$w ;
    my $self = { w=>$w,h=>$h,map=>\@Map } ;
    return bless $self,$class ;
}

sub get
{
    my ($self) = @_ ;
    my @Map = map { [@$_] } @{$self->{map}} ;
    return \@Map ;
}

sub empty
{
    my ($self) = @_ ;
    for my $x (0..$self->{w}-1) {
	for my $y (0..$self->{h}-1) {
	    return 0
		if 0 != $self->{map}->[$x]->[$y] ;
	}
    }
    return 1 ;
}

sub equal
{
    my ($self,$other) = @_ ;
    return 0
	unless defined($other) ;
    for my $x (0..$self->{w}-1) {
	for my $y (0..$self->{h}-1) {
	    return 0
		if $self->{map}->[$x]->[$y] != $other->{map}->[$x]->[$y] ;
	}
    }
    return 1 ;
}

sub hash
{
    use integer ;
    my ($self) = @_ ;
    my $h = 0 ;
    for my $x (0..$self->{w}-1) {
	for my $y (0..$self->{h}-1) {
	    $h = ($h*16777619) ^ $self->{map}->[$x]->[$y] ;
	}
    }
    return $h ;
}

sub next
{
    my ($self) = @_ ;
    my @Map = map { [(0) x  $self->{w}] } 1..$self->{h} ;
    for my $row (0 .. $self->{h}-1) {
	for my $col (0 .. $self->{w}-1) {
	    my $val = 0 ;
	    my @N = ([-1,-1],[-1,0],[-1,1],
		     [ 0,-1],       [ 0,1],
		     [ 1,-1],[ 1,0],[ 1,1]) ;
	    for (@N) {
		my ($x,$y) = @$_ ;
		$x += $col ;
		$y += $row ;
		next
		    unless 0<=$x && $x<$self->{w} && 0<=$y && $y<$self->{h} ;
		$val += $self->{map}->[$x]->[$y] ;
	    }
	    $Map[$col][$row] = ($val == 3) || ($val==2 && $self->{map}->[$col]->[$row]) ;
	}
    }
    my $next = { w=>$self->{w},h=>$self->{h},map=>\@Map } ;
    return bless $next,ref($self) ;
}

