/* Copyright (C) 1991,1992,1995,1997,1999,2001 Free Software Foundation, Inc.
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

#include <stdio.h>
#include <time.h>

/* Make a definition for sys_errlist.  */

extern int sys_nerr;
#ifndef HAVE_STRERROR
extern const char *const sys_errlist[];
#endif

int
main (void)
{
  register int i;
  struct tm timenow;
  time_t now;
  int year;

  now = time (NULL);
  timenow = *localtime (&now);

  year = timenow.tm_year + 1900;

  printf ("/* Copyright (C) %d Free Software Foundation, Inc.\n\
   This file is part of the GNU C Library.\n\
\n\
   The GNU C Library is free software; you can redistribute it and/or\n\
   modify it under the terms of the GNU Lesser General Public\n\
   License as published by the Free Software Foundation; either\n\
   version 2.1 of the License, or (at your option) any later version.\n\
\n\
   The GNU C Library is distributed in the hope that it will be useful,\n\
   but WITHOUT ANY WARRANTY; without even the implied warranty of\n\
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU\n\
   Lesser General Public License for more details.\n\
\n\
   You should have received a copy of the GNU Lesser General Public\n\
   License along with the GNU C Library; if not, write to the Free\n\
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA\n\
   02111-1307 USA.  */\n", year);

  puts ("#include <stddef.h>\n");
  puts ("#include <libintl.h>\n");
  puts ("\n/* This is a list of all known `errno' codes.  */\n");

  printf ("\nconst int _sys_nerr = %d;\n\n", sys_nerr);
  puts ("const char *const _sys_errlist[] =\n  {");

  for (i = 0; i < sys_nerr; ++i)
    printf ("    N_(\"%s\"),\n",
#ifdef HAVE_STRERROR
	  strerror (i)
#else /* ! HAVE_STRERROR */
	  sys_errlist[i]
#endif /* HAVE_STRERROR */
	  );

  puts ("    NULL\n  };\n");

  puts ("weak_alias (_sys_errlist, sys_errlist)");
  puts ("weak_alias (_sys_nerr, sys_nerr)");

  exit (0);
}
