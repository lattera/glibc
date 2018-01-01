#! /bin/sh
# Test that glibc's signal numbers match the kernel's.
# Copyright (C) 2017-2018 Free Software Foundation, Inc.
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

set -e
if [ -n "$BASH_VERSION" ]; then set -o pipefail; fi
LC_ALL=C; export LC_ALL

# We cannot use Linux's asm/signal.h to define signal numbers, because
# it isn't sufficiently namespace-clean.  Instead, this test checks
# that our signal numbers match the kernel's.  This script expects
# "$@" to be $(CC) $(CPPFLAGS) as set by glibc's Makefiles, and $AWK
# to be set in the environment.

# Before doing anything else, fail if the compiler doesn't work.
"$@" -E -xc -dM - < /dev/null > /dev/null

tmpG=`mktemp -t signums_glibc.XXXXXXXXX`
tmpK=`mktemp -t signums_kernel.XXXXXXXXX`
trap "rm -f '$tmpG' '$tmpK'" 0

# Filter out constants that aren't signal numbers.
# If SIGPOLL is defined as SIGIO, swap it around so SIGIO is defined as
# SIGPOLL. Similarly for SIGABRT and SIGIOT.
# Discard obsolete signal numbers and unrelated constants:
#    SIGCLD, SIGIOT, SIGSWI, SIGUNUSED.
#    SIGSTKSZ, SIGRTMIN, SIGRTMAX.
# Then sort the list.
filter_defines ()
{
    $AWK '
/^#define SIG[A-Z]+ ([0-9]+|SIG[A-Z0-9]+)$/ { signals[$2] = $3 }
END {
  if ("SIGPOLL" in signals && "SIGIO" in signals &&
      signals["SIGPOLL"] == "SIGIO") {
    signals["SIGPOLL"] = signals["SIGIO"]
    signals["SIGIO"] = "SIGPOLL"
  }
  if ("SIGABRT" in signals && "SIGIOT" in signals &&
      signals["SIGABRT"] == "SIGIOT") {
    signals["SIGABRT"] = signals["SIGIOT"]
    signals["SIGIOT"] = "SIGABRT"
  }
  for (sig in signals) {
    if (sig !~ /^SIG(CLD|IOT|RT(MIN|MAX)|STKSZ|SWI|UNUSED)$/) {
      printf("#define %s %s\n", sig, signals[sig])
    }
  }
}' | sort
}

# $CC may contain command-line switches, so it should be word-split.
printf '%s' '#define _GNU_SOURCE 1
#include <signal.h>
' |
    "$@" -E -xc -dM - |
    filter_defines > "$tmpG"

printf '%s' '#define _GNU_SOURCE 1
#define __ASSEMBLER__ 1
#include <asm/signal.h>
' |
    "$@" -E -xc -dM - |
    filter_defines > "$tmpK"

if cmp -s "$tmpG" "$tmpK"; then
    exit 0
else
    diff -u "$tmpG" "$tmpK"
    exit 1
fi
