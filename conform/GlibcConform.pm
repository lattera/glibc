#!/usr/bin/perl

# Shared code for glibc conformance tests.

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

package GlibcConform;
require Exporter;
@ISA = qw(Exporter);
@EXPORT = qw(%CFLAGS list_exported_functions);

# Compiler options for each standard.
$CFLAGS{"ISO"} = "-ansi";
$CFLAGS{"ISO99"} = "-std=c99";
$CFLAGS{"ISO11"} = "-std=c11";
$CFLAGS{"POSIX"} = "-D_POSIX_C_SOURCE=199506L -ansi";
$CFLAGS{"XPG3"} = "-ansi -D_XOPEN_SOURCE";
$CFLAGS{"XPG4"} = "-ansi -D_XOPEN_SOURCE -D_XOPEN_SOURCE_EXTENDED";
$CFLAGS{"UNIX98"} = "-ansi -D_XOPEN_SOURCE=500";
$CFLAGS{"XOPEN2K"} = "-std=c99 -D_XOPEN_SOURCE=600";
$CFLAGS{"XOPEN2K8"} = "-std=c99 -D_XOPEN_SOURCE=700";
$CFLAGS{"POSIX2008"} = "-std=c99 -D_POSIX_C_SOURCE=200809L";

# Return a list of functions exported by a header, empty if an include
# of the header does not compile.
sub list_exported_functions {
  my ($cc, $standard, $header, $tmpdir) = @_;
  my ($cc_all) = "$cc -D_ISOMAC $CFLAGS{$standard}";
  my ($tmpfile) = "$tmpdir/list-$$.c";
  my ($auxfile) = "$tmpdir/list-$$.c.aux";
  my ($ret);
  my (%res) = ();
  open (TMPFILE, ">$tmpfile") || die ("open $tmpfile: $!\n");
  print TMPFILE "#include <$header>\n";
  close (TMPFILE) || die ("close $tmpfile: $!\n");
  $ret = system "$cc_all -c $tmpfile -o /dev/null -aux-info $auxfile > /dev/null";
  unlink ($tmpfile) || die ("unlink $tmpfile: $!\n");
  if ($ret != 0) {
    return;
  }
  open (AUXFILE, "<$auxfile") || die ("open $auxfile: $!\n");
  while (<AUXFILE>) {
    s|/\*.*?\*/||g;
    if (/^\s*$/) {
      next;
    }
    # The word before a '(' that isn't '(*' is the function name
    # before the argument list (not fully general, but sufficient for
    # -aux-info output on standard headers).
    if (/(\w+)\s*\([^*]/) {
      $res{$1} = 1;
    } else {
      die ("couldn't parse -aux-info output: $_\n");
    }
  }
  close (AUXFILE) || die ("close $auxfile: $!\n");
  unlink ($auxfile) || die ("unlink $auxfile: $!\n");
  return sort keys %res;
}
