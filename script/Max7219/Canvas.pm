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
# A canvas (actually a 2D area) to draw on. Each spot is either 0 or 1.
#
# -----------+------------------+
#            |        x         | 
# -----------+------------------+---
# (height-1) |                  |
#        ... |      CANVAS      | y
#          1 |       AREA       |
#          0 |                  |
# -----------+------------------+---
#            | 0 1 ... (width-1)
#
# Object construction with make().
#
# Note: this is just a little light demo. It might not work properly 
#   on your system if the dot matrices are aligned differently.
# ---------------------------------------------------------------------

package Canvas ;

use strict ;
use warnings ;

my $pkg = __PACKAGE__ ;

# return a new and empty {width} x {height} canvas 
sub make
{
    my ($class,$w,$h) = @_ ;
    my @Map = map { [(int(0)) x $h] } 1..$w ;
    my $self = { w=>$w,h=>$h,map=>\@Map } ;
    return bless $self,$class ;
}

# return 1 if (x,y) is inside the canvas area
sub isInside
{
    my ($self,$x,$y) = @_ ;
    return 0<=$x && $x<$self->{w} && 0<=$y && $y<$self->{h} ;
}

# return a 2D array relative to the origin (clipped)
sub map
{
    my ($self,$x0,$y0,$w,$h) = @_ ;
    my @Map ;
    for my $x (0..$w-1) {
	my @C = (0) x $h ;
	for my $y (0..$h-1) {
	    $C[$y] = $self->{map}->[$x0+$x]->[$y0+$y] 
		if $self->isInside($x0+$x,$y0+$y) ;
	}
	push @Map,\@C ;
    }
    return \@Map ;
}

# write 1 to (x,y) if inside the canvas area (clipped)
sub plot
{
    my ($self,$x,$y,$on) = @_ ;
    die("binary value expected <$on>")
	unless ($on==0) || ($on==1) ;
    return 0
	unless $self->isInside($x,$y) ;
    $self->{map}->[$x]->[$y] = int($on) ;
    return 1 ;
}

# write $on to all spots in the given area (clipped)
sub fill
{
    my ($self,$x0,$y0,$w,$h,$on) = @_ ;
    for (my $x=$x0 ; $x<$x0+$w ; ++$x) {
	for (my $y=$y0 ; $y<$y0+$h ; ++$y) {
	    $self->plot($x,$y,$on) ;
	}
    }
    return $self ;
}

# fill complete area
sub clear
{
    my ($self,$on) = @_ ;
    $on = 0
	unless defined($on) ;
    $self->fill(0,0,$self->{w},$self->{h},$on) ;
    return $self ;
}

# put the whole canvas area upside-down
sub upsideDown
{
    use integer ;
    my ($self) = @_ ;
    for my $y (0..$self->{h}/2-1) {
	for my $x (0..$self->{w}-1) {
	    my $a = \$self->{map}->[$x]->[             $y] ;
	    my $b = \$self->{map}->[$x]->[$self->{h}-1-$y] ;
	    ($$a,$$b) = ($$b,$$a) ;
	}
    }
    return $self ;
}

# Bresenham's line
sub line
{
    my ($self,$x0,$y0,$x1,$y1,$on) = @_ ;
    my $steep = abs($y1-$y0) > abs($x1-$x0) ;
    if ($steep != 0) {
	($y0,$x0) = ($x0,$y0) ;
	($y1,$x1) = ($x1,$y1) ;
    }
    if ($x0 > $x1) {
	($x1,$x0) = ($x0,$x1) ;
	($y1,$y0) = ($y0,$y1) ;
    }
    my $dx =     $x1-$x0  ;
    my $dy = abs($y1-$y0) ;
    my $error = $dx / 2 ;
    my $y = $y0;
    my $ystep = ($y0<$y1) ? 1 : -1 ;
    for(my $x=$x0 ; $x<=$x1 ; ++$x) {
	if ($steep) { $self->{map}->[$y]->[$x] = $on ;	} 
	else        { $self->{map}->[$x]->[$y] = $on ;	}
	$error -= $dy ;
	if ($error < 0) {
	    $y += $ystep ;
	    $error += $dx ;
	}
    }
    return $self ;
}

1 ;
