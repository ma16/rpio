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

# ---------------------------------------------------------------------
# This script parses the output of the test cases that were generated
# by <throughput.pl 0x80000000 0x8000000 0x8000000 0x8000000>
# Other parameters will probably not work here.
#
# arguments: text-file with output of throughput.pl
#
# creates 8 x 2 files with test results (csv- + png-file)
#
# Note:
#
# The code is hardly maintainable, and this will probably not change.
#
# The generated files are used to create the documentation. However,
# there is quite some manual work left to embed the tables from CSV 
# files (pictures are easy). And it also depends heavily on the 
# throughput.pl script and its arguments. In case it should become 
# necessary to be changed / to be used again, maybe something new/other
# is better suited.
# 
# ---------------------------------------------------------------------

use warnings ;
use strict ;

use GD::Graph::bars ;
use GD::Graph::bars3d ;

{
    my $in = $ARGV[0] ;

    open(my $fin,"<$in") 
	or die $!;

    my $h_rwc_word = {} ;
    my $h_plain_nn = {} ;
    my $h_plain_0n = {} ;
    my $h_ports_peek = {} ;
    my $h_rwc_event = {} ;
    my $h_read_timer = {} ;
    my $h_read_level = {} ;
    my $h_read_event = {} ;
    
    while (my $line = readline($fin)) {
	my $r = parse($line) ;

	enter_rwc_word($h_rwc_word,$r) ;
	enter_plain_xn($h_plain_nn,$r,1) ;
	enter_plain_xn($h_plain_0n,$r,0) ;
	enter_ports_peek($h_ports_peek,$r) ;
	enter_rwc_event($h_rwc_event,$r) ;
	enter_read_port($h_read_timer,$r,'0x003','0x004') ;
	enter_read_port($h_read_level,$r,'0x200','0x034') ;
	enter_read_port($h_read_event,$r,'0x200','0x040') ;
    }
    die("readline:$!")
	unless eof($fin) ;

    my ($table,$z) ;
    ($table,$z) = make_rwc_word($h_rwc_word) ; csv($table,$z,"$in.rwc_word.csv") ; draw($table,$z,1000,"$in.rwc_word.png") ;
    ($table,$z) = make_plain_xn($h_plain_nn,1) ; csv($table,$z,"$in.plain_nn.csv") ; draw($table,$z, 700,"$in.plain_nn.png") ;
    ($table,$z) = make_plain_xn($h_plain_0n,0) ; csv($table,$z,"$in.plain_0n.csv") ; draw($table,$z,1000,"$in.plain_0n.png") ;
    ($table,$z) = make_ports_peek($h_ports_peek) ; csv($table,$z,"$in.ports_peek.csv") ; draw($table,$z,25,"$in.ports_peek.png") ;
    ($table,$z) = make_rwc_event($h_rwc_event) ; csv($table,$z,"$in.rwc_event.csv") ; draw($table,$z,100,"$in.rwc_event.png") ;
    ($table,$z) = make_read_port($h_read_timer) ; csv($table,$z,"$in.read_timer.csv") ; draw($table,$z,20,"$in.read_timer.png") ;
    ($table,$z) = make_read_port($h_read_level) ; csv($table,$z,"$in.read_level.csv") ; draw($table,$z,20,"$in.read_level.png") ;
    ($table,$z) = make_read_port($h_read_event) ; csv($table,$z,"$in.read_event.csv") ; draw($table,$z,20,"$in.read_event.png") ;

    exit(0) ;
}

# ---------------------------------------------------------------------

sub draw
{
    my ($table,$z,$max,$fname) = @_ ;
    my $graph = new GD::Graph::bars3d(800,600)
	or die GD::Graph->error() ;
    $graph->set_legend(@$z) ;
    $graph->set(
	overwrite => 1,
	#shading => 0, # doesn't work
	y_label => 'Throughput',
	#bar_depth => 20, # should depend on $z length
	#box_axis => 0, # doesn't work
	transparent => 0,
	#bar_spacing        => 1, # doesn't work
	#bargroup_spacing   => 100, # doesn't work
	y_max_value => $max,
	)
	or die $graph->error() ;
    my $font = '/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf' ;
    # ..or any other that actually works
    my $fs = 15 ;
    # ..or any other; should relate to image size 800x600
    $graph->set_title_font($font,$fs);
    $graph->set_x_label_font($font,$fs);
    $graph->set_y_label_font($font,$fs);
    $graph->set_x_axis_font($font,$fs);
    $graph->set_y_axis_font($font,$fs);
    $graph->set_legend_font($font,$fs);
    my $gd = $graph->plot( $table )
	or die $graph->error() ;
    open(my $file,">$fname") 
	or die $!;
    binmode $file ;
    print $file $gd->png() ;
    close($file) ;
}

sub csv
{
    my ($table,$z,$fname) = @_ ;
    open(my $file,">$fname") 
	or die("open>$fname:$!") ;
    print $file "," . join(',',@{$table->[0]}) . "\n" ;
    for (my $i=1 ; $i<scalar(@$table) ; ++$i) {
	print $file "$z->[$i-1]" ;
	for my $v ( @{$table->[$i]} ) {
	    $v = 'n/a'
		unless defined($v) ;
	    print $file ",$v" ;
	}
	print $file "\n" ;
    }
    close($file) ;
}

# ---------------------------------------------------------------------

sub location
{
    my ($l) = @_ ;
    my $arg = shift(@$l) ;
    if ($arg eq 'plain') {
	return [ $arg ] ;
    }
    if ($arg eq 'port') {
	my $page = shift(@$l) ;
	my $index = shift(@$l) ;
	return [ $arg,$page,$index ] ;
    }
    die("not supported location:$arg") ;
}

sub parse
{
    my ($line) = @_ ;  my $r = {} ;

    chomp($line) ; my @F = split(/ /,$line) ;

    $r->{rep} = hex(shift(@F)) ;

    my $arg = shift(@F) ;
    die("not supported mode:$arg")
	unless $arg =~ /^0:1|0:n|1:0|1:1|1:n|n:0|n:1|n:n$/ ;
    my $n ;
    if ($arg =~ /n/) {
	$n = hex(shift(@F)) + 0 ;
	die("buffer must have at least two bytes:$n") 
	    if $n < 2 ;
    }
    my $nsrc = substr($arg,0,1) ; $nsrc = $n if $nsrc eq 'n' ; my @Src ; @Src = @{location(\@F)} unless $nsrc == 0 ; unshift(@Src,$nsrc) ; $r->{src} = \@Src ;
    my $ndst = substr($arg,2,1) ; $ndst = $n if $ndst eq 'n' ; my @Dst ; @Dst = @{location(\@F)} unless $ndst == 0 ; unshift(@Dst,$ndst) ; $r->{dst} = \@Dst ;

    $arg = shift(@F) ;
    my @Type = ( $arg ) ;
    if    ($arg eq 'blck') { my $m=hex(shift(@F)) ; push(@Type,$m) ; }
    elsif ($arg eq 'iter') { }
    elsif ($arg eq 'libc') { }
    elsif ($arg eq 'pool') { my $m=hex(shift(@F)) ; push(@Type,$m) ; }
    else { die("unrecognized type:$arg") ; }
    $r->{type} = \@Type ;

    $arg = shift(@F) ;
    $r->{rate} = substr($arg,0,length($arg)-2) + 0.0 ; 

    return $r ;
}

# ---------------------------------------------------------------------

sub enter_rwc_word
{
    my ($h,$r) = @_ ;
    my ($sn,$stype) = @{$r->{src}} ;
    my ($dn,$dtype) = @{$r->{dst}} ;
    return
	if ($sn >= 2) || ($dn >= 2) ;
    return
	if ($sn && $stype ne 'plain') || ($dn && $dtype ne 'plain') ;
    my $nn = "$sn:$dn" ;
    my $h_nn = $h->{$nn} ;
    unless (defined($h_nn)) {
	$h_nn = {} ;
	$h->{$nn} = $h_nn ;
    }
    my $type = join('/',@{$r->{type}}) ;
    die("duplicate:<$nn>:<$type>")
	if defined($h_nn->{$type}) ;
    $h_nn->{$type} = $r->{rate} ;
}

sub make_rwc_word
{
    my ($h) = @_ ;
    my @Table ;
    my @Types = qw( pool/1 pool/2 pool/4 pool/8 pool/16 pool/32 pool/64 pool/128 pool/256 iter ) ;
    my @Row = qw( 1 2 4 8 16 32 64 128 256 Iter. ) ;
    push @Table,\@Row ;
    my @Nn = qw( 1:0 0:1 1:1 ) ; my @Z = qw( Peek Poke Copy ) ;
    for my $nn (@Nn) {
	my $h_nn = $h->{$nn} ;
	my @Row ;
	for my $type (@Types) {
	    push @Row,$h_nn->{$type}/1E+6 ;
	}
	push @Table,\@Row ;
    }
    return (\@Table,\@Z) ;
}

# ---------------------------------------------------------------------

sub enter_plain_xn
{
    my ($h,$r,$copy) = @_ ;
    my ($sn,$stype) = @{$r->{src}} ;
    my ($dn,$dtype) = @{$r->{dst}} ;
    if ($copy != 0) {
	return # consider only n:n (copy)
	    unless ($sn>1) && ($sn==$dn) && ($stype eq 'plain') && ($dtype eq 'plain') ;
    }
    else {
	return # consider only 0:n (poke)
	    unless ($sn==0) && ($dn>1) && ($dtype eq 'plain') ;
    }
    my $type = join('/',@{$r->{type}}) ;
    my $h_type = $h->{$type} ;
    unless (defined($h_type)) {
	$h_type = {} ;
	$h->{$type} = $h_type ;
    }
    my $n = "$dn" ;
    die("duplicate:<$n>:<$type>")
	if defined($h_type->{$n}) ;
    $h_type->{$n} = $r->{rate} ;
}

sub make_plain_xn
{
    my ($h) = @_ ;
    my @Table ;
    my @Row ; for (my $i=1 ; $i<=24 ; ++$i) { push @Row,$i ; }
    push @Table,\@Row ;
    my @N = map { 2**$_ } @Row ;
    my @Types = qw( libc iter blck/256 blck/128 blck/64 blck/32 blck/16 blck/8 blck/4 blck/2 blck/1 ) ;
    my @Z = qw( Libc Iter. 256 128 64 32 16 8 4 2 1 ) ;
    for my $type (@Types) {
	my $h_type = $h->{$type} ;
	my @Row ;
	for my $n (@N) {
	    if (exists $h_type->{$n}) {
		push @Row,$h_type->{$n}/1E+6 ;
	    }
	    else {
		push @Row,undef ;
	    }
	}
	push @Table,\@Row ;
    }
    return (\@Table,\@Z) ;
}

# ---------------------------------------------------------------------

sub enter_ports_peek
{
    my ($h,$r) = @_ ;
    my ($sn,$stype,$spage,$six) = @{$r->{src}} ;
    my ($dn) = @{$r->{dst}} ;
    return 
	unless ($sn==1 && $stype eq 'port' && $dn==0) ;
    my $addr = "$spage:$six" ;
    my $h_addr = $h->{$addr} ;
    unless (defined($h_addr)) {
	$h_addr = {} ;
	$h->{$addr} = $h_addr ;
    }
    my $type = join('/',@{$r->{type}}) ;
    die("duplicate:<$addr>:<$type>")
	if defined($h_addr->{$type}) ;
    $h_addr->{$type} = $r->{rate} ;
}

sub make_ports_peek
{
    my ($h) = @_ ;
    my @Table ;
    my @Types = qw( pool/1 pool/2 pool/4 pool/8 pool/16 pool/32 pool/64 pool/128 pool/256 iter ) ;
    my @Row = qw( 1 2 3 8 16 32 64 128 256 Iter.  ) ;
    push @Table,\@Row ;
    my @Addr = qw ( 0x003:0x004 0x200:0x034 0x200:0x040 ) ; my @Z = qw( Timer Level Event ) ;
    for my $addr (@Addr) {
	my $h_addr = $h->{$addr} ;
	my @Row ;
	for my $type (@Types) {
	    push @Row,$h_addr->{$type}/1E+6 ;
	}
	push @Table,\@Row ;
    }
    return (\@Table,\@Z) ;
}

# ---------------------------------------------------------------------

sub enter_rwc_event
{
    my ($h,$r) = @_ ;
    my ($sn,$stype,$spage,$six) = @{$r->{src}} ;
    my ($dn,$dtype,$dpage,$dix) = @{$r->{dst}} ;
    return 
	unless 
	($sn==1 && $stype eq 'port' && $spage eq '0x200' && $six eq '0x040' && $dn==0) ||
	($dn==1 && $dtype eq 'port' && $dpage eq '0x200' && $dix eq '0x040' && $sn==0) ||
	($sn==1 && $stype eq 'port' && $spage eq '0x200' && $six eq '0x040' && 
	 $dn==1 && $dtype eq 'port' && $dpage eq '0x200' && $dix eq '0x040') ;
    my $nn = "$sn:$dn" ;
    my $h_nn = $h->{$nn} ;
    unless (defined($h_nn)) {
	$h_nn = {} ;
	$h->{$nn} = $h_nn ;
    }
    my $type = join('/',@{$r->{type}}) ;
    die("duplicate:<$nn>:<$type>")
	if defined($h_nn->{$type}) ;
    $h_nn->{$type} = $r->{rate} ;
}

sub make_rwc_event
{
    my ($h) = @_ ;
    my @Table ;
    my @Types = qw( pool/1 pool/2 pool/4 pool/8 pool/16 pool/32 pool/64 pool/128 pool/256 iter ) ;
    my @Row = qw( 1 2 4 8 16 32 64 128 256 Iter.  ) ;
    push @Table,\@Row ;
    my @Nn = ( "0:1","1:0","1:1" ) ; my @Z = ( "Poke","Peek","Copy" ) ;
    for my $nn (@Nn) {
	my $h_nn = $h->{$nn} ;
	my @Row ;
	for my $type (@Types) {
	    push @Row,$h_nn->{$type}/1E+6 ;
	}
	push @Table,\@Row ;
    }
    return (\@Table,\@Z) ;
}

# ---------------------------------------------------------------------

sub enter_read_port
{
    my ($h,$r,$page,$ix) = @_ ;
    my ($sn,$stype,$spage,$six) = @{$r->{src}} ;
    my ($dn,$dtype) = @{$r->{dst}} ;
    return
	unless ($sn==1) && ($stype eq 'port') && ($spage eq $page) && ($six eq $ix) && ($dn>1) && ($dtype eq 'plain') ;
    my $type = join('/',@{$r->{type}}) ;
    my $h_type = $h->{$type} ;
    unless (defined($h_type)) {
	$h_type = {} ;
	$h->{$type} = $h_type ;
    }
    my $n = "$dn" ;
    die("duplicate:<$n>:<$type>")
	if defined($h_type->{$n}) ;
    $h_type->{$n} = $r->{rate} ;
}

sub make_read_port
{
    my ($h) = @_ ;
    my @Table ;
    my @Row ; for (my $i=1 ; $i<=24 ; ++$i) { push @Row,$i ; }
    push @Table,\@Row ;
    my @N = map { 2**$_ } @Row ;
    my @Types = qw ( iter blck/256 blck/128 blck/64 blck/32 blck/16 blck/8 blck/4 blck/2 blck/1 ) ;
    my @Z = qw( Iter. 256 128 64 32 16 8 4 2 1 ) ;
    for my $type (@Types) {
	my $h_type = $h->{$type} ;
	my @Row ;
	for my $n (@N) {
	    if (exists $h_type->{$n}) {
		push @Row,$h_type->{$n}/1E+6 ;
	    }
	    else {
		push @Row,undef ;
	    }
	}
	push @Table,\@Row ;
    }
    return (\@Table,\@Z) ;
}

# ---------------------------------------------------------------------

