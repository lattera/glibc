#!/usr/bin/perl -w
# Copyright (C) 1999-2018 Free Software Foundation, Inc.
# This file is part of the GNU C Library.
# Contributed by Andreas Jaeger <aj@suse.de>, 1999.

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

# Information about tests are stored in: %results
# $results{$test}{"type"} is the result type, e.g. normal or complex.
# In the following description $platform, $type and $float are:
# - $platform is the used platform
# - $type is either "normal", "real" (for the real part of a complex number)
#   or "imag" (for the imaginary part # of a complex number).
# - $float is either of float, ifloat, double, idouble, ldouble, ildouble;
#   It represents the underlying floating point type (float, double or long
#   double) and if inline functions (the leading i stands for inline)
#   are used.
# $results{$test}{$platform}{$type}{$float} is defined and has a delta
# or 'fail' as value.

use File::Find;

use strict;

use vars qw ($sources @platforms %pplatforms);
use vars qw (%results @all_floats %suffices %all_functions);


# all_floats is in output order and contains all recognised float types that
# we're going to output
@all_floats = ('float', 'double', 'ldouble', 'float128');
%suffices =
  ( 'float' => 'f',
    'double' => '',
    'ldouble' => 'l',
    'float128' => 'f128'
  );

# Pretty description of platform
%pplatforms = ();

%all_functions = ();

if ($#ARGV == 0) {
  $sources = $ARGV[0];
} else {
  $sources = '/usr/src/cvs/libc';
}

find (\&find_files, $sources);

@platforms = sort by_platforms @platforms;

&print_all;

sub find_files {
  if ($_ eq 'libm-test-ulps') {
    # print "Parsing $File::Find::name\n";
    push @platforms, $File::Find::dir;
    my ($file, $name);
    $file = "${File::Find::name}-name";
    open NAME, $file or die ("Can't open $file: $!");
    $name = <NAME>;
    chomp $name;
    close NAME;
    $pplatforms{$File::Find::dir} = $name;
    &parse_ulps ($File::Find::name, $File::Find::dir);
  }
}

# Parse ulps file
sub parse_ulps {
  my ($file, $platform) = @_;
  my ($test, $type, $float, $eps, $ignore_fn);

  # $type has the following values:
  # "normal": No complex variable
  # "real": Real part of complex result
  # "imag": Imaginary part of complex result
  open ULP, $file  or die ("Can't open $file: $!");
  while (<ULP>) {
    chop;
    # ignore comments and empty lines
    next if /^#/;
    next if /^\s*$/;
    if (/^Function: /) {
      if (/Real part of/) {
	s/Real part of //;
	$type = 'real';
      } elsif (/Imaginary part of/) {
	s/Imaginary part of //;
	$type = 'imag';
      } else {
	$type = 'normal';
      }
      ($test) = ($_ =~ /^Function:\s*\"([a-zA-Z0-9_]+)\"/);
      next;
    }
    if ($test =~ /_(downward|towardzero|upward|vlen)/) {
      $ignore_fn = 1;
    } else {
      $ignore_fn = 0;
      $all_functions{$test} = 1;
    }
    if (/^i?(float|double|ldouble|float128):/) {
      ($float, $eps) = split /\s*:\s*/,$_,2;
      if ($ignore_fn) {
	next;
      } elsif ($eps eq 'fail') {
	$results{$test}{$platform}{$type}{$float} = 'fail';
      } elsif ($eps eq "0") {
	# ignore
	next;
      } elsif (!exists $results{$test}{$platform}{$type}{$float}
	    || $results{$test}{$platform}{$type}{$float} ne 'fail') {
	$results{$test}{$platform}{$type}{$float} = $eps;
      }
      if ($type =~ /^real|imag$/) {
	$results{$test}{'type'} = 'complex';
      } elsif ($type eq 'normal') {
	$results{$test}{'type'} = 'normal';
      }
      next;
    }
    print "Skipping unknown entry: `$_'\n";
  }
  close ULP;
}

sub get_value {
  my ($fct, $platform, $type, $float) = @_;

  return (exists $results{$fct}{$platform}{$type}{$float}
	  ? $results{$fct}{$platform}{$type}{$float} : "0");
}

sub print_platforms {
  my (@p) = @_;
  my ($fct, $platform, $float, $first, $i, $platform_no, $platform_total);

  print '@multitable {nexttowardf} ';
  foreach (@p) {
    print ' {1000 + i 1000}';
  }
  print "\n";

  print '@item Function ';
  foreach (@p) {
    print ' @tab ';
    print $pplatforms{$_};
  }
  print "\n";


  foreach $fct (sort keys %all_functions) {
    foreach $float (@all_floats) {
      print "\@item $fct$suffices{$float} ";
      foreach $platform (@p) {
	print ' @tab ';
	if (exists $results{$fct}{$platform}{'normal'}{$float}
	    || exists $results{$fct}{$platform}{'real'}{$float}
	    || exists $results{$fct}{$platform}{'imag'}{$float}) {
	  if ($results{$fct}{'type'} eq 'complex') {
	    print &get_value ($fct, $platform, 'real', $float),
	    ' + i ', &get_value ($fct, $platform, 'imag', $float);
	  } else {
	    print $results{$fct}{$platform}{'normal'}{$float};
	  }
	} else {
	  print '-';
	}
      }
      print "\n";
    }
  }

  print "\@end multitable\n";
}

sub print_all {
  my ($i, $max);

  my ($columns) = 5;

  # Print only 5 platforms at a time.
  for ($i=0; $i < $#platforms; $i+=$columns) {
    $max = $i+$columns-1 > $#platforms ? $#platforms : $i+$columns-1;
    print_platforms (@platforms[$i .. $max]);
  }
}

sub by_platforms {
  return $pplatforms{$a} cmp $pplatforms{$b};
}
