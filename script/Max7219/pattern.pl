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
# A simple pattern test
#
# A text file is generated and written to stdout. This text file can be 
# used as argument for the "rpio" tool in "max7219" mode.
#
# arguments:
#   -- the number of 8x8 dot matrices to be used
#
# A bar is written from left to right and bottom up with all 16 intensity 
# values.
#
# Note: this is just a little light demo. It's barely tested and might 
#   not work properly.
# ---------------------------------------------------------------------

use strict ;
use warnings ;

use File::Basename qw(dirname) ; use lib dirname($0) ;

use Canvas ;
use Host ;

die("arguments: <matrices>")
    unless 1 == scalar(@ARGV) ;

my $matrices = int($ARGV[0]) ;

my $host = Host->ctor(\*STDOUT,$matrices) ; 
$host->comment("test pattern file for rpio max7219; see https://github.com/ma16/rpio") ;

for my $i (0..15,reverse(0..14)) {
    $host->echo("intensity:$i\n") ;
    $host->init($i) ;
    my $canvas = Canvas->make($matrices*8,8) ;
    for my $on (1,0) {
	for my $j (0..$matrices*8-1) {
	    $canvas->line($j,0,$j,7,$on) ;
	    $host->display($canvas->map(0,0,$matrices*8,8)) ;
	    $host->delay(0.05) ;
	}
    }
    for my $on (1,0) {
	for my $j (0..7) {
	    $canvas->line(0,$j,$matrices*8,$j,$on) ;
	    $host->display($canvas->map(0,0,$matrices*8,8)) ;
	    $host->delay(0.05) ;
	}
    }
}
