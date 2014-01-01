#! /usr/bin/perl -w
# Copyright (C) 2013-2014 Free Software Foundation, Inc.
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

if (@ARGV < 1) {
  die "Usage: bench.pl <function>"
}

my $func = $ARGV[0];
my @args;
my $ret = "void";
my $getret = "";

# We create a hash of inputs for each variant of the test.
my $variant = "";
my @curvals;
my %vals;
my @include_headers;
my @include_sources;
my $incl;

open INPUTS, "<$func-inputs" or die $!;

LINE:while (<INPUTS>) {
  chomp;

  # Directives.
  if (/^## ([\w-]+): (.*)/) {
    # Function argument types.
    if ($1 eq "args") {
      @args = split(":", $2);
    }

    # Function return type.
    elsif ($1 eq "ret") {
      $ret = $2;
    }

    elsif ($1 eq "includes") {
      @include_headers = split (",", $2);
    }

    elsif ($1 eq "include-sources") {
      @include_sources = split (",", $2);
    }

    # New variant.  This is the only directive allowed in the body of the
    # inputs to separate inputs into variants.  All others should be at the
    # top or else all hell will break loose.
    elsif ($1 eq "name") {

      # Save values in the previous variant.
      my @copy = @curvals;
      $vals{$variant} = \@copy;

      # Prepare for the next.
      $variant=$2;
      undef @curvals;
      next LINE;
    }

    else {
      die "Unknown directive: ".$1;
    }
  }

  # Skip over comments and blank lines.
  if (/^#/ || /^$/) {
    next LINE;
  }
  push (@curvals, $_);
}


my $bench_func = "#define CALL_BENCH_FUNC(v, i) $func (";

# Output variables.  These include the return value as well as any pointers
# that may get passed into the function, denoted by the <> around the type.
my $outvars = "";

if ($ret ne "void") {
  $outvars = "static $ret volatile ret;\n";
}

# Print the definitions and macros.
foreach $incl (@include_headers) {
  print "#include <" . $incl . ">\n";
}

# Print the source files.
foreach $incl (@include_sources) {
  print "#include \"" . $incl . "\"\n";
}

if (@args > 0) {
  # Save values in the last variant.
  $vals{$variant} = \@curvals;
  my $struct =
    "struct _variants
    {
      const char *name;
      int count;
      struct args *in;
    };\n";

  my $arg_struct = "struct args {";

  my $num = 0;
  my $arg;
  foreach $arg (@args) {
    if ($num > 0) {
      $bench_func = "$bench_func,";
    }

    $_ = $arg;
    if (/<(.*)\*>/) {
      # Output variables.  These have to be pointers, so dereference once by
      # dropping one *.
      $outvars = $outvars . "static $1 out$num;\n";
      $bench_func = "$bench_func &out$num";
    }
    else {
      $arg_struct = "$arg_struct $arg volatile arg$num;";
      $bench_func = "$bench_func variants[v].in[i].arg$num";
    }

    $num = $num + 1;
  }

  $arg_struct = $arg_struct . "};\n";
  $bench_func = $bench_func . ");\n";

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
else {
  print $bench_func . ");\n";
  print "#define NUM_VARIANTS (1)\n";
  print "#define NUM_SAMPLES(v) (1)\n";
  print "#define VARIANT(v) FUNCNAME \"()\"\n"
}

# Print the output variable definitions.
print "$outvars\n";

# In some cases not storing a return value seems to result in the function call
# being optimized out.
if ($ret ne "void") {
  $getret = "ret = ";
}

# And we're done.
print "#define BENCH_FUNC(i, j) ({$getret CALL_BENCH_FUNC (i, j);})\n";

print "#define FUNCNAME \"$func\"\n";
print "#include \"bench-skeleton.c\"\n";
