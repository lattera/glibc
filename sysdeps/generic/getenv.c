/* Copyright (C) 1991, 1992, 1994, 1996 Free Software Foundation, Inc.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef	HAVE_GNU_LD
#define	__environ	environ
#endif

/* Return the value of the environment variable NAME.  */
char *
getenv (name)
     const char *name;
{
  const size_t len = strlen (name);
  char **ep;

  if (__environ == NULL)
    return NULL;

  for (ep = __environ; *ep != NULL; ++ep)
    if (!strncmp (*ep, name, len) && (*ep)[len] == '=')
      return &(*ep)[len + 1];

  return NULL;
}
