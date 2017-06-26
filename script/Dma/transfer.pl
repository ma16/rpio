#!/usr/bin/perl

use strict ;
use warnings ;
use System ;

my ($file,$dmac,$rpio,$image) = @ARGV ;

die("file not found:\"$file\"")
    unless -r "$file" ;

$dmac = 0
    unless defined($dmac) ;

$rpio = "./rpio"
    unless defined($rpio) ;
die("rpio program not found or not executable:\"$rpio\"")
    unless -x "$rpio" ;

$image = "./image.pl"
    unless defined($image) ;
die("image.pl script not found or not executable:\"$image\"")
    unless -x "$image" ;

print STDERR "building binary file to estimate size..." ;

System::invoke("$image","0","$file","$file.bin") ;

my $nbytes = `ls -l "$file.bin" | awk '{print \$5}'` ;
die("command failed:$!")
    unless 0 == $? ;
chomp $nbytes ;
print " $nbytes bytes\n" ;

my $mh = System::invoke("sudo","$rpio","mbox","memory","allocate","$nbytes","0x1000","0x44") ;
chomp($mh) ;
print "memory handle: $mh\n" ;

my $addr = `sudo "$rpio" mbox memory lock $mh` ; 
die("command failed:$!")
    unless 0 == $? ;
chomp($addr) ;
$addr = System::strtou($addr) ;
printf("memory address: %x\n",$addr) ;
    
print STDERR "building binary file with address relocation..." ;
System::invoke("$image","$addr","$file","$file.bin") ;
print " done\n" ;

System::invoke("sudo","$rpio","poke",$addr & 0x3fffffff,"write","$file.bin") ;
System::invoke("sudo","$rpio","dma","start","$dmac","$addr") ;
System::invoke("sudo","$rpio","poke",$addr & 0x3fffffff,"read","$nbytes","$file.out") ;
System::invoke("sudo","$rpio","mbox","memory","release","$mh") ;

my $user  = System::invoke("id","-n","-u") ; chomp($user) ;
my $group = System::invoke("id","-n","-g") ; chomp($group) ;
System::invoke("sudo","chown","$user:$group","$file.out") ;

print "finished\n" ;
