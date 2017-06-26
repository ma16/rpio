#!/usr/bin/perl

use strict ;
use warnings ;

die("filename missing")
    if 0 == scalar @ARGV ;
my $fname = shift @ARGV ;
die("cannot open file <$fname>:$!")
    unless open(my $fh,"<:raw",$fname) ;

my @Pins ;
for my $pin (@ARGV) {
    die("invalid pin <$pin>")
	if ($pin!~/^[0-9]+$/) || ($pin<0) || (27<=$pin) ;
    push @Pins,int($pin) ;
}
die("not enough pins given")
    unless 14 == scalar(@Pins) ;

my $line = 0 ;
my $n = read($fh,my $buffer,4) ;
while ($n == 4) {
    my $i = ord(substr($buffer,3,1)) ;
    $i <<= 8 ;
    $i += ord(substr($buffer,2,1)) ;
    $i <<= 8 ;
    $i += ord(substr($buffer,1,1)) ;
    $i <<= 8 ;
    $i += ord(substr($buffer,0,1)) ;
    
    my $j = 0 ;
    for my $pin (reverse @Pins) {
	$j += $j ;
	$j += ($i >> $pin) & 0x1 ;
    }
    
    my $b = '' ;
    for (my $k=0 ; $k<scalar @Pins ; ++$k) {
	$b = $b . (0x1 & ($j>>$k)) . ' ' ;
    }

    printf "$line $b\n" ;
    
    ++$line ;
    $n = read($fh,$buffer,4) ;
}
die("read failed:$!")
    unless $n == 0 ;
