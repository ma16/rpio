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
# A generator for text files that can be used as argument for the "rpio" 
# tool in "max7219" mode.
#
# Object construction:
# -- ctor
#
# The follwoing methods correpond to the MAX7219' work of operating.
# -- latch
# -- shift
# (see also rpio max7219 help)
#
# The following methods provide some sorts of control 
# -- comment
# -- delay
# -- echo
# (see also rpio max7219 help)
#
# The following two methods are for convenience (they take care of some
# of the details)
# --display
# --init
#
#
# Note: this is just a little light demo. It's barely tested and might 
#   not work properly.
# ---------------------------------------------------------------------

package Host ;

use strict ;
use warnings ;
use integer ;

my $pkg = __PACKAGE__ ;

# ---------------------------------------------------------------------

# arguments: file handle, number of 8x8 dot matrices
sub ctor
{
    my ($class,$fh,$n) = @_ ;
    my $self = { fh=>$fh,n=>$n } ;
    return bless $self,$class ;
}

# ---------------------------------------------------------------------

# even though rpio ignores comments, they might be useful for documentation
sub comment
{
    my ($self,$comment) = @_ ;
    $comment =~ s/\n/{LF}/g ;
    printf {$self->{fh}} "# $comment\n" ;
    return $self ;
}

# rpio sleeps for the given amount of seconds
sub delay
{
    my ($self,$seconds) = @_ ;
    die("invalid delay")
	unless $seconds >= 0 ;
    printf {$self->{fh}} "+%f\n",$seconds ;
    return $self ;
}
    
# print the given message to stdout
sub echo
{
    my ($self,$message) = @_ ;
    $message =~ s/"/'/g ;
    printf {$self->{fh}} "\"$message\"\n" ;
    return $self ;
}

# rpio tells MAX7219 to latch (load) current data
sub latch
{
    my ($self) = @_ ;
    printf {$self->{fh}} "!\n" ;
    return $self ;
}

# rpio tells MAX7219 to shift current data by the (hi,lo) word
sub shift
{
    my ($self,$hi,$lo) = @_ ;
    printf {$self->{fh}} ">0x%x%02x",$hi,$lo ;
    return $self ;
}

# ---------------------------------------------------------------------
# Below are calls that correspond to the actual MAX7219 commands
# ---------------------------------------------------------------------

sub nop
{
    my ($self) = @_ ;
    $self->shift(0,0) ;
    return $self ;
}

#         |   Digit:                                |
# Segment |   0    1    2    3    4    5    6    7  | Row
# --------+-----------------------------------------+----
#       G | 1-01 2-01 3-01 4-01 5-01 6-01 7-01 8-01 | 0
#       F | 1-02 2-02 3-02 4-02 5-02 6-02 7-02 8-02 | 1
#       E | 1-04 2-04 3-04 4-04 5-04 6-04 7-04 8-04 | 2
#       D | 1-08 2-08 3-08 4-08 5-08 6-08 7-08 8-08 | 3
#       C | 1-10 2-10 3-10 4-10 5-10 6-10 7-10 8-10 | 4
#       B | 1-20 2-20 3-20 4-20 5-20 6-20 7-20 8-20 | 5
#       A | 1-40 2-40 3-40 4-40 5-40 6-40 7-40 8-40 | 6
#      DP | 1-80 2-80 3-80 4-80 5-80 6-80 7-80 8-80 | 7
# --------+-----------------------------------------+----
#         |   Column                                |
sub digit
{
    my ($self,$col,$row) = @_ ;
    die("invalid column (digit) <$col>")
	unless 0<=$col && $col<8 ;
    die("invalid row (segment) <$row>")
	unless 0<=$row && $row<0x100 ;
    $self->shift($col+1,$row) ;
    return $self ;
}

sub decode
{
    my ($self,$mode) = @_ ;
    die("invalid decode mode")
	unless $mode==0 || $mode==1 || $mode==0x0f || $mode==0xff ;
    $self->shift(9,$mode) ;
    return $self ;
}

sub intensity
{
    my ($self,$intensity) = @_ ;
    die("invalid intensity")
	unless 0<=$intensity && $intensity<16 ;
    $self->shift(10,$intensity) ;
    return $self ;
}

sub scanLimit
{
    my ($self,$limit) = @_ ;
    die("invalid intensity")
	unless 0<=$limit && $limit<8 ;
    $self->shift(11,$limit) ;
    return $self ;
}

sub shutdown
{
    my ($self,$mode) = @_ ;
    die("invalid shutdown value")
	unless $mode==0 || $mode==1 ;
    $self->shift(12,$mode) ;
    return $self ;
}

sub test
{
    my ($self,$on) = @_ ;
    die("invalid test mode")
	unless $on==0 || $on==1 ;
    $self->shift(15,$on) ;
    return $self ;
}

# ---------------------------------------------------------------------

# some initialisation of the MAX7219; intensity 0..15
sub init
{
    my ($self,$intensity) = @_ ;
    for my $i (1..$self->{n}) { $self->test(0)               ; } $self->latch() ;
    for my $i (1..$self->{n}) { $self->shutdown(0)           ; } $self->latch() ;
    for my $i (1..$self->{n}) { $self->decode(0)             ; } $self->latch() ;
    for my $i (1..$self->{n}) { $self->intensity($intensity) ; } $self->latch() ;
    for my $i (1..$self->{n}) { $self->scanLimit(7)          ; } $self->latch() ;
    for my $x (0..7) {
	for my $i (1..$self->{n}) { $self->digit($x,0) ; }       $self->latch() ;
    }
    for my $i (1..$self->{n}) { $self->shutdown(1)  ; } $self->latch() ;
    return $self ;
}

# ---------------------------------------------------------------------

# the given 2D array will be displayed on the MAX7219. the array must 
# have a size of [columns] x [rows] = [8*n][8] where each field holds a 
# value of either 0 or 1; n is the number of dot matrices. columns
# start on the left and go right. Rows start a the bottom and go up.
sub display
{
    my ($self,$map) = @_ ;
    for my $x (0..7) {
	for my $i (reverse(0..$self->{n}-1)) {
	    my $value = 0 ;
	    for (my $y=0 ; $y<8 ; ++$y) {
		$value <<= 1 ;
		$value += $map->[8*$i+$x]->[$y] ;
	    }
	    $self->digit($x,$value) ;
	}
	$self->latch() ;
    }
    return $self ;
}

# ---------------------------------------------------------------------

1 ;
