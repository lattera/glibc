#! /bin/sh

# Copyright (C) 1998 Free Software Foundation, Inc.
# This file is part of the GNU C Library.
# Contributed by Ulrich Drepper <drepper@cygnus.com>, 1998.

# The GNU C Library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public License as
# published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.

# The GNU C Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.

# You should have received a copy of the GNU Library General Public
# License along with the GNU C Library; see the file COPYING.LIB.  If not,
# write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA 02111-1307, USA.

prog="$1"
shift
args="$*"

if test $# -eq 0; then
  case "$args" in
    --h | --he | --hel | --help)
      echo 'Usage: catchsegv PROGRAM ARGS...'
      echo '  --help      print this help, then exit'
      echo '  --version   print version number, then exit'
      echo "Report bugs using the \`glibcbug' script to <bugs@gnu.org>."
      exit 0
      ;;
    --v | --ve | --ver | --vers | --versi | --versio | --version)
      echo 'catchsegv (GNU libc) @VERSION@'
      echo 'Copyright (C) 1998 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
Written by Ulrich Drepper.'
      exit 0
      ;;
    *)
      ;;
  esac
fi

LD_PRELOAD="${LD_PRELOAD:+${LD_PRELOAD}:}@SLIB@/libSegFault.so"
export LD_PRELOAD
SEGFAULT_USE_ALTSTACK=1
export SEGFAULT_USE_ALTSTACK
SEGFAULT_OUTPUT_NAME="$prog.segv.$$"
export SEGFAULT_OUTPUT_NAME

$prog $args
exval=$?

unset LD_PRELOAD
# Check for an segmentation fault.
if test $exval -eq 139; then
  # We caught a segmentation error.  The output is in the file with the
  # name we have in SEGFAULT_OUTPUT_NAME.  In the output the names of
  # functions in shared objects are available, but names in the static
  # part of the program are not.  We use addr2line to get this information.
  (read line; echo "$line"
   read line; echo "$line"
   while read line; do
     case "$line" in
       [*) addr="`echo $line | sed 's/^[\(.*\)]$/\1/'`"
           complete="`addr2line -f -e $prog $addr 2>/dev/null|`"
           if $? -eq 0; then
             echo "`echo $complete|sed 'N;s/\(.*\)\n\(.*\)/\2(\1)/;'`$line"
           else
             echo "$line"
           fi
           ;;
        *) echo "$line"
           ;;
     esac
   done) < $SEGFAULT_OUTPUT_NAME
   rm $SEGFAULT_OUTPUT_NAME
fi

exit $exval
