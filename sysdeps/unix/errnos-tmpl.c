/* Copyright (C) 1991, 1993, 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <errno.h>

static char iferrno[] = "#ifdef _ERRNO_H";
static char endiferrno[] = "#endif /* <errno.h> included.  */";
static char ifEmath[] = "#if !defined(__Emath_defined) && \
 (defined(_ERRNO_H) || defined(__need_Emath))";
static char endifEmath[] = "#endif /* Emath not defined and <errno.h> \
included or need Emath.  */";

static int biggest_value = 0;
static int done_ENOSYS = 0;
static int done_ERANGE = 0, done_EDOM = 0;

static void
DO(name, value)
     char *name;
     int value;
{
  int is_ERANGE = !done_ERANGE && !strcmp(name, "ERANGE");
  int is_EDOM = !done_EDOM && !strcmp(name, "EDOM");
  int is_Emath = is_ERANGE || is_EDOM;

  if (is_Emath)
    {
      puts(endiferrno);
      puts(ifEmath);
    }

  if (!strcmp (name, "EWOULDBLOCK"))
    {
      puts ("#define EWOULDBLOCK EAGAIN /* Translated in glibc. */");
      name = "EWOULDBLOCK_sys /* Value actually returned by kernel. */";
    }

  printf ("#define %s %d\n", name, value);

  if (is_Emath)
    {
      puts(endifEmath);
      puts(iferrno);
    }

  if (value > biggest_value)
    biggest_value = value;

  if (is_ERANGE)
    done_ERANGE = 1;
  else if (is_EDOM)
    done_EDOM = 1;
  else if (!done_ENOSYS && !strcmp(name, "ENOSYS"))
    done_ENOSYS = 1;
}

int
main()
{
  puts(iferrno);

  ERRNOS;

  if (!done_EDOM || !done_ERANGE)
    {
      puts(endiferrno);
      puts(ifEmath);
      if (!done_EDOM)
	printf("#define EDOM %d\n", ++biggest_value);
      if (!done_ERANGE)
	printf("#define ERANGE %d\n", ++biggest_value);
      puts(endifEmath);
    }

  if (!done_ENOSYS)
    printf("#define ENOSYS %d\n", ++biggest_value);

  puts(endiferrno);

  puts("#undef __need_Emath");
  puts("#ifndef __Emath_defined\n#define __Emath_defined 1\n#endif");

  exit(0);
}
