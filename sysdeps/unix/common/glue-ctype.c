/* Copyright (C) 1992, 1993, 1994, 1997 Free Software Foundation, Inc.
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

/* Different systems have different names for the array.
   This order is important for some systems.  */

#if !defined(TABLE) && defined(HAVE__LOCP)
/* OSF/1 has the name _ctype defined as a macro, which points down into
   the _locp structure.  Jesus.  We'll hope this works.  We need to
   check for LOCP first, since there is no symbol actually named _ctype
   in their library.  */
#include <sys/types.h> /* for wchar_t used by localdef.h */
#include <sys/localedef.h>
extern loc_t *_locp;
#define TABLE		(_locp->lc_chrtbl)->lc_ctype
#undef _ctype
#define TABLE_NAME	_ctype
#endif
#ifdef	HAVE__CTYPE__
#define	TABLE	_ctype__
#endif
#if !defined(TABLE) && defined(HAVE__CTYPE)
#define TABLE   _ctype
#endif
#if !defined(TABLE) && defined(HAVE__CTYPE_)
#define	TABLE	_ctype_
#endif
#if !defined(TABLE) && defined(HAVE___CTYPE_)
#define	TABLE	__ctype_
#endif
#if !defined(TABLE) && defined(HAVE___CTYPE)
#define	TABLE	__ctype
#endif

#if defined (__STDC__) && __STDC__
#define	STRINGIFY(arg)	#arg
#else
#define	STRINGIFY(arg)	"arg"
#endif

#define EVALLED_STRINGIFY(x)	STRINGIFY (x)

int
main ()
{
#ifdef TABLE

  int i;

#ifndef HAVE__LOCP
  /* This won't work for the define to look into _locp.  */
  extern unsigned char TABLE[];
#endif

#ifdef TABLE_NAME
  printf ("const unsigned char %s[] =\n  {\n", EVALLED_STRINGIFY (TABLE_NAME));
#else
  printf ("const unsigned char %s[] =\n  {\n", EVALLED_STRINGIFY (TABLE));
#endif

  for (i = -1; i < 256; ++i)
    printf ("    %d,\n", (int) ((TABLE+1)[i]));

  puts ("  };");

#else

  puts ("/* I don't know what the ctype table is called on this system.");
  puts ("   If there is a table, and you want the ctype glue to work,");
  puts ("   edit configure.in and glue-ctype.c in sysdeps/unix/common/");
  puts ("   to check for the right name.  */");

#endif

  exit (0);
}
