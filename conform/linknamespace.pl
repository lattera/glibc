#!/usr/bin/perl

# Check that use of symbols declared in a given header does not result
# in any symbols being brought in that are not reserved with external
# linkage for the given standard.

# Copyright (C) 2014-2018 Free Software Foundation, Inc.
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

GetOptions ('header=s' => \$header, 'standard=s' => \$standard,
	    'flags=s' => \$flags, 'cc=s' => \$CC, 'tmpdir=s' => \$tmpdir,
	    'stdsyms=s' => \$stdsyms_file, 'libsyms=s' => \$libsyms_file,
	    'readelf=s' => \$READELF);

# Load the list of symbols that are OK.
%stdsyms = ();
open (STDSYMS, "<$stdsyms_file") || die ("open $stdsyms_file: $!\n");
while (<STDSYMS>) {
  chomp;
  $stdsyms{$_} = 1;
}
close (STDSYMS) || die ("close $stdsyms_file: $!\n");

# The following whitelisted symbols are also allowed for now.
#
# * Bug 17576: stdin, stdout, stderr only reserved with external
# linkage when stdio.h included (and possibly not then), not
# generally.
#
# * Bug 18442: re_syntax_options wrongly brought in by regcomp and
# used by re_comp.
#
@whitelist = qw(stdin stdout stderr re_syntax_options);
foreach my $sym (@whitelist) {
  $stdsyms{$sym} = 1;
}

# Return information about GLOBAL and WEAK symbols listed in readelf
# -s output.
sub list_syms {
  my ($syms_file) = @_;
  open (SYMS, "<$syms_file") || die ("open $syms_file: $!\n");
  my ($file) = $syms_file;
  my (@ret) = ();
  while (<SYMS>) {
    chomp;
    if (/^File: (.*)/) {
      $file = $1;
      $file =~ s|^.*/||;
      next;
    }
    s/^\s*//;
    # Architecture-specific st_other bits appear inside [] and disrupt
    # the format of readelf output.
    s/\[.*?\]//;
    my (@fields) = split (/\s+/, $_);
    if (@fields < 8) {
      next;
    }
    my ($bind) = $fields[4];
    my ($ndx) = $fields[6];
    my ($sym) = $fields[7];
    if ($bind ne "GLOBAL" && $bind ne "WEAK") {
      next;
    }
    if ($sym !~ /^\w+$/) {
      next;
    }
    push (@ret, [$file, $sym, $bind, $ndx ne "UND"]);
  }
  close (SYMS) || die ("close $syms_file: $!\n");
  return @ret;
}

# Load information about GLOBAL and WEAK symbols defined or used in
# the standard libraries.
# Symbols from a given object, except for weak defined symbols.
%seen_syms = ();
# Strong undefined symbols from a given object.
%strong_undef_syms = ();
# Objects defining a given symbol (strongly or weakly).
%sym_objs = ();
@sym_data = list_syms ($libsyms_file);
foreach my $sym (@sym_data) {
  my ($file, $name, $bind, $defined) = @$sym;
  if ($defined) {
    if (!defined ($sym_objs{$name})) {
      $sym_objs{$name} = [];
    }
    push (@{$sym_objs{$name}}, $file);
  }
  if ($bind eq "GLOBAL" || !$defined) {
    if (!defined ($seen_syms{$file})) {
      $seen_syms{$file} = [];
    }
    push (@{$seen_syms{$file}}, $name);
  }
  if ($bind eq "GLOBAL" && !$defined) {
    if (!defined ($strong_undef_syms{$file})) {
      $strong_undef_syms{$file} = [];
    }
    push (@{$strong_undef_syms{$file}}, $name);
  }
}

# Determine what ELF-level symbols are brought in by use of C-level
# symbols declared in the given header.
#
# The rules followed are heuristic and so may produce false positives
# and false negatives.
#
# * All undefined symbols are considered of signficance, but it is
# possible that (a) any standard library definition is weak, so can be
# overridden by the user's definition, and (b) the symbol is only used
# conditionally and not if the program is limited to standard
# functionality.
#
# * If a symbol reference is only brought in by the user using a data
# symbol rather than a function from the standard library, this will
# not be detected.
#
# * If a symbol reference is only brought in by crt*.o or libgcc, this
# will not be detected.
#
# * If a symbol reference is only brought in through __builtin_foo in
# a standard macro being compiled to call foo, this will not be
# detected.
#
# * Header inclusions should be compiled several times with different
# options such as -O2, -D_FORTIFY_SOURCE and -D_FILE_OFFSET_BITS=64 to
# find out what symbols are undefined from such a compilation; this is
# not yet implemented.
#
# * This script finds symbols referenced through use of macros on the
# basis that if a macro calls an internal function, that function must
# also be declared in the header.  However, the header might also
# declare implementation-namespace functions that are not called by
# any standard macro in the header, resulting in false positives for
# any symbols brought in only through use of those
# implementation-namespace functions.
#
# * Namespace issues can apply for dynamic linking as well as static
# linking, when a call is from one shared library to another or uses a
# PLT entry for a call within a shared library; such issues are only
# detected by this script if the same namespace issue applies for
# static linking.

@c_syms = list_exported_functions ("$CC $flags", $standard, $header, $tmpdir);
$cincfile = "$tmpdir/undef-$$.c";
$cincfile_o = "$tmpdir/undef-$$.o";
$cincfile_sym = "$tmpdir/undef-$$.sym";
open (CINCFILE, ">$cincfile") || die ("open $cincfile: $!\n");
print CINCFILE "#include <$header>\n";
foreach my $sym (sort @c_syms) {
  print CINCFILE "void *__glibc_test_$sym = (void *) &$sym;\n";
}
close CINCFILE || die ("close $cincfile: $!\n");
system ("$CC $flags -D_ISOMAC $CFLAGS{$standard} -c $cincfile -o $cincfile_o")
  && die ("compiling failed\n");
system ("LC_ALL=C $READELF -W -s $cincfile_o > $cincfile_sym")
  && die ("readelf failed\n");
@elf_syms = list_syms ($cincfile_sym);
unlink ($cincfile) || die ("unlink $cincfile: $!\n");
unlink ($cincfile_o) || die ("unlink $cincfile_o: $!\n");
unlink ($cincfile_sym) || die ("unlink $cincfile_sym: $!\n");

%seen_where = ();
%files_seen = ();
%all_undef = ();
%current_undef = ();
foreach my $sym (@elf_syms) {
  my ($file, $name, $bind, $defined) = @$sym;
  if ($bind eq "GLOBAL" && !$defined) {
    $seen_where{$name} = "[initial] $name";
    $all_undef{$name} = "[initial] $name";
    $current_undef{$name} = "[initial] $name";
  }
}

while (%current_undef) {
  %new_undef = ();
  foreach my $sym (sort keys %current_undef) {
    foreach my $file (@{$sym_objs{$sym}}) {
      if (defined ($files_seen{$file})) {
	next;
      }
      $files_seen{$file} = 1;
      foreach my $ssym (@{$seen_syms{$file}}) {
	if (!defined ($seen_where{$ssym})) {
	  $seen_where{$ssym} = "$current_undef{$sym} -> [$file] $ssym";
	}
      }
      foreach my $usym (@{$strong_undef_syms{$file}}) {
	if (!defined ($all_undef{$usym})) {
	  $all_undef{$usym} = "$current_undef{$sym} -> [$file] $usym";
	  $new_undef{$usym} = "$current_undef{$sym} -> [$file] $usym";
	}
      }
    }
  }
  %current_undef = %new_undef;
}

$ret = 0;
foreach my $sym (sort keys %seen_where) {
  if ($sym =~ /^_/) {
    next;
  }
  if (defined ($stdsyms{$sym})) {
    next;
  }
  print "$seen_where{$sym}\n";
  $ret = 1;
}

exit $ret;
