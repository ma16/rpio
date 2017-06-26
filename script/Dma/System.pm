package System ;

sub readEof 
{
    my ($fh) = @_ ;
    my ($ofs,$buffer) = (0,"") ;
    my $n = read($fh,$buffer,0x1000,$ofs) ;
    while ($n > 0) {
        $ofs += $n ;
        $n = read($fh,$buffer,0x1000,$ofs) ;
    }
    die("read() failed:$!\n")
        if $n < 0 ;
    return $buffer ;
}

sub popen 
{
    my ($pid,$fin,$fout,$ferr) = eval {
        use Symbol qw(gensym) ;
        use IPC::Open3 ;
        my $pid = IPC::Open3::open3(my $fin,my $fout,my $ferr=Symbol::gensym(),@_);
        return ($pid,$fin,$fout,$ferr) ;
    } ;
    if ($@) {
        die("popen() failed:$@") ;
    }
    return ($pid,$fin,$fout,$ferr) ;
}

sub system2 
{
    my ($pid,$fin,$fout,$ferr) = popen(@_) ;
    close($fin) ;
    waitpid($pid, 0) ;
    my $status = $? ;
    my $sout = readEof($fout) ;
    my $serr = readEof($ferr) ;
    return ($status,$sout,$serr) ;
}

sub invoke
{
    my ($status,$sout,$serr) = system2(@_) ;
    die($serr) 
        if $status != 0 ;
    return $sout ;
}

sub invokeA
{
    my $prefix = shift ;
    my ($status,$sout,$serr) = system2(@_) ;
    die($serr) 
        if $status != 0 ;
    my @L ;
    for my $line (split('\n',$sout)) {
        push @L, $prefix . $line ;
    }
    return @L ;
}

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
    
1 ;
