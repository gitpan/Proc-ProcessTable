symlink "os/IRIX.c", "OS.c" || die "Could not link os/IRIX.c to os/OS.c\n";

# If this OS version supports the new /proc filesystem, use it; 
# otherwise default to ioctl-proc
#`uname -r` =~ /^(\d+\.\d+)/;
#if( $1 > 5.5 ){
#    $self->{DEFINE} = "-DPROC_FS";
#}
