#! @BASH@
# Copyright (C) 1999 Free Software Foundation, Inc.
# This file is part of the GNU C Library.
# Contributed by Ulrich Drepper <drepper@gnu.org>, 1999.

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

memprofso=@LIBDIR@/libmemprof.so
memprofstat=@BINDIR@/memprofstat

# Print usage message.
do_usage() {
  echo $"Try \`memprof --help' for more information."
  exit 1
}

do_help() {
  echo $"Usage: memprof [OPTION]... PROGRAM [PROGRAMOPTION...]
      --help              print this help and exit
      --version           print version information and exit
      --progname          name of the program file to profile
      --png=FILE          generate PNG graphic and store it in FILE
      --data=FILE         generate binary data file and store it in FILE
Report bugs using the \`glibcbug' script to <bugs@gnu.org>."
  exit 0
}

do_version() {
  echo 'memprof (GNU libc) @VERSION@'
  echo $"Copyright (C) 1999 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
Written by Ulrich Drepper."
  exit 0
}

# Process arguments.  But stop as soon as the program name is found.
while test $# -gt 0; do
  case "$1" in
  --v | --ve | --ver | --vers | --versi | --versio | --version)
    do_version
    ;;
  --h | --he | --hel | --help)
    do_help
    ;;
  --pr | --pro | --prog | --progn | --progna | --prognam | --progname)
    progname="$1"
    ;;
  --pn | --png)
    png="$1"
    ;;
  --d | --da | --dat | --data)
    data="$1"
    ;;
  --)
    # Stop processing arguments.
    shift
    break
    ;;
  *)
    # Unknown option.  This means the rest is the program name and parameters.
    break
    ;;
  esac
  shift
done

# See whether any arguments are left.
if test $# -le 0; then
  usage
fi

# This will be in the environment.
add_env="LD_PRELOAD=$memprofso"

# Generate data file name.
if test -n "$data"; then
  datafile="$data"
elif test -n "$png"; then
  datafile=$(mktemp ${TMPDIR:-/tmp}/memprof.XXXXXX 2> /dev/null)
  if test $? -ne 0; then
    # Lame, but if there is no `mktemp' program the user cannot expect more.
    datafile=${TMPDIR:-/tmp}/memprof.$$
  fi
fi
if test -n "$datafile"; then
  add_env="$add_env MEMPROF_OUTPUT=$datafile"
fi

# Execute the program itself.
eval $add_env $*
result=$?

# Generate the PNG data file is wanted and there is something to generate
# it from.
if test -n "$png" -a -s "$datafile"; then
  eval $memprofstat $datafile $png
fi

if test -z $data -a -n $datafile; then
  rm -f $datafile
fi

exit $result
# Local Variables:
#  mode:ksh
# End:
