#!/usr/bin/perl -w
# Copyright (C) 1999, 2001 Free Software Foundation, Inc.
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
# License along with the GNU C Library; if not, write to the Free
# Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
# 02111-1307 USA.

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
use vars qw (%results @all_floats %suffices @all_functions);


# all_floats is in output order and contains all recognised float types that
# we're going to output
@all_floats = ('float', 'double', 'ldouble');
%suffices =
  ( 'float' => 'f',
    'double' => '',
    'ldouble' => 'l'
  );

# Pretty description of platform
%pplatforms =
  ( "i386/fpu" => "ix86",
    "generic" => "Generic",
    "alpha/fpu" => "Alpha",
    "ia64/fpu" => "IA64",
    "m68k/fpu" => "M68k",
    "mips/fpu" => "MIPS",
    "powerpc/fpu" => "PowerPC",
    "sparc/sparc32/fpu" => "Sparc 32-bit",
    "sparc/sparc64/fpu" => "Sparc 64-bit",
    "sh/sh4/fpu" => "SH4",
    "s390/fpu" => "S/390",
    "arm" => "ARM"
  );

@all_functions =
  ( "acos", "acosh", "asin", "asinh", "atan", "atanh",
    "atan2", "cabs", "cacos", "cacosh", "carg", "casin", "casinh",
    "catan", "catanh", "cbrt", "ccos", "ccosh", "ceil", "cexp", "cimag",
    "clog", "clog10", "conj", "copysign", "cos", "cosh", "cpow", "cproj",
    "creal", "csin", "csinh", "csqrt", "ctan", "ctanh", "erf", "erfc",
    "exp", "exp10", "exp2", "expm1", "fabs", "fdim", "floor", "fma",
    "fmax", "fmin", "fmod", "frexp", "gamma", "hypot",
    "ilogb", "j0", "j1", "jn", "lgamma", "lrint",
    "llrint", "log", "log10", "log1p", "log2", "logb", "lround",
    "llround", "modf", "nearbyint", "nextafter", "nexttoward", "pow",
    "remainder", "remquo", "rint", "round", "scalb", "scalbn", "scalbln",
    "sin", "sincos", "sinh", "sqrt", "tan", "tanh", "tgamma",
    "trunc", "y0", "y1", "yn" );
# "fpclassify", "isfinite", "isnormal", "signbit" are not tabulated

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
    &parse_ulps ($File::Find::name, $File::Find::dir);
  }
}

# Parse ulps file
sub parse_ulps {
  my ($file, $platform) = @_;
  my ($test, $type, $float, $eps, $kind);

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
    if (/^Test/) {
      $kind = 'test';
      next;
    }
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
      $kind = 'fct';
      next;
    }
    # Only handle maximal errors of functions
    next if ($kind eq 'test');
    if (/^i?(float|double|ldouble):/) {
      ($float, $eps) = split /\s*:\s*/,$_,2;
      if ($eps eq 'fail') {
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

sub canonicalize_platform {
  my ($platform) = @_;

  $platform =~ s|^(.*/sysdeps/)||;


  return exists $pplatforms{$platform} ? $pplatforms{$platform} : $platform;
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
    print &canonicalize_platform ($_);
  }
  print "\n";


  foreach $fct (@all_functions) {
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
  my ($pa, $pb);

  $pa = $pplatforms{$a} ? $pplatforms{$a} : $a;
  $pb = $pplatforms{$b} ? $pplatforms{$b} : $b;
  
  return $pa cmp $pb;
}
