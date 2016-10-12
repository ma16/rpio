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
#
# This project is hosted at https://github.com/ma16/rpio.git

# usage: <n> <n> <n> <n> <path>
# 1: memory coverage for RAM test
# 2: memory coverage for read i/o test
# 3: memory coverage for write i/o test
# 4: memory coverage for copy i/o test
# 5: path to "rpio" program

use integer ;
use strict ;
use warnings ;

use File::Basename qw(dirname) ;
    
my @Pools = ("pool 0x1","pool 0x2","pool 0x4","pool 0x8","pool 0x10","pool 0x20","pool 0x40","pool 0x80","pool 0x100") ; 

my $self = "$0" ;

my ($mcov,$rcov,$wcov,$ccov,$prog) = @ARGV ;

$prog = dirname($self) . "/rpio"
    if !defined($prog) || $prog eq '' ; 
die("program not found:$prog")
    unless -e "$prog" ;

$mcov = strto($mcov,0x1000000) ;
$rcov = strto($rcov,0x100000) ;
$wcov = strto($wcov,0x100000) ;
$ccov = strto($ccov,0x100000) ;

if ($mcov != 0) {
    test_ram($mcov) ;
}

if ($rcov != 0) {
    for my $src ("port 0x003 0x004","port 0x200 0x034","port 0x200 0x040") {
	test_rio($rcov,$src) ;
    }
}

if ($wcov != 0) {
    for my $dst ("port 0x200 0x040") {
	test_wio($wcov,$dst) ;
    }
}

if ($ccov != 0) {
    for my $src_dst ("port 0x200 0x040 port 0x200 0x040") {
	test_cio($ccov,$src_dst) ;
    }
}

exit ;

sub strto
{
    use POSIX 'strtoul';
    my ($s,$default) = @_ ;
    return $default 
	if !defined($s) || $s eq '' ;
    $! = 0 ;
    my ($i,$left) = POSIX::strtoul($s,0) ;
    die("number <$s> not recognized: $!")
	if $! ;
    die("trailing characters in: <$s>")
	if $left != 0 ;
    return $i ;
}

sub invoke
{
    my ($args) = @_ ;
    print "$args " ;
    my $r = qx{$prog $args 2>&1} ;
    die("program <$prog $args> failed:$?")
	unless $? == 0 ;
    print "$r" ;
}

sub test_ram
{
    my ($coverage) = @_ ; 

    my $mwords = 0x1000000 ; $mwords = $coverage if $coverage < $mwords ;

    # 0:1
    for my $how ("iter",@Pools) {
	my $args = sprintf("0x%x 0:1 plain $how",$coverage) ;
	invoke($args) ;
    }

    # 0:n
    for (my $nwords=2 ; $nwords<=$mwords ; $nwords+=$nwords) {
	my $rep = $coverage / $nwords ;
	invoke(sprintf("0x%x 0:n 0x%x plain libc",$rep,$nwords)) ;
	invoke(sprintf("0x%x 0:n 0x%x plain iter",$rep,$nwords)) ;
	for (my $m=1 ; $m<=0x100 && $m<=$nwords ; $m+=$m) {
	    invoke(sprintf("0x%x 0:n 0x%x plain blck 0x%x",$rep,$nwords,$m)) ;
	}
    }

    # 1:0
    for my $how ("iter",@Pools) {
	my $args = sprintf("0x%x 1:0 plain $how",$coverage) ;
	invoke($args) ;
    }

    # 1:1
    for my $how ("iter",@Pools) {
	my $args = sprintf("0x%x 1:1 plain plain $how",$coverage) ;
	invoke($args) ;
    }

    # n:0
    for (my $nwords=2 ; $nwords<=$mwords ; $nwords+=$nwords) {
	my $rep = $coverage / $nwords ;
	invoke(sprintf("0x%x n:0 0x%x plain iter",$rep,$nwords)) ;
	for (my $m=1 ; $m<=0x100 && $m<=$nwords ; $m+=$m) {
	    invoke(sprintf("0x%x n:0 0x%x plain blck 0x%x",$rep,$nwords,$m)) ;
	}
    }

    # 1:n
    for (my $nwords=2 ; $nwords<=$mwords ; $nwords+=$nwords) {
	my $rep = $coverage / $nwords ;
	invoke(sprintf("0x%x 1:n 0x%x plain plain iter",$rep,$nwords)) ;
	for (my $m=1 ; $m<=0x100 && $m<=$nwords ; $m+=$m) {
	    invoke(sprintf("0x%x 1:n 0x%x plain plain blck 0x%x",$rep,$nwords,$m)) ;
	}
    }

    # n:n
    for (my $nwords=2 ; $nwords<=$mwords ; $nwords+=$nwords) {
	my $rep = $coverage / $nwords ;
	invoke(sprintf("0x%x n:n 0x%x plain plain libc",$rep,$nwords)) ;
	invoke(sprintf("0x%x n:n 0x%x plain plain iter",$rep,$nwords)) ;
	for (my $m=1 ; $m<=0x100 && $m<=$nwords ; $m+=$m) {
	    invoke(sprintf("0x%x n:n 0x%x plain plain blck 0x%x",$rep,$nwords,$m)) ;
	}
    }
}

sub test_rio
{
    my ($coverage,$src) = @_ ;

    my $mwords = 0x1000000 ; $mwords = $coverage if $coverage < $mwords ;

    # 1:0
    for my $how ("iter",@Pools) {
	my $args = sprintf("0x%x 1:0 $src $how",$coverage) ;
	invoke($args) ;
    }

    # 1:1
    for my $how ("iter",@Pools) {
	my $args = sprintf("0x%x 1:1 $src plain $how",$coverage) ;
	invoke($args) ;
    }

    # 1:n
    for (my $nwords=2 ; $nwords<=$mwords ; $nwords+=$nwords) {
	my $rep = $coverage / $nwords ;
	invoke(sprintf("0x%x 1:n 0x%x $src plain iter",$rep,$nwords)) ;
	for (my $m=1 ; $m<=0x100 && $m<=$nwords ; $m+=$m) {
	    invoke(sprintf("0x%x 1:n 0x%x $src plain blck 0x%x",$rep,$nwords,$m)) ;
	}
    }
}

sub test_wio
{
    my ($coverage,$dst) = @_ ;

    my $mwords = 0x1000000 ; $mwords = $coverage if $coverage < $mwords ;

    # 0:1
    for my $how ("iter",@Pools) {
	my $args = sprintf("0x%x 0:1 $dst $how",$coverage) ;
	invoke($args) ;
    }

    # 1:1
    for my $how ("iter",@Pools) {
	my $args = sprintf("0x%x 1:1 plain $dst $how",$coverage) ;
	invoke($args) ;
    }

    # n:1
    for (my $nwords=2 ; $nwords<=$mwords ; $nwords+=$nwords) {
	my $rep = $coverage / $nwords ;
	invoke(sprintf("0x%x n:1 0x%x plain $dst iter",$rep,$nwords)) ;
	for (my $m=1 ; $m<=0x100 && $m<=$nwords ; $m+=$m) {
	    invoke(sprintf("0x%x n:1 0x%x plain $dst blck 0x%x",$rep,$nwords,$m)) ;
	}
    }
}

sub test_cio
{
    my ($coverage,$src_dst) = @_ ;

    # 1:1
    for my $how ("iter",@Pools) {
	my $args = sprintf("0x%x 1:1 $src_dst $how",$coverage) ;
	invoke($args) ;
    }
}
