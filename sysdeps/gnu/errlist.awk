# Copyright (C) 1991, 92, 93, 94, 95, 96 Free Software Foundation, Inc.
# This file is part of the GNU C Library.

# The GNU C Library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public License
# as published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.

# The GNU C Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.

# You should have received a copy of the GNU Library General Public
# License along with the GNU C Library; see the file COPYING.LIB.  If
# not, write to the Free Software Foundation, Inc., 675 Mass Ave,
# Cambridge, MA 02139, USA.

# errno.texinfo contains lines like:
# @comment errno.h
# @comment POSIX.1: Function not implemented
# @deftypevr Macro int ENOSYS
# @comment errno 78

BEGIN {
    print "/* This file is generated from errno.texi by errlist.awk.  */"
    print "";
    print "#include <errno.h>";
    print "";
    print "const char *_sys_errlist[] =";
    print "  {";
    print "    [0] = N_(\"Success\"),"
  }
$1 == "@comment" && $2 == "errno.h" { errnoh=1; next }
errnoh == 1 && $1 == "@comment" \
  {
    ++errnoh;
    etext = $3;
    for (i = 4; i <= NF; ++i)
      etext = etext " " $i;
    next;
  }
errnoh == 2 && $1 == "@deftypevr" && $2 == "Macro" && $3 == "int" \
  {
    e = $4; errnoh++; next;
  }
errnoh == 3 && $1 == "@comment" && $2 == "errno" \
  {
    errno = $3 + 0;
    if (e == "EWOULDBLOCK")
      print "#if defined (EWOULDBLOCK) && EWOULDBLOCK != EAGAIN";
    else
      printf "#ifdef %s\n", e;
    printf "    [%s] = N_(\"%s\"),\n", e, etext;
    print "#endif";
    next;
  }
{ errnoh=0 }
END {
  print "  };";
  print "";
  print "const int _sys_nerr = sizeof _sys_errlist / sizeof _sys_errlist[0];";
  print "weak_alias (_sys_errlist, sys_errlist)";
  print "weak_alias (_sys_nerr, sys_nerr)";
  }
