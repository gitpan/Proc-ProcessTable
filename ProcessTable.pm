package Proc::ProcessTable;

require 5.004;

use strict;
use Carp;
use vars qw($VERSION @ISA @EXPORT @EXPORT_OK $AUTOLOAD);

require Exporter;
require DynaLoader;

@ISA = qw(Exporter DynaLoader);
# Items to export into callers namespace by default. Note: do not export
# names by default without a very good reason. Use EXPORT_OK instead.
# Do not simply export all your public functions/methods/constants.
@EXPORT = qw(
	
);
$VERSION = '0.16';

sub AUTOLOAD {
    # This AUTOLOAD is used to 'autoload' constants from the constant()
    # XS function.  If a constant is not found then control is passed
    # to the AUTOLOAD in AutoLoader.

    my $constname;
    ($constname = $AUTOLOAD) =~ s/.*:://;
    my $val = constant($constname, @_ ? $_[0] : 0);
    if ($! != 0) {
	if ($! =~ /Invalid/) {
	    $AutoLoader::AUTOLOAD = $AUTOLOAD;
	    goto &AutoLoader::AUTOLOAD;
	}
	else {
		croak "Your vendor has not defined Proc::ProcessTable macro $constname";
	}
    }
    eval "sub $AUTOLOAD { $val }";
    goto &$AUTOLOAD;
}

bootstrap Proc::ProcessTable $VERSION;

# Preloaded methods go here.
use Proc::ProcessTable::Process;
use File::Find;
use Storable qw(store retrieve);

my %TTYDEVS;
my $TTYDEVSFILE = "/tmp/TTYDEVS"; # Where we store the TTYDEVS hash

sub new 
{
  my ($this, %args) = @_;
  my $class = ref($this) || $this;
  my $self = {};
  bless $self, $class;

  if ( exists $args{cache_ttys} && $args{cache_ttys} == 1 )
  { 
    $self->{cache_ttys} = 1 
  }

  my $status = $self->initialize;
  if($status)
  {
    return $self; 
  }
  else
  {
    return undef;
  }
}

sub initialize 
{
  my ($self) = @_;

  # Get the mapping of TTYs to device nums
  # reading/writing the cache if we are caching
  if( $self->{cache_ttys} )
  {
    if( -r $TTYDEVSFILE )
    {
      $_ = retrieve($TTYDEVSFILE);
      %Proc::ProcessTable::TTYDEVS = %$_;
    }
    else
    {
      $self->_get_tty_list;
      store(\%Proc::ProcessTable::TTYDEVS, $TTYDEVSFILE);
    }
  }
  else
  {
    $self->_get_tty_list;
  }

  # Call the os-specific initialization
  $self->_initialize_os;

  return 1; 
}

###############################################
# Generate a hash mapping TTY numbers to paths.
# This might be faster in Table.xs,
# but it's a lot more portable here
###############################################
sub _get_tty_list 
{
  my ($self) = @_;
  undef %Proc::ProcessTable::TTYDEVS;
  find(
       sub{
	 my($dev,$ino,$mode,$nlink,$uid,$gid,$rdev,$size,
	    $atime,$mtime,$ctime,$blksize,$blocks) = stat($File::Find::name);
	 $Proc::ProcessTable::TTYDEVS{$rdev} = $File::Find::name if($rdev);
       },
       "/dev" 
      );
}

# Apparently needed for mod_perl
sub DESTROY {}

1;
__END__

=head1 NAME

Proc::ProcessTable - Perl extension to access the unix process table

=head1 SYNOPSIS

  use ProcessTable;

  $p = new Proc::ProcessTable( 'cache_ttys' => 1 ); 
  $ref = $p->table;

=head1 DESCRIPTION

Perl interface to the unix process table.

=head1 METHODS

=over 4

=item new

Creates a new ProcessTable object. The constructor can take one flag:

cache_ttys -- causes the constructor to look for and use a file that
caches a mapping of tty names to device numbers, and to create the
file if it doesn't exist (this file is /tmp/TTYDEVS by default). This
feature requires the Storable module.

=item table

Reads the process table and returns a reference to an array of
Proc::ProcessTable::Process objects. Attributes of a process object
are returned by accessors named for the attribute; for example, to get
the uid of a process just do:

$process->uid

The priority and pgrp methods also allow values to be set, since these
are supported directly by internal perl functions.

=back

=head1 EXAMPLES

 # A cheap and sleazy version of ps
 use Proc::ProcessTable;

 $FORMAT = "%-6s %-10s %-8s %-24s %s\n";
 $t = new Proc::ProcessTable;
 printf($FORMAT, "PID", "TTY", "STAT", "START", "COMMAND"); 
 foreach $p ( @{$t->table} ){
   printf($FORMAT, 
          $p->pid, 
          $p->ttydev, 
          $p->state, 
          scalar(localtime($p->start)), 
          $p->cmndline);
 }

=head1 CAVEATS

Please see the file README in the distribution for a list of supported
operating systems. Please see the file PORTING for information on how
to help make this work on your OS.

=head1 AUTHOR

D. Urist, durist@world.std.com

=head1 SEE ALSO

Proc::ProcessTable::Process.pm, perl(1).

=cut
