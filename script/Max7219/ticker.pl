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
# A ticker that scrolls a text forward and back again (upside-down).
#
# A text file is generate and written to stdout. This text file can be 
# used as argument for the "rpio" tool in "max7219" mode.
#
# arguments:
#   -- the number of 8x8 dot matrices to be used
#   -- the quoted text string to scroll
#
# e.g.: $ ticker.pl 5 "The quick brown fox jumps over the lazy dog."
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

die("arguments: <matrices> <quoted-text>")
    unless 2 == scalar(@ARGV) ;
my ($matrices,$text) = @ARGV ;

my $host = Host->ctor(\*STDOUT,$matrices) ; 
$host->comment("ticker file for rpio max7219; see https://github.com/ma16/rpio") ;
$host->comment("text:'$text'") ;
my $intensity = 5 ;
$host->init($intensity) ; 
$host->echo("$text\n") ;

my $font = Font->load(dirname($0)."/6x9-ISO8859-15.bdf") ;
my ($fwidth,$fheight,$y0) = (6,9,0) ;

my $canvas = Canvas->make($fwidth*length($text),$fheight) ;
for my $i (0..length($text)-1) {
    $font->draw($canvas,$fwidth*$i,$y0,ord(substr($text,$i,1))) ;
}

my $t1 = 0.05 ; # delay between frames
my $t2 = 2.00 ; # delay before/after scrolling

$host->display($canvas->map(0,0,$matrices*8,8)) ;
$host->delay($t2) ;
for my $i (0..$fwidth*length($text)-$matrices*8) {
    $host->init($intensity) ; 
    $host->display($canvas->map($i,0,$matrices*8,8)) ;
    $host->delay($t1) ;
}
$host->delay($t2) ;
$canvas->upsideDown() ;
$host->display($canvas->map($fwidth*length($text)-$matrices*8,0,$matrices*8,8)) ;
$host->delay($t2) ;
for my $i (reverse(0..$fwidth*length($text)-$matrices*8)) {
    $host->init($intensity) ; 
    $host->display($canvas->map($i,0,$matrices*8,8)) ;
    $host->display($canvas->map($i,0,$matrices*8,8)) ;
    $host->delay($t1) ;
}
$host->delay($t2) ;
$host->init($intensity) ; 
