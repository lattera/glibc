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
Profile memory usage of PROGRAM.

      --help              print this help and exit
      --version           print version information and exit
      --progname          name of the program file to profile
      --png=FILE          generate PNG graphic and store it in FILE
      --data=FILE         generate binary data file and store it in FILE
      --unbuffered        don't buffer output
      --buffer=SIZE       collect SIZE entries before writing them out
      --no-timer          don't collect additional information though timer

   The following options only apply when generating graphical output:
      --time-based        make graph linear in time
      --total             also draw graph of total memory use
      --title=STRING      use STRING as title of the graph
      --x-size=SIZE       make graphic SIZE pixels wide
      --y-size=SIZE       make graphic SIZE pixels high
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
    if test $# -eq 1; then
      usage
    fi
    shift
    progname="$1"
    ;;
  --pr=* | --pro=* | --prog=* | --progn=* | --progna=* | --prognam=* | --progname=*)
    progname=${1##*=}
    ;;
  --pn | --png)
    if test $# -eq 1; then
      usage
    fi
    shift
    png="$1"
    ;;
  --pn=* | --png=*)
    png=${1##*=}
    ;;
  --d | --da | --dat | --data)
    if test $# -eq 1; then
      usage
    fi
    shift
    data="$1"
    ;;
  --d=* | --da=* | --dat=* | --data=*)
    data=${1##*=}
    ;;
  --u | --un | --unb | --unbu | --unbuf | --unbuff | --unbuffe | --unbuffer | --unbuffere | --unbuffered)
    buffer=1
    ;;
  --b | --bu | --buf | --buff | --buffe | --buffer)
    if test $# -eq 1; then
      usage
    fi
    shift
    buffer="$1"
    ;;
  --b=* | --bu=* | --buf=* | --buff=* | --buffe=* | --buffer=*)
    buffer=${1##*=}
    ;;
  --n | --no | --no- | --no-t | --no-ti | --no-tim | --no-time | --no-timer)
    notimer=yes
    ;;
  --tim | --time | --time- | --time-b | --time-ba | --time-bas | --time-base | --time-based)
    memprofstat_args="$memprofstat_args -t"
    ;;
  --to | --tot | --tota | --total)
    memprofstat_args="$memprofstat_args -T"
    ;;
  --tit | --titl | --title)
    if test $# -eq 1; then
      usage
    fi
    shift
    memprofstat_args="$memprofstat_args -s $1"
    ;;
  --tit=* | --titl=* | --title=*)
    memprofstat_args="$memprofstat_args -s ${1##*=}"
    ;;
  --x | --x- | --x-s | --x-si | --x-siz | --x-size)
    if test $# -eq 1; then
      usage
    fi
    shift
    memprofstat_args="$memprofstat_args -x $1"
    ;;
  --x=* | --x-=* | --x-s=* | --x-si=* | --x-siz=* | --x-size=*)
    memprofstat_args="$memprofstat_args -x ${1##*=}"
    ;;
  --y | --y- | --y-s | --y-si | --y-siz | --y-size)
    if test $# -eq 1; then
      usage
    fi
    shift
    memprofstat_args="$memprofstat_args -y $1"
    ;;
  --y=* | --y-=* | --y-s=* | --y-si=* | --y-siz=* | --y-size=*)
    memprofstat_args="$memprofstat_args -y ${1##*=}"
    ;;
  --p | --p=* | --t | --t=* | --ti | --ti=*)
    echo >&2 $"memprof: option \`${1##*=}' is ambiguous"
    usage
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

# Set buffer size.
if test -n "$buffer"; then
  add_env="$add_env MEMPROF_BUFFER_SIZE=$buffer"
fi

# Disable timers.
if test -n "$notimer"; then
  add_env="$add_env MEMPROF_NO_TIMER=yes"
fi

# Execute the program itself.
eval $add_env $*
result=$?

# Generate the PNG data file is wanted and there is something to generate
# it from.
if test -n "$png" -a -s "$datafile"; then
  # Append extension .png if it isn't already there.
  if test $png = ${png%*.png}; then
    png="$png.png"
  fi
  eval $memprofstat $memprofstat_args $datafile $png
fi

if test -z "$data" -a -n $datafile; then
  rm -f $datafile
fi

exit $result
# Local Variables:
#  mode:ksh
# End:
