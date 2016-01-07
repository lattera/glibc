#!/usr/bin/perl

# Print a list of symbols exported by some headers that would
# otherwise be in the user's namespace.

# Copyright (C) 2014-2016 Free Software Foundation, Inc.
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

use GlibcConform;
use Getopt::Long;

GetOptions ('headers=s' => \$headers, 'standard=s' => \$standard,
	    'flags=s' => \$flags, 'cc=s' => \$CC, 'tmpdir=s' => \$tmpdir);
@headers = split (/\s+/, $headers);

# Extra symbols possibly not found through -aux-info but still
# reserved by the standard: either data symbols, or symbols where the
# standard leaves unspecified whether the identifier is a macro or
# defined with external linkage.
$extra_syms{"ISO"} = ["errno", "setjmp", "va_end"];
$extra_syms{"ISO99"} = ["errno", "math_errhandling", "setjmp", "va_end"];
# stdatomic.h not yet covered by conformance tests; as per DR#419, all
# the generic functions there or may not be defined with external
# linkage (but are reserved in any case).
$extra_syms{"ISO11"} = ["errno", "math_errhandling", "setjmp", "va_end"];
# The following lists may not be exhaustive.
$extra_syms{"POSIX"} = ["errno", "setjmp", "va_end", "environ", "sigsetjmp",
			"optarg", "optind", "opterr", "optopt", "tzname"];
$extra_syms{"XPG3"} = ["errno", "setjmp", "va_end", "environ", "signgam",
		       "loc1", "loc2", "locs", "sigsetjmp", "optarg",
		       "optind", "opterr", "optopt", "daylight", "timezone",
		       "tzname"];
$extra_syms{"XPG4"} = ["errno", "setjmp", "va_end", "environ", "signgam",
		       "loc1", "loc2", "locs", "sigsetjmp", "optarg",
		       "optind", "opterr", "optopt", "daylight", "timezone",
		       "tzname", "getdate_err", "h_errno"];
$extra_syms{"UNIX98"} = ["errno", "setjmp", "va_end", "environ", "signgam",
			 "loc1", "loc2", "locs", "sigsetjmp", "optarg",
			 "optind", "opterr", "optopt", "daylight", "timezone",
			 "tzname", "getdate_err", "h_errno"];
$extra_syms{"XOPEN2K"} = ["errno", "setjmp", "va_end", "environ", "signgam",
			  "sigsetjmp", "optarg", "optind", "opterr", "optopt",
			  "daylight", "timezone", "tzname", "getdate_err",
			  "h_errno", "in6addr_any", "in6addr_loopback"];
$extra_syms{"XOPEN2K8"} = ["errno", "setjmp", "va_end", "environ", "signgam",
			   "sigsetjmp", "optarg", "optind", "opterr", "optopt",
			   "daylight", "timezone", "tzname", "getdate_err",
			   "in6addr_any", "in6addr_loopback"];
$extra_syms{"POSIX2008"} = ["errno", "setjmp", "va_end", "environ",
			    "sigsetjmp", "optarg", "optind", "opterr", "optopt",
			    "tzname", "in6addr_any", "in6addr_loopback"];

%user_syms = ();

foreach my $header (@headers) {
  @syms = list_exported_functions ("$CC $flags", $standard, $header, $tmpdir);
  foreach my $sym (@syms) {
    if ($sym !~ /^_/) {
      $user_syms{$sym} = 1;
    }
  }
}
foreach my $sym (@{$extra_syms{$standard}}) {
  $user_syms{$sym} = 1;
}

foreach my $sym (sort keys %user_syms) {
  print "$sym\n";
}
