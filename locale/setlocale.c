/* Copyright (C) 1991, 1992 Free Software Foundation, Inc.
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

#include <ansidecl.h>
#include <localeinfo.h>
#include <errno.h>
#include <locale.h>
#include <string.h>


/* Switch to the locale called NAME in CATEGORY.
   Return a string describing the locale.  This string can
   be used as the NAME argument in a later call.
   If NAME is NULL, don't switch locales, but return the current one.
   If NAME is "", switch to a locale based on the environment variables,
   as per POSIX.  Return NULL on error.  */
char *
DEFUN(setlocale, (category, name), int category AND CONST char *name)
{
  /* Braindead implementation until I finish the fancy one.  */

  if (name == NULL || name[0] == '\0')
    return (char *) "C";

  if (!strcmp(name, "C") || !strcmp(name, "POSIX"))
    return (char *) name;

  errno = EINVAL;
  return NULL;
}
