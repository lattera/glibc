#! /bin/sh -f
# Run available iconv(1) tests.
# Copyright (C) 1998 Free Software Foundation, Inc.
# This file is part of the GNU C Library.
# Contributed by Ulrich Drepper <drepper@cygnus.com>, 1998.
#
# The GNU C Library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public License as
# published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.
#
# The GNU C Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public
# License along with the GNU C Library; see the file COPYING.LIB.  If not,
# write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA 02111-1307, USA.

codir=$1

# We use always the same temporary file.
temp1=$codir/iconvdata/iconv-test.xxx
temp2=$codir/iconvdata/iconv-test.yyy

trap "rm -f $temp1 $temp2" 1 2 3 15

# We must tell the iconv(1) program where the modules we want to use can
# be found.
GCONV_PATH=$codir/iconvdata
export GCONV_PATH

# How the start the iconv(1) program.
ICONV="$codir/elf/ld.so --library-path $codir $codir/iconv/iconv_prog"

# We read the file named TESTS.  All non-empty lines not starting with
# `#' are interpreted as commands.
while read from to targets; do
  # Ignore empty and comment lines.
  if test -z "$targets" || test "$from" = '#'; then continue; fi

  for t in $targets; do
    $ICONV -f $from -t $t testdata/$from > $temp1 ||
      { echo "*** conversion from $from to $t failed"; exit 1; }
    if test -s testdata/$from..$t; then
      cmp $temp1 testdata/$from..$t >& /dev/null ||
	{ echo "*** $from -> $t conversion failed"; exit 1; }
    fi
    $ICONV -f $t -t $to -o $temp2 $temp1 ||
      { echo "*** conversion from $t to $to failed"; exit 1; }
    test -s $temp1 && cmp testdata/$from $temp2 >& /dev/null ||
      { echo "*** $from -> t -> $to conversion failed"; exit 1; }
    rm -f $temp1 $temp2

    # Now test some bigger text, entirely in ASCII.
    $ICONV -f $from -t $t testdata/suntzus |
    $ICONV -f $t -t $to > $temp1 ||
      { echo "*** conversion $from->$t->$to of suntzus failed"; exit 1; }
    cmp testdata/suntzus.txt $temp1 ||
      { echo "*** conversion $from->$t->$to of suntzus incorrect"; exit 1; }
    rm -f $temp1

    # All tests ok.
    echo "$from -> $t -> $to ok"
  done
done < TESTS

exit $?
# Local Variables:
#  mode:shell-script
# End:
