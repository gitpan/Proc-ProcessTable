symlink "os/Solaris.c", "OS.c" || die "Could not link os/Solaris.c to os/OS.c\n";

`uname -r` =~ /^(\d+\.\d+)/;
if( $1 > 2.5 ){
    $self->{DEFINE} = "-DPROC_FS";
}
