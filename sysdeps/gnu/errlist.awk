# Copyright (C) 1991,92,93,94,95,96,97,98,99 Free Software Foundation, Inc.
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
# License along with the GNU C Library; if not, write to the Free
# Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
# 02111-1307 USA.

# errno.texi contains lines like:
# @comment errno.h
# @comment POSIX.1: Function not implemented
# @deftypevr Macro int ENOSYS
# @comment errno 78
# Descriptive paragraph...
# @end deftypevr

BEGIN {

    # Here we list the E* names that might be duplicate names for the
    # same integer value on some systems.  This causes the code below
    # to generate ``#if defined (ALIAS) && ALIAS != ORIGINAL'' in the code,
    # so the output does not presume that these are in fact aliases.
    # We list here all the known potential cases on any system,
    # so that the C source we produce will do the right thing based
    # on the actual #define'd values it's compiled with.
    alias["EWOULDBLOCK"]= "EAGAIN";
    alias["EDEADLOCK"]	= "EDEADLK";
    alias["ENOTSUP"]	= "EOPNOTSUPP";

    print "/* This file is generated from errno.texi by errlist.awk.  */"
    print "";
    print "#include <errno.h>";
    print "#include <libintl.h>";
    print "";
    print "#ifndef SYS_ERRLIST";
    print "# define SYS_ERRLIST _sys_errlist";
    print "# define SYS_ERRLIST_ALIAS sys_errlist";
    print "#endif";
    print "#ifndef SYS_NERR";
    print "# define SYS_NERR _sys_nerr";
    print "# define SYS_NERR_ALIAS sys_nerr";
    print "#endif";
    print "#ifndef ERR_REMAP";
    print "# define ERR_REMAP(n) n";
    print "#endif";
    print "";
    print "const char *const SYS_ERRLIST[] =";
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
    if (alias[e])
      printf "#if defined (%s) && %s != %s\n", e, e, alias[e];
    else
      printf "#ifdef %s\n", e;
    errnoh = 4;
    desc="";
    next;
  }
errnoh == 4 && $1 == "@end" && $2 == "deftypevr" \
  {
    printf "/*%s */\n", desc;
    printf "    [ERR_REMAP (%s)] = N_(\"%s\"),\n", e, etext;
    print "#endif";
    errnoh = 0;
    next;
  }
errnoh == 4 \
  {
    # This magic tag in C comments gets them copied into libc.pot.
    desc = desc "\nTRANS " $0; next
  }
{ errnoh=0 }
END {
  print "  };";
  print "";
  print "const int SYS_NERR = sizeof SYS_ERRLIST / sizeof SYS_ERRLIST [0];";
  print "#ifdef SYS_ERRLIST_ALIAS";
  print "weak_alias (_sys_errlist, SYS_ERRLIST_ALIAS)";
  print "#endif";
  print "#ifdef SYS_NERR_ALIAS";
  print "weak_alias (_sys_nerr, SYS_NERR_ALIAS)";
  print "#endif";
  }
