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
# This project is hosted at https://github.com/ma16/rpio

# ---------------------------------------------------------------------
# a simple parser that returns tokens and skips whitespaces.
#
# a token is
# --either a word that does not contain any whitespace
# --or a sequence of characters enclosed in quotes (")
#
# this module is just used by the Font module ro read .BDF files
#
# Note: this is just a little light demo. It's barely tested and might 
#   not work properly.
# ---------------------------------------------------------------------

package Parser ;

use strict ;
use warnings ;

my $pkg = __PACKAGE__ ;

sub make
{
    my ($class,$fh) = @_ ;
    my $self = { fh=>$fh } ;
    return bless $self,$class ;
}

# return the head character
sub peek
{
    my ($self) = @_ ;
    my $c = $self->{peek} ;
    unless (defined($c)) {
	$c = getc($self->{fh}) ;
	die("read error:$!")
	    unless defined($c) || eof($self->{fh}) ;
	# ...actually, an error should have occurred if defined($!). but
	# that doesn't seem to work. It says at eof: bad file descriptor.
	$self->{peek} = $c ;
    }
    return $c ;
}

# return and delete the head character
sub pop
{
    my ($self) = @_ ;
    my $c = $self->peek() ;
    delete $self->{peek} ;
    return $c ;
}

# return and delete the head token
sub next
{
    my ($self) = @_ ;
    my $c = $self->pop() ;
    # skip whitespaces
    while (defined($c) && $c =~ /^\s$/) {
	$c = $self->pop() ;
    }
    return undef
	unless defined($c) ;
    my $s = $c ;
    if ($c eq '"') {
	# quoted text as "hello world!"
	$c = $self->pop() ;
	while (defined($c) && ($c ne '"')) {
	    $s .= $c ;
	    $c = $self->pop() ;
	}
	die("quotes do not match")
	    unless defined($c) ;
	$s .= $c ;
    }
    else {
	# words, i.e. sequences of characters that contain no whitespaces
	$c = $self->peek() ;
	while (defined($c) && ($c =~ /^\S$/)) {
	    $s .= $c ;
	    $self->pop() ;
	    $c = $self->peek() ;
	}
    }
    return $s ;
}

# just a helper that aborts if the expected string is not there
sub expect
{
    my ($self,$e) = @_ ;
    my $s = $self->next() ;
    die("expected <$e> got <$s>")
	unless $e eq $s ;
}
    
1 ;
