# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl test.pl'

######################### We start with some black magic to print on failure.

# Change 1..1 below to 1..last_test_to_print .
# (It may become useful if the test is moved to ./t subdirectory.)

BEGIN { $| = 1; print "1..4\n"; }
END {print "not ok 1\n" unless $loaded;}
use Proc::ProcessTable;
$loaded = 1;
print "ok 1\n";

######################### End of black magic.

# Insert your test code below (better if it prints "ok 13"
# (correspondingly "not ok 13") depending on the success of chunk 13
# of the test code):

$SIG{CHLD} = sub{wait;};

$t = new Proc::ProcessTable;
if ($pid = fork){ # parent
  foreach $got ( @{$t->table} ){
    if( $got->pid == $pid ){
      print "ok 2\n";

      if( $got->kill(9) ){
	print "ok 3\n";
      }
      else{
	print "not ok 3, skipping rest of tests.\n";
	kill 9, $pid;
	exit -1;
      }

      foreach $got ( @{$t->table} ){
	if( $got->pid == $pid ){
	  print "not ok 4\n";
	  kill 9, $pid;
	  exit -1;
	}
      }
      print "ok 4\n";
      exit 0;
    }
  }

  print "not ok 2, skipping rest of tests.\n";
  exit -1;

}
else{ # child
  sleep 10000;
}
