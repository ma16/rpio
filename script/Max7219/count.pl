#!/usr/bin/perl

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

# This project is hosted at https://github.com/ma16/rpio

# ---------------------------------------------------------------------
# An ordinary counter 
#
# A text file is generated and written to stdout. This text file can be 
# used as argument for the "rpio" tool in "max7219" mode.
#
# arguments:
#   -- the number of 8x8 dot matrices to be used
#   -- the start value
#   -- the number of iterations
#   -- the delay in seconds between frames
#
# e.g.: $ count.pl 1 0 10 1
#
# The numbers are written left to right, so make sure there is enough 
# space (clipped otherwise).
#
# The font file "6x9-ISO8859-15.bdf" needs to be located in the same
# directory as this perl script.
#
# Note: this is just a little light demo. It's barely tested and might 
#   not work properly.
# ---------------------------------------------------------------------

use strict ;
use warnings ;

use File::Basename qw(dirname) ; use lib dirname($0) ;

use Canvas ;
use Font ;
use Host ;

die("arguments: <matrices> <start> <iterations> <delay>")
    unless 4 == scalar(@ARGV) ;
my ($matrices,$start,$iterations,$delay) = @ARGV ;

my $host = Host->ctor(\*STDOUT,$matrices) ; 
$host->comment("create counter digits for rpio's max7219 mode") ;
$host->comment("for $matrices 8x8 dot matrices start=$start iterations=$iterations delay=$delay") ;
$host->comment("see https://github.com/ma16/rpio") ;
my $intensity = 5 ;
$host->init($intensity) ; 

my $font = Font->load(dirname($0)."/6x9-ISO8859-15.bdf") ;
my $canvas = Canvas->make(8*$matrices,8) ;

for my $i ($start..$start+$iterations-1) {
    use integer ;
    my @L ;
    do {
	push @L,($i%10) ;
	$i /= 10 ;
    }
    while ($i != 0) ;
    my $x0 = 0 ;
    $canvas->clear() ;
    for my $i (reverse(@L)) {
	$x0 = $font->draw($canvas,$x0,0,ord('0')+$i) ;
    }
    $host->display($canvas->map(0,0,8*$matrices,8)) ;
    $host->delay($delay) ;
}
