#!/usr/bin/perl -w
# Copyright (C) 1999-2016 Free Software Foundation, Inc.
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

# This file needs to be tidied up
# Note that functions and tests share the same namespace.

# Information about tests are stored in: %results
# $results{$test}{"type"} is the result type, e.g. normal or complex.
# $results{$test}{"has_ulps"} is set if deltas exist.
# In the following description $type and $float are:
# - $type is either "normal", "real" (for the real part of a complex number)
#   or "imag" (for the imaginary part # of a complex number).
# - $float is either of float, ifloat, double, idouble, ldouble, ildouble;
#   It represents the underlying floating point type (float, double or long
#   double) and if inline functions (the leading i stands for inline)
#   are used.
# $results{$test}{$type}{"ulp"}{$float} is defined and has a delta as value


use Getopt::Std;

use strict;

use vars qw ($input $output $auto_input);
use vars qw (%results);
use vars qw (%beautify @all_floats %all_floats_pfx);
use vars qw ($output_dir $ulps_file $srcdir);
use vars qw (%auto_tests);

# all_floats is sorted and contains all recognised float types
@all_floats = ('double', 'float', 'idouble',
	       'ifloat', 'ildouble', 'ldouble');

# all_floats_pfx maps C types to their C like prefix for macros.
%all_floats_pfx =
  ( "double" => "DBL",
    "ldouble" => "LDBL",
    "float" => "FLT",
  );

%beautify =
  ( "minus_zero" => "-0",
    "plus_zero" => "+0",
    "-0x0p+0f" => "-0",
    "-0x0p+0" => "-0",
    "-0x0p+0L" => "-0",
    "0x0p+0f" => "+0",
    "0x0p+0" => "+0",
    "0x0p+0L" => "+0",
    "minus_infty" => "-inf",
    "plus_infty" => "inf",
    "qnan_value" => "qNaN",
    "snan_value" => "sNaN",
    "snan_value_ld" => "sNaN",
  );


# get Options
# Options:
# u: ulps-file
# h: help
# o: output-directory
# n: generate new ulps file
use vars qw($opt_u $opt_h $opt_o $opt_n);
getopts('u:o:nh');

$ulps_file = 'libm-test-ulps';
$output_dir = '';
($srcdir = $0) =~ s{[^/]*$}{};

if ($opt_h) {
  print "Usage: gen-libm-test.pl [OPTIONS]\n";
  print " -h         print this help, then exit\n";
  print " -o DIR     directory where generated files will be placed\n";
  print " -n         only generate sorted file NewUlps from libm-test-ulps\n";
  print " -u FILE    input file with ulps\n";
  exit 0;
}

$ulps_file = $opt_u if ($opt_u);
$output_dir = $opt_o if ($opt_o);

$input = "libm-test.inc";
$auto_input = "${srcdir}auto-libm-test-out";
$output = "${output_dir}libm-test.c";

&parse_ulps ($ulps_file);
&parse_auto_input ($auto_input);
&generate_testfile ($input, $output) unless ($opt_n);
&output_ulps ("${output_dir}libm-test-ulps.h", $ulps_file) unless ($opt_n);
&print_ulps_file ("${output_dir}NewUlps") if ($opt_n);

# Return a nicer representation
sub beautify {
  my ($arg) = @_;
  my ($tmp);

  if (exists $beautify{$arg}) {
    return $beautify{$arg};
  }
  if ($arg =~ /^-/) {
    $tmp = $arg;
    $tmp =~ s/^-//;
    if (exists $beautify{$tmp}) {
      return '-' . $beautify{$tmp};
    }
  }
  if ($arg =~ /^-?0x[0-9a-f.]*p[-+][0-9]+f$/) {
    $arg =~ s/f$//;
  }
  if ($arg =~ /[0-9]L$/) {
    $arg =~ s/L$//;
  }
  return $arg;
}

# Return a nicer representation of a complex number
sub build_complex_beautify {
  my ($r, $i) = @_;
  my ($str1, $str2);

  $str1 = &beautify ($r);
  $str2 = &beautify ($i);
  if ($str2 =~ /^-/) {
    $str2 =~ s/^-//;
    $str1 .= ' - ' . $str2;
  } else {
    $str1 .= ' + ' . $str2;
  }
  $str1 .= ' i';
  return $str1;
}

# Return the text to put in an initializer for a test's exception
# information.
sub show_exceptions {
  my ($ignore_result, $non_finite, $test_snan, $exception) = @_;
  $ignore_result = ($ignore_result ? "IGNORE_RESULT|" : "");
  $non_finite = ($non_finite ? "NON_FINITE|" : "");
  $test_snan = ($test_snan ? "TEST_SNAN|" : "");
  if (defined $exception) {
    return ", ${ignore_result}${non_finite}${test_snan}$exception";
  } else {
    return ", ${ignore_result}${non_finite}${test_snan}0";
  }
}

# Apply the LIT(x) macro to a literal floating point constant
# and strip any existing suffix.
sub _apply_lit {
  my ($lit) = @_;
  my $exp_re = "([+-])?[[:digit:]]+";
  # Don't wrap something that does not look like a:
  #  * Hexadecimal FP value
  #  * Decimal FP value without a decimal point
  #  * Decimal value with a fraction
  return $lit if $lit !~ /([+-])?0x[[:xdigit:]\.]+[pP]$exp_re/
		 and $lit !~ /[[:digit:]]+[eE]$exp_re/
		 and $lit !~ /[[:digit:]]*\.[[:digit:]]*([eE]$exp_re)?/;

  # Strip any existing literal suffix.
  $lit =~ s/[lLfF]$//;

  return "LIT (${lit})";
}

# Apply LIT macro to individual tokens within an expression.
#
# This function assumes the C expression follows GNU coding
# standards.  Specifically, a space separates each lexical
# token.  Otherwise, this post-processing may apply LIT
# incorrectly, or around an entire expression.
sub apply_lit {
  my ($lit) = @_;
  my @toks = split (/ /, $lit);
  foreach (@toks) {
    $_ = _apply_lit ($_);
  }
  return join (' ', @toks);
}

# Parse the arguments to TEST_x_y
sub parse_args {
  my ($file, $descr, $args) = @_;
  my (@args, $descr_args, $descr_res, @descr);
  my ($current_arg, $cline, $cline_res, $i);
  my (@special);
  my ($call_args);
  my ($ignore_result_any, $ignore_result_all);
  my ($num_res, @args_res, @start_rm, $rm);
  my (@plus_oflow, @minus_oflow, @plus_uflow, @minus_uflow);
  my (@errno_plus_oflow, @errno_minus_oflow);
  my (@errno_plus_uflow, @errno_minus_uflow);
  my ($non_finite, $test_snan);

  ($descr_args, $descr_res) = split /_/,$descr, 2;

  @args = split /,\s*/, $args;

  $call_args = "";

  # Generate first the string that's shown to the user
  $current_arg = 1;
  @descr = split //,$descr_args;
  for ($i = 0; $i <= $#descr; $i++) {
    my $comma = "";
    if ($current_arg > 1) {
      $comma = ', ';
    }
    # FLOAT, int, long int, long long int
    if ($descr[$i] =~ /f|j|i|l|L/) {
      $call_args .= $comma . &beautify ($args[$current_arg]);
      ++$current_arg;
      next;
    }
    # Argument passed via pointer.
    if ($descr[$i] =~ /p/) {
      next;
    }
    # &FLOAT, &int - simplify call by not showing argument.
    if ($descr[$i] =~ /F|I/) {
      next;
    }
    # complex
    if ($descr[$i] eq 'c') {
      $call_args .= $comma . &build_complex_beautify ($args[$current_arg], $args[$current_arg+1]);
      $current_arg += 2;
      next;
    }

    die ("$descr[$i] is unknown");
  }

  # Result
  @args_res = @args[$current_arg .. $#args];
  $num_res = 0;
  @descr = split //,$descr_res;
  foreach (@descr) {
    if ($_ =~ /f|i|l|L/) {
      ++$num_res;
    } elsif ($_ eq 'c') {
      $num_res += 2;
    } elsif ($_ eq 'b') {
      # boolean
      ++$num_res;
    } elsif ($_ eq '1') {
      ++$num_res;
    } else {
      die ("$_ is unknown");
    }
  }
  # consistency check
  if ($#args_res == $num_res - 1) {
    # One set of results for all rounding modes, no flags.
    @start_rm = ( 0, 0, 0, 0 );
  } elsif ($#args_res == $num_res) {
    # One set of results for all rounding modes, with flags.
    die ("wrong number of arguments")
      unless ($args_res[$#args_res] =~ /EXCEPTION|ERRNO|IGNORE_ZERO_INF_SIGN|TEST_NAN_SIGN|NO_TEST_INLINE|XFAIL_TEST/);
    @start_rm = ( 0, 0, 0, 0 );
  } elsif ($#args_res == 4 * $num_res + 3) {
    # One set of results per rounding mode, with flags.
    @start_rm = ( 0, $num_res + 1, 2 * $num_res + 2, 3 * $num_res + 3 );
  } else {
    die ("wrong number of arguments");
  }

  # Put the C program line together
  # Reset some variables to start again
  $current_arg = 1;
  $call_args =~ s/\"/\\\"/g;
  $cline = "{ \"$call_args\"";
  @descr = split //,$descr_args;
  for ($i=0; $i <= $#descr; $i++) {
    # FLOAT, int, long int, long long int
    if ($descr[$i] =~ /f|j|i|l|L/) {
      if ($descr[$i] eq "f") {
        $cline .= ", " . &apply_lit ($args[$current_arg]);
      } else {
        $cline .= ", $args[$current_arg]";
      }
      $current_arg++;
      next;
    }
    # &FLOAT, &int, argument passed via pointer
    if ($descr[$i] =~ /F|I|p/) {
      next;
    }
    # complex
    if ($descr[$i] eq 'c') {
      $cline .= ", " . &apply_lit ($args[$current_arg]);
      $cline .= ", " . &apply_lit ($args[$current_arg+1]);
      $current_arg += 2;
      next;
    }
  }

  @descr = split //,$descr_res;
  @plus_oflow = qw(max_value plus_infty max_value plus_infty);
  @minus_oflow = qw(minus_infty minus_infty -max_value -max_value);
  @plus_uflow = qw(plus_zero plus_zero plus_zero min_subnorm_value);
  @minus_uflow = qw(-min_subnorm_value minus_zero minus_zero minus_zero);
  @errno_plus_oflow = qw(0 ERRNO_ERANGE 0 ERRNO_ERANGE);
  @errno_minus_oflow = qw(ERRNO_ERANGE ERRNO_ERANGE 0 0);
  @errno_plus_uflow = qw(ERRNO_ERANGE ERRNO_ERANGE ERRNO_ERANGE 0);
  @errno_minus_uflow = qw(0 ERRNO_ERANGE ERRNO_ERANGE ERRNO_ERANGE);
  for ($rm = 0; $rm <= 3; $rm++) {
    $current_arg = $start_rm[$rm];
    $ignore_result_any = 0;
    $ignore_result_all = 1;
    $cline_res = "";
    @special = ();
    foreach (@descr) {
      if ($_ =~ /b|f|j|i|l|L/ ) {
	my ($result) = $args_res[$current_arg];
	if ($result eq "IGNORE") {
	  $ignore_result_any = 1;
	  $result = "0";
	} else {
	  $ignore_result_all = 0;
	}
	if ($_ eq "f") {
	  $result = apply_lit ($result);
	}
	$cline_res .= ", $result";
	$current_arg++;
      } elsif ($_ eq 'c') {
	my ($result1) = $args_res[$current_arg];
	if ($result1 eq "IGNORE") {
	  $ignore_result_any = 1;
	  $result1 = "0";
	} else {
	  $ignore_result_all = 0;
	}
	my ($result2) = $args_res[$current_arg + 1];
	if ($result2 eq "IGNORE") {
	  $ignore_result_any = 1;
	  $result2 = "0";
	} else {
	  $ignore_result_all = 0;
	}
	$result1 = apply_lit ($result1);
	$result2 = apply_lit ($result2);
	$cline_res .= ", $result1, $result2";
	$current_arg += 2;
      } elsif ($_ eq '1') {
	push @special, $args_res[$current_arg];
	++$current_arg;
      }
    }
    if ($ignore_result_any && !$ignore_result_all) {
      die ("some but not all function results ignored\n");
    }
    # Determine whether any arguments or results, for any rounding
    # mode, are non-finite.
    $non_finite = ($args =~ /qnan_value|snan_value|plus_infty|minus_infty/);
    $test_snan = ($args =~ /snan_value/);
    # Add exceptions.
    $cline_res .= show_exceptions ($ignore_result_any,
				   $non_finite,
				   $test_snan,
				   ($current_arg <= $#args_res)
				   ? $args_res[$current_arg]
				   : undef);

    # special treatment for some functions
    $i = 0;
    foreach (@special) {
      ++$i;
      my ($extra_expected) = $_;
      my ($run_extra) = ($extra_expected ne "IGNORE" ? 1 : 0);
      if (!$run_extra) {
	$extra_expected = "0";
      } else {
	$extra_expected = apply_lit ($extra_expected);
      }
      $cline_res .= ", $run_extra, $extra_expected";
    }
    $cline_res =~ s/^, //;
    $cline_res =~ s/plus_oflow/$plus_oflow[$rm]/g;
    $cline_res =~ s/minus_oflow/$minus_oflow[$rm]/g;
    $cline_res =~ s/plus_uflow/$plus_uflow[$rm]/g;
    $cline_res =~ s/minus_uflow/$minus_uflow[$rm]/g;
    $cline_res =~ s/ERRNO_PLUS_OFLOW/$errno_plus_oflow[$rm]/g;
    $cline_res =~ s/ERRNO_MINUS_OFLOW/$errno_minus_oflow[$rm]/g;
    $cline_res =~ s/ERRNO_PLUS_UFLOW/$errno_plus_uflow[$rm]/g;
    $cline_res =~ s/ERRNO_MINUS_UFLOW/$errno_minus_uflow[$rm]/g;
    $cline .= ", { $cline_res }";
  }
  print $file "    $cline },\n";
}

# Convert a condition from auto-libm-test-out to C form.
sub convert_condition {
  my ($cond) = @_;
  my (@conds, $ret);
  @conds = split /:/, $cond;
  foreach (@conds) {
    s/-/_/g;
    s/^/TEST_COND_/;
  }
  $ret = join " && ", @conds;
  return "($ret)";
}

# Return text to OR a value into an accumulated flags string.
sub or_value {
  my ($cond) = @_;
  if ($cond eq "0") {
    return "";
  } else {
    return " | $cond";
  }
}

# Return a conditional expression between two values.
sub cond_value {
  my ($cond, $if, $else) = @_;
  if ($cond eq "1") {
    return $if;
  } elsif ($cond eq "0") {
    return $else;
  } else {
    return "($cond ? $if : $else)";
  }
}

# Return text to OR a conditional expression between two values into
# an accumulated flags string.
sub or_cond_value {
  my ($cond, $if, $else) = @_;
  return or_value (cond_value ($cond, $if, $else));
}

# Generate libm-test.c
sub generate_testfile {
  my ($input, $output) = @_;

  open INPUT, $input or die ("Can't open $input: $!");
  open OUTPUT, ">$output" or die ("Can't open $output: $!");

  # Replace the special macros
  while (<INPUT>) {
    # AUTO_TESTS (function),
    if (/^\s*AUTO_TESTS_/) {
      my ($descr, $func, @modes, $auto_test, $num_auto_tests);
      my (@rm_tests, $rm, $i);
      @modes = qw(downward tonearest towardzero upward);
      ($descr, $func) = ($_ =~ /AUTO_TESTS_(\w+)\s*\((\w+)\)/);
      for ($rm = 0; $rm <= 3; $rm++) {
	$rm_tests[$rm] = [sort keys %{$auto_tests{$func}{$modes[$rm]}}];
      }
      $num_auto_tests = scalar @{$rm_tests[0]};
      for ($rm = 1; $rm <= 3; $rm++) {
	if ($num_auto_tests != scalar @{$rm_tests[$rm]}) {
	  die ("inconsistent numbers of tests for $func\n");
	}
	for ($i = 0; $i < $num_auto_tests; $i++) {
	  if ($rm_tests[0][$i] ne $rm_tests[$rm][$i]) {
	    die ("inconsistent list of tests of $func\n");
	  }
	}
      }
      if ($num_auto_tests == 0) {
	die ("no automatic tests for $func\n");
      }
      foreach $auto_test (@{$rm_tests[0]}) {
	my ($format, $inputs, $format_conv, $args_str);
	($format, $inputs) = split / /, $auto_test, 2;
	$inputs =~ s/ /, /g;
	$format_conv = convert_condition ($format);
	print OUTPUT "#if $format_conv\n";
	$args_str = "$func, $inputs";
	for ($rm = 0; $rm <= 3; $rm++) {
	  my ($auto_test_out, $outputs, $flags);
	  my ($flags_conv, @flags, %flag_cond);
	  $auto_test_out = $auto_tests{$func}{$modes[$rm]}{$auto_test};
	  ($outputs, $flags) = split / : */, $auto_test_out;
	  $outputs =~ s/ /, /g;
	  @flags = split / /, $flags;
	  foreach (@flags) {
	    if (/^([^:]*):(.*)$/) {
	      my ($flag, $cond);
	      $flag = $1;
	      $cond = convert_condition ($2);
	      if (defined ($flag_cond{$flag})) {
		if ($flag_cond{$flag} ne "1") {
		  $flag_cond{$flag} .= " || $cond";
		}
	      } else {
		$flag_cond{$flag} = $cond;
	      }
	    } else {
	      $flag_cond{$_} = "1";
	    }
	  }
	  $flags_conv = "";
	  if (defined ($flag_cond{"ignore-zero-inf-sign"})) {
	    $flags_conv .= or_cond_value ($flag_cond{"ignore-zero-inf-sign"},
					  "IGNORE_ZERO_INF_SIGN", "0");
	  }
	  if (defined ($flag_cond{"no-test-inline"})) {
	    $flags_conv .= or_cond_value ($flag_cond{"no-test-inline"},
					  "NO_TEST_INLINE", "0");
	  }
	  if (defined ($flag_cond{"xfail"})) {
	    $flags_conv .= or_cond_value ($flag_cond{"xfail"},
					  "XFAIL_TEST", "0");
	  }
	  my (@exc_list) = qw(divbyzero inexact invalid overflow underflow);
	  my ($exc);
	  foreach $exc (@exc_list) {
	    my ($exc_expected, $exc_ok, $no_exc, $exc_cond, $exc_ok_cond);
	    $exc_expected = "\U$exc\E_EXCEPTION";
	    $exc_ok = "\U$exc\E_EXCEPTION_OK";
	    $no_exc = "0";
	    if ($exc eq "inexact") {
	      $exc_ok = "0";
	      $no_exc = "NO_INEXACT_EXCEPTION";
	    }
	    if (defined ($flag_cond{$exc})) {
	      $exc_cond = $flag_cond{$exc};
	    } else {
	      $exc_cond = "0";
	    }
	    if (defined ($flag_cond{"$exc-ok"})) {
	      $exc_ok_cond = $flag_cond{"$exc-ok"};
	    } else {
	      $exc_ok_cond = "0";
	    }
	    $flags_conv .= or_cond_value ($exc_cond,
					  cond_value ($exc_ok_cond,
						      $exc_ok, $exc_expected),
					  cond_value ($exc_ok_cond,
						      $exc_ok, $no_exc));
	  }
	  my ($errno_expected, $errno_unknown_cond);
	  if (defined ($flag_cond{"errno-edom"})) {
	    if ($flag_cond{"errno-edom"} ne "1") {
	      die ("unexpected condition for errno-edom");
	    }
	    if (defined ($flag_cond{"errno-erange"})) {
	      die ("multiple errno values expected");
	    }
	    $errno_expected = "ERRNO_EDOM";
	  } elsif (defined ($flag_cond{"errno-erange"})) {
	    if ($flag_cond{"errno-erange"} ne "1") {
	      die ("unexpected condition for errno-erange");
	    }
	    $errno_expected = "ERRNO_ERANGE";
	  } else {
	    $errno_expected = "ERRNO_UNCHANGED";
	  }
	  if (defined ($flag_cond{"errno-edom-ok"})) {
	    if (defined ($flag_cond{"errno-erange-ok"})
		&& ($flag_cond{"errno-erange-ok"}
		    ne $flag_cond{"errno-edom-ok"})) {
	      $errno_unknown_cond = "($flag_cond{\"errno-edom-ok\"} || $flag_cond{\"errno-erange-ok\"})";
	    } else {
	      $errno_unknown_cond = $flag_cond{"errno-edom-ok"};
	    }
	  } elsif (defined ($flag_cond{"errno-erange-ok"})) {
	    $errno_unknown_cond = $flag_cond{"errno-erange-ok"};
	  } else {
	    $errno_unknown_cond = "0";
	  }
	  $flags_conv .= or_cond_value ($errno_unknown_cond,
					"0", $errno_expected);
	  if ($flags_conv eq "") {
	    $flags_conv = ", NO_EXCEPTION";
	  } else {
	    $flags_conv =~ s/^ \|/,/;
	  }
	  $args_str .= ", $outputs$flags_conv";
	}
	&parse_args (\*OUTPUT, $descr, $args_str);
	print OUTPUT "#endif\n";
      }
      next;
    }

    # TEST_...
    if (/^\s*TEST_/) {
      my ($descr, $args);
      chop;
      ($descr, $args) = ($_ =~ /TEST_(\w+)\s*\((.*)\)/);
      &parse_args (\*OUTPUT, $descr, $args);
      next;
    }
    print OUTPUT;
  }
  close INPUT;
  close OUTPUT;
}



# Parse ulps file
sub parse_ulps {
  my ($file) = @_;
  my ($test, $type, $float, $eps, $float_regex);

  # Build a basic regex to match type entries in the
  # generated ULPS file.
  foreach my $ftype (@all_floats) {
    $float_regex .= "|" . $ftype;
  }
  $float_regex = "^" . substr ($float_regex, 1) . ":";

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
    if (/$float_regex/) {
      ($float, $eps) = split /\s*:\s*/,$_,2;

      if ($eps eq "0") {
	# ignore
	next;
      } else {
	if (!defined ($results{$test}{$type}{'ulp'}{$float})
	    || $results{$test}{$type}{'ulp'}{$float} < $eps) {
	  $results{$test}{$type}{'ulp'}{$float} = $eps;
	  $results{$test}{'has_ulps'} = 1;
	}
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


# Clean up a floating point number
sub clean_up_number {
  my ($number) = @_;

  # Remove trailing zeros after the decimal point
  if ($number =~ /\./) {
    $number =~ s/0+$//;
    $number =~ s/\.$//;
  }
  return $number;
}

# Output a file which can be read in as ulps file.
sub print_ulps_file {
  my ($file) = @_;
  my ($test, $type, $float, $eps, $fct, $last_fct);

  $last_fct = '';
  open NEWULP, ">$file" or die ("Can't open $file: $!");
  print NEWULP "# Begin of automatic generation\n";
  print NEWULP "\n# Maximal error of functions:\n";

  foreach $fct (sort keys %results) {
    foreach $type ('real', 'imag', 'normal') {
      if (exists $results{$fct}{$type}) {
	if ($type eq 'normal') {
	  print NEWULP "Function: \"$fct\":\n";
	} elsif ($type eq 'real') {
	  print NEWULP "Function: Real part of \"$fct\":\n";
	} elsif ($type eq 'imag') {
	  print NEWULP "Function: Imaginary part of \"$fct\":\n";
	}
	foreach $float (@all_floats) {
	  if (exists $results{$fct}{$type}{'ulp'}{$float}) {
	    print NEWULP "$float: ",
	    &clean_up_number ($results{$fct}{$type}{'ulp'}{$float}),
	    "\n";
	  }
	}
	print NEWULP "\n";
      }
    }
  }
  print NEWULP "# end of automatic generation\n";
  close NEWULP;
}

sub get_ulps {
  my ($test, $type, $float) = @_;

  return (exists $results{$test}{$type}{'ulp'}{$float}
	  ? $results{$test}{$type}{'ulp'}{$float} : "0");
}

# Return the ulps value for a single test.
sub get_all_ulps_for_test {
  my ($test, $type) = @_;
  my ($ldouble, $double, $float, $ildouble, $idouble, $ifloat);
  my ($ulps_str);

  if (exists $results{$test}{'has_ulps'}) {
    foreach $float (@all_floats) {
      $ulps_str .= &get_ulps ($test, $type, $float) . ", ";
    }
    return "{" . substr ($ulps_str, 0, -2) . "}";
  } else {
    die "get_all_ulps_for_test called for \"$test\" with no ulps\n";
  }
}

# Print include file
sub output_ulps {
  my ($file, $ulps_filename) = @_;
  my ($i, $fct, $type, $ulp, $ulp_real, $ulp_imag);
  my (%func_ulps, %func_real_ulps, %func_imag_ulps);

  open ULP, ">$file" or die ("Can't open $file: $!");

  print ULP "/* This file is automatically generated\n";
  print ULP "   from $ulps_filename with gen-libm-test.pl.\n";
  print ULP "   Don't change it - change instead the master files.  */\n\n";

  print ULP "struct ulp_data\n";
  print ULP "{\n";
  print ULP "  const char *name;\n";
  print ULP "  FLOAT max_ulp[" . @all_floats . "];\n";
  print ULP "};\n\n";

  for ($i = 0; $i <= $#all_floats; $i++) {
    $type = $all_floats[$i];
    print ULP "#define ULP_";
    if ($type =~ /^i/) {
      print ULP "I_";
      $type = substr $type, 1;
    }
    print ULP "$all_floats_pfx{$type} $i\n";
  }

  foreach $fct (keys %results) {
    $type = $results{$fct}{'type'};
    if ($type eq 'normal') {
      $ulp = get_all_ulps_for_test ($fct, 'normal');
    } elsif ($type eq 'complex') {
      $ulp_real = get_all_ulps_for_test ($fct, 'real');
      $ulp_imag = get_all_ulps_for_test ($fct, 'imag');
    } else {
      die "unknown results ($fct) type $type\n";
    }
    if ($type eq 'normal') {
      $func_ulps{$fct} = $ulp;
    } else {
      $func_real_ulps{$fct} = $ulp_real;
      $func_imag_ulps{$fct} = $ulp_imag;
    }
  }
  print ULP "\n/* Maximal error of functions.  */\n";
  print ULP "static const struct ulp_data func_ulps[] =\n  {\n";
  foreach $fct (sort keys %func_ulps) {
    print ULP "    { \"$fct\", $func_ulps{$fct} },\n";
  }
  print ULP "  };\n";
  print ULP "static const struct ulp_data func_real_ulps[] =\n  {\n";
  foreach $fct (sort keys %func_real_ulps) {
    print ULP "    { \"$fct\", $func_real_ulps{$fct} },\n";
  }
  print ULP "  };\n";
  print ULP "static const struct ulp_data func_imag_ulps[] =\n  {\n";
  foreach $fct (sort keys %func_imag_ulps) {
    print ULP "    { \"$fct\", $func_imag_ulps{$fct} },\n";
  }
  print ULP "  };\n";
  close ULP;
}

# Parse auto-libm-test-out.
sub parse_auto_input {
  my ($file) = @_;
  open AUTO, $file or die ("Can't open $file: $!");
  while (<AUTO>) {
    chop;
    next if !/^= /;
    s/^= //;
    if (/^(\S+) (\S+) ([^:]*) : (.*)$/) {
      $auto_tests{$1}{$2}{$3} = $4;
    } else {
      die ("bad automatic test line: $_\n");
    }
  }
  close AUTO;
}
