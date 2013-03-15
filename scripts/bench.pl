#! /usr/bin/perl -w
# Copyright (C) 2013 Free Software Foundation, Inc.
# This file is part of the GNU C Library.

# The GNU C Library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.

# The GNU C Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.

# You should have received a copy of the GNU Lesser General Public
# License along with the GNU C Library; if not, see
# <http://www.gnu.org/licenses/>.


use strict;
use warnings;
# Generate a benchmark source file for a given input.

if (@ARGV < 2) {
  die "Usage: bench.pl <function> <iterations> [parameter types] [return type]"
}

my $arg;
my $func = $ARGV[0];
my $iters = $ARGV[1];
my @args;
my $ret = "void";
my $getret = "";
my $retval = "";

if (@ARGV >= 3) {
  @args = split(':', $ARGV[2]);
}

if (@ARGV == 4) {
  $ret = $ARGV[3];
}

my $decl = "extern $ret $func (";

if (@args == 0 || $args[0] eq "void") {
  print "$decl void);\n";
  print "#define CALL_BENCH_FUNC(j) $func();\n";
  print "#define NUM_SAMPLES (1)\n";
}
else {
  my $num = 0;
  my $bench_func = "#define CALL_BENCH_FUNC(j) $func (";
  my $struct = "struct args {";

  foreach $arg (@args) {
    if ($num > 0) {
      $bench_func = "$bench_func,";
      $decl = "$decl,";
    }

    $struct = "$struct $arg arg$num;";
    $bench_func = "$bench_func in[j].arg$num";
    $decl = "$decl $arg";
    $num = $num + 1;
  }

  print "$decl);\n";
  print "$bench_func);\n";
  print "$struct } in[] = {";

  open INPUTS, "<$func-inputs" or die $!;

  while (<INPUTS>) {
    chomp;
    print "{$_},\n";
  }
  print "};\n";
  print "#define NUM_SAMPLES (sizeof (in) / sizeof (struct args))\n"
}

# In some cases not storing a return value seems to result in the function call
# being optimized out.
if ($ret ne "void") {
  print "static volatile $ret ret = 0.0;\n";
  $getret = "ret = ";
}

print "#define BENCH_FUNC(j) ({$getret CALL_BENCH_FUNC (j);})\n";

print "#define ITER $iters\n";
print "#define FUNCNAME \"$func\"\n";
print "#include \"bench-skeleton.c\"\n";
