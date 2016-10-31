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
# A Game Of Life
#
# A text file is generated and written to stdout. This text file can be 
# used as argument for the "rpio" tool in "max7219" mode.
#
# arguments:
#   -- the number of 8x8 dot matrices to be used
#   -- the number of iterations
#   -- the random seed to start with
#
# e.g.: $ life.pl 1 116 1002747
#   or: $ life.pl 5 383 1001621
#
# Note: this is just a little light demo. It's barely tested and might 
#   not work properly.
# ---------------------------------------------------------------------

use strict ;
use warnings ;

use File::Basename qw(dirname) ; use lib dirname($0) ;

use Host ;
use Life ;

die("arguments: <matrices> <iterations> <seed>")
    unless 3 == scalar(@ARGV) ;
my ($matrices,$iterations,$seed) = @ARGV ;

my $t1 = 1.0 ; # delay between frames
my $t2 = 0.1 ; # delay after last frame

my $host = Host->ctor(\*STDOUT,$matrices) ; 
$host->comment("create game of life patterns for rpio's max7219 mode") ;
$host->comment("for $matrices 8x8 dot matrices") ;
$host->comment("see https://github.com/ma16/rpio") ;

my %H ;
# ...keep track of positions to detect cycles
my $life = Life->make($matrices*8,8,$seed) ; 
# ...width x height
my ($start,$i) = (0,0) ;
# ...count iterations

while ($start+$i < $iterations) {

    $host->init(5)
	# ...there may be glitches in the electronics, 
	#   so we reset periodically
	if 0 == (($start+$i) % (100/$t2)) ;
    
    $host->display($life->get()) ; $host->delay($t2) ;
    
    my $next = $life->next() ; my $r = $H{$next->hash()} ;
    if (defined($r) && $next->equal($r->[0])) {
	# we've got a cylce, so let's start over
	$host->delay($t1) ;
	$host->echo("seed=$seed iteration=$i period=${\($i-$r->[1])}\n") ;
	$next = Life->make($matrices*8,8,++$seed) ;
	%H = () ;
	$start += $i ;
	$i = 0 ;
    }
    else {
	$H{$life->hash()} = [$life,$i] ;
	++$i ;
    }
    $life = $next ;
}
