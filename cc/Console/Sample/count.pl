#!/usr/bin/perl

use strict ;
use warnings ;

my ($file) = @ARGV ;

die("filename required")
    if !defined($file) ;

die("file not found:\"$file\"")
    unless -r "$file" ;

open(my $fh, "<", $file)
    or die "Can't open $file: $!";

my $buffer = '' ;
my $n = read($fh,$buffer,4) ;

my $count=0 ; 
my $total ;

my $slast ; # the previous sampled value
my $clast ;

my @Q ;
my @M ;

sub medium 
{
    my ($delta,$count) = @_ ;
    my $sum = 0 ; my $n = 0 ;
    if (scalar(@M) > 0)
    {
	for (my $i=0 ; $i<scalar(@M) ; ++$i)
	{
	    $sum += ($i+1) * $M[$i] ;
	    $n += ($i+1) ;
	}
    }
    for (my $i=0 ; $i<$count ; ++$i)
    {
	push @M, $delta/$count ;
	$sum += scalar(@M) * $delta/$count ;
	$n += scalar(@M) ;
    }
    my $avg = $sum / $n ;
    shift @M
	while scalar(@M) > 32 ;
    return $avg ;
}

while (defined($n) && $n == 4)
{
    my $i = ord(substr($buffer,3,1)) ;
    {
	use integer ;
	$i <<= 8 ;
	$i += ord(substr($buffer,2,1)) ;
	$i <<= 8 ;
	$i += ord(substr($buffer,1,1)) ;
	$i <<= 8 ;
	$i += ord(substr($buffer,0,1)) ;
    }

    my $sample = 1 ^ (($i/16) % 0x800) ;

    push @Q,$sample ; # look ahead queue

    next
	if scalar(@Q) <= 0x800 ;

    # todo: wait until 0b11111111111 to sync !?
    #   and any wrap-around -> check for number of samples between wrap arounds
    # wrap-around: highest bit gets set - highest bit gets unset
    
    $sample = pop @Q ;
    
    if ($count == 0)
    {
	$total = 0 ;
	
	printf "0 %11b\n", $sample ;
	
	$slast = $sample ; $clast = $count ;
    }
    elsif ($sample > $slast)
    {
	$total += $sample - $slast ;

	my $avg = $total / $count ;

	my $avg2 = medium($sample - $slast,$count - $clast) ;
	
	printf "+ %11b %f %f %f\n", 
	$sample, ($sample - $slast) / ($count - $clast), $avg2, $avg ;
	
	$slast = $sample ; $clast = $count ;

	# todo: (example)

	# = 1 1 1 1 1 1 1 1 1 1 1       = 1 1 1 1 1 1 1 1 1 1 1
	# - 1 1 1 1 1<0 0 0 0<0 1  [NA] ~ 0 0 0 0 0 0 0 0 1 0 1 (+6)
	# + 0 0 0 0 0 0 0 0<0 1 0  [+1] ~ 0 0 0 0 0 0 0 1 0 1 0 (+5)  
	# = 0 0 0 0 0 0 0 1 1 1<0 [+12] ~ 0 0 0 0 0 0 1 0 0 0 0 (+6)
	# = 0 0 0 0 0 0 1 0 1 0<0  [+6] ~ 0 0 0 0 0 0 1 0 1 1 0 (+6)
	# = 0 0 0 0 0 0 1 1 0 1<0  [+6] ~ 0 0 0 0 0 0 1 1 1 0 0 (+6)
	# - 0 0 0 0 0 0 1 1<0 0 1  [NA] ~ 0 0 0 0 0 1 0 0 0 0 1 (+5)
	# = 0 0 0 0 0 1 0 0 1 1 1  [+6] = 0 0 0 0 0 1 0 0 1 1 1 (+6)
	# = 0 0 0 0 0 1 0 1 0<0 1  [+2] ~ 0 0 0 0 0 1 0 1 1 0 1 (+6)
	# = 0 0 0 0 0 1 0 1<0 1 1  [+2] ~ 0 0 0 0 0 1 1 0 0 1 1 (+6)
	# = 0 0 0 0 0 1 1 0 1 1 0 [+11] = 0 0 0 0 0 1 1 0 1 1 0 (+5)

	#   0 | 1 1 1 1 1 1 1 1 1 1 1 = 1 1 1 1 1 1 1 1 1 1 1
	#   . | 0 0 0 0 0 0 0 0 0 0 0 ~ 1 1 1 1 1 1 1 1 1 1<0
	#   . | 0 0 0 0 0 0 0 0 0 0 1 ~ 1 1 1 1 1 1 1 1 1<0 1
	#   . | 0 0 0 0 0 0 0 0 0 1 0 ~ 1 1 1 1 1 1 1 1<0 0<0 
	#   . | 0 0 0 0 0 0 0 0 0 1 1 ~ 1 1 1 1 1 1 1<0 0 1 1
	#   . | 0 0 0 0 0 0 0 0 1 0 0 ~ 1 1 1 1 1 1<0 0 0 1<0
	# +60 | 0 0 0 0 0 0 0 0 1 0 1 ~ 1 1 1 1 1<0 0 0 0<0 1
	
	# + 0 0 0 0 0 0 0 0<0 1 0  [+1] ~ 0 0 0 0 0 0 0 1 0 1 0 (+5)  
	# = 0 0 0 0 0 0 0 1 1 1<0 [+12] ~ 0 0 0 0 0 0 1 0 0 0 0 (+6)
	# = 0 0 0 0 0 0 1 0 1 0<0  [+6] ~ 0 0 0 0 0 0 1 0 1 1 0 (+6)
	# = 0 0 0 0 0 0 1 1 0 1<0  [+6] ~ 0 0 0 0 0 0 1 1 1 0 0 (+6)
	# - 0 0 0 0 0 0 1 1<0 0 1  [NA] ~ 0 0 0 0 0 1 0 0 0 0 1 (+5)
	# = 0 0 0 0 0 1 0 0 1 1 1  [+6] = 0 0 0 0 0 1 0 0 1 1 1 (+6)
	# = 0 0 0 0 0 1 0 1 0<0 1  [+2] ~ 0 0 0 0 0 1 0 1 1 0 1 (+6)
	# = 0 0 0 0 0 1 0 1<0 1 1  [+2] ~ 0 0 0 0 0 1 1 0 0 1 1 (+6)
	# = 0 0 0 0 0 1 1 0 1 1 0 [+11] = 0 0 0 0 0 1 1 0 1 1 0 (+5)
	
	# -> how detect whether one or more carry-over are in progress 
	#    and if, where they are located
	# a ripple can only be in progress after (before) a 0-bit; hence, 
	# if all bits are one: no ripple is in progress
    }
    elsif (((0x400 & $sample) == 0) && ((0x400 & $slast) != 0))
    {
	# wrap around ; same caveats as above apply [todo]
	
	$total += 0x800 + $sample - $slast ;

	my $avg = $total / $count ;
	
	my $avg2 = medium(0x800 + $sample - $slast,$count - $clast) ;
	
	printf "1 %11b %f %f %f\n", 
	$sample, (0x800 + $sample - $slast) / ($count - $clast), $avg2, $avg ;
	
	$slast = $sample ; $clast = $count ;
    }
    elsif ($sample == $slast)
    {
	printf "= %11b\n", $sample ;
    }
    else
    {
	# we need to deal with that if caused by wrap around
	# otherwise the next sample>=last will hold a lot of counts
	printf "- %11b\n", $sample ;
    }

    ++$count ;

    $n = read($fh,$buffer,4) ;
}

die("file not a multiple of word-size")
    if !defined($n) || ($n != 0) ;
