#!/usr/bin/perl

use Proc::ProcessTable;

$ref = new Proc::ProcessTable;

foreach $proc (@{$ref->table}) {
  print "--------------------------------\n";
  foreach $field ($ref->fields){
    print $field, ":  ", $proc->{$field}, "\n";
  }
}
