#!/usr/bin/perl

# ---------------------------------------------------------------------
# Create a binary file according to the given (text) definition
#
# This is used to create a list of control-blocks for DMA tests. The 
# resulting file can be loaded into memory.
#
# Example:
# +0x0000    0x0 0x7e003004     +0x60      0x4 0x0 +0x0020 0x0
# +0x0020 0x7030 0x7e200034     +0x64 0x100000 0x0 +0x0040 0x0 # --ti-dest-inc --ti-dest-width --ti-burst-length 7
# +0x0040    0x0 0x7e003004 +0x100064      0x4 0x0       0
# +0x100068
#
# only flat memory layout supported at the moment (not segmented pages)
# ---------------------------------------------------------------------

use strict ;
use warnings ;

die("arguments: base in out\n")
    unless 3 == scalar(@ARGV) ;

my ($base,$ifname,$ofname) = @ARGV ;

$base = strtou($base) ;
die("open($ifname):$!")
    unless defined(open(my $ifh,'<',$ifname)) ;
die("open($ofname):$!")
    unless defined(open(my $ofh,'>',$ofname)) ;
binmode($ofh) ;

my $pos = 0 ;
my $word = "\0\0\0\0" ;

my $line = readline($ifh) ;
while (defined($line)) {
    chomp $line ;
    my @R = split(/\s+/,$line) ;
    next 
	if (0 < scalar(@R)) && (substr($R[0],0,1) eq '#') ;
    # first item: offset
    my ($n,$plus) = parse(shift(@R)) ;
    die("offset must be relative")
	unless $plus ;
    while ($pos < $n) {
	print $ofh $word ;
	$pos += 4 ;
    }
    die("would overwrite previously written data")
	if $n < $pos ;
    # list of 32-bit words
    for my $s ( @R ) {
	last 
	    if substr($s,0,1) eq '#' ;
	my ($n,$plus) = parse($s) ;
	$n += $base
	    if $plus ;
	$word = utobin($n,4) ;
	print $ofh $word ;
	$pos += 4 ;
    }
    $line = readline($ifh) ;
}
die("readline($ifname):$!")
    if !eof($ifh) ;

sub strtou
{
    my ($s) = @_ ;
    die("strtoul(undef)")
        unless defined($s) ;
    die("strtoul($s):empty")
        if 0 == length($s) ;
    die("strtoul($s):signed")
        if ('+' eq substr($s,0,1)) || ('-' eq substr($s,0,1)) ;
    use POSIX 'strtoul';
    $! = 0 ;
    my ($i,$left) = POSIX::strtoul($s,0) ;
    die("strtoul($s): $!")
        if 0 != $! ;
    die("strtoul($s):trailing characters")
        if $left != 0 ;
    return $i ;
}

sub parse 
{
    my ($s) = @_ ;
    my $plus ;
    die("parse(undef)")
        unless defined($s) ;
    if ((length($s)>0) && ('+' eq substr($s,0,1))) {
	$plus = 1 ;
	$s = substr($s,1)
    }
    my $n = strtou($s) ;
    return ($n,$plus) ;
}

sub utobin
{
    my ($n,$digits) = @_ ;
    my $s = "" ;
    # LSB first
    while ($digits-- > 0) {
	$s .= chr($n % 0x100) ;
	$n /= 0x100 ;
    }
    return $s ;
}
