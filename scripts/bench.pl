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
  die "Usage: bench.pl <function> [parameter types] [return type]"
}

my $arg;
my $func = $ARGV[0];
my @args;
my $ret = "void";
my $getret = "";
my $retval = "";

if (@ARGV >= 2) {
  @args = split(':', $ARGV[1]);
}

if (@ARGV == 3) {
  $ret = $ARGV[2];
}

my $decl = "extern $ret $func (";

# Function has no arguments.
if (@args == 0 || $args[0] eq "void") {
  print "$decl void);\n";
  print "#define CALL_BENCH_FUNC(i,j) $func();\n";
  print "#define NUM_VARIANTS (1)\n";
  print "#define NUM_SAMPLES(v) (1)\n";
  print "#define VARIANT(v) FUNCNAME \"()\"\n"
}
# The function has arguments, so parse them and populate the inputs.
else {
  my $num = 0;
  my $bench_func = "#define CALL_BENCH_FUNC(v, i) $func (";

  my $struct =
    "struct _variants
    {
      const char *name;
      int count;
      struct args *in;
    };\n";

  my $arg_struct = "struct args {";

  foreach $arg (@args) {
    if ($num > 0) {
      $bench_func = "$bench_func,";
      $decl = "$decl,";
    }

    $arg_struct = "$arg_struct volatile $arg arg$num;";
    $bench_func = "$bench_func variants[v].in[i].arg$num";
    $decl = "$decl $arg";
    $num = $num + 1;
  }

  $arg_struct = $arg_struct . "};\n";
  $decl = $decl . ");\n";
  $bench_func = $bench_func . ");\n";

  # We create a hash of inputs for each variant of the test.
  my $variant = "";
  my @curvals;
  my %vals;

  open INPUTS, "<$func-inputs" or die $!;

  LINE:while (<INPUTS>) {
    chomp;

    # New variant.
    if (/^## (\w+): (\w+)/) {
      #We only identify Name for now.
      if ($1 ne "name") {
        next LINE;
      }

      # Save values in the last variant.
      my @copy = @curvals;
      $vals{$variant} = \@copy;

      # Prepare for the next.
      $variant=$2;
      undef @curvals;
      next LINE;
    }

    # Skip over comments.
    if (/^#/) {
      next LINE;
    }
    push (@curvals, $_);
  }

  $vals{$variant} = \@curvals;

  # Print the definitions and macros.
  print $decl;
  print $bench_func;
  print $arg_struct;
  print $struct;

  my $c = 0;
  my $key;

  # Print the input arrays.
  foreach $key (keys %vals) {
    my @arr = @{$vals{$key}};

    print "struct args in" . $c . "[" . @arr . "] = {\n";
    foreach (@arr) {
      print "{$_},\n";
    }
    print "};\n\n";
    $c += 1;
  }

  # The variants.  Each variant then points to the appropriate input array we
  # defined above.
  print "struct _variants variants[" . (keys %vals) . "] = {\n";
  $c = 0;
  foreach $key (keys %vals) {
    print "{\"$func($key)\", " . @{$vals{$key}} . ", in$c},\n";
    $c += 1;
  }
  print "};\n\n";

  # Finally, print the last set of macros.
  print "#define NUM_VARIANTS $c\n";
  print "#define NUM_SAMPLES(i) (variants[i].count)\n";
  print "#define VARIANT(i) (variants[i].name)\n";
}

# In some cases not storing a return value seems to result in the function call
# being optimized out.
if ($ret ne "void") {
  print "static volatile $ret ret = 0.0;\n";
  $getret = "ret = ";
}

# And we're done.
print "#define BENCH_FUNC(i, j) ({$getret CALL_BENCH_FUNC (i, j);})\n";

print "#define FUNCNAME \"$func\"\n";
print "#include \"bench-skeleton.c\"\n";
