/* Copyright (C) 1991, 1994, 1995 Free Software Foundation, Inc.

This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#if defined (_AIX) && !defined (__GNUC__)
 #pragma alloca
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#if _LIBC || HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if _LIBC || HAVE_STRING_H
#include <string.h>
#endif

#if !__GNU_LIBRARY__ && !HAVE_STRCHR
#define strchr index
#endif

#ifndef _LIBC
#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#else
#ifdef __GNUC__
#define alloca __builtin_alloca
#else
extern char *alloca ();
#endif /* __GNUC__ */
#endif /* HAVE_ALLOCA_H */
#endif /* _LIBC */


/* Put STRING, which is of the form "NAME=VALUE", in the environment.  */
int
putenv (string)
     const char *string;
{
  const char *const name_end = strchr (string, '=');

  if (name_end)
    {
      char *name = alloca (name_end - string + 1);
      memcpy (name, string, name_end - string);
      name[name_end - string] = '\0';
      return setenv (name, name_end + 1, 1);
    }

  unsetenv (string);
  return 0;
}
