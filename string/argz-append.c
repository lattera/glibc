/* Routines for dealing with '\0' separated arg vectors.

   Copyright (C) 1995, 1996 Free Software Foundation, Inc.

   Written by Miles Bader <miles@gnu.ai.mit.edu>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA. */

#include <argz.h>
#include <string.h>
#include <stdlib.h>

/* Add BUF, of length BUF_LEN to the argz vector in ARGZ & ARGZ_LEN.  */
error_t
__argz_append (char **argz, size_t *argz_len, const char *buf, size_t buf_len)
{
  size_t new_argz_len = *argz_len + buf_len;
  char *new_argz = realloc (*argz, new_argz_len);
  if (new_argz)
    {
      memcpy (new_argz + *argz_len, buf, buf_len);
      *argz = new_argz;
      *argz_len = new_argz_len;
      return 0;
    }
  else
    return ENOMEM;
}
weak_alias (__argz_append, argz_append)

/* Add STR to the argz vector in ARGZ & ARGZ_LEN.  This should be moved into
   argz.c in libshouldbelibc.  */
error_t
__argz_add (char **argz, size_t *argz_len, const char *str)
{
  return __argz_append (argz, argz_len, str, strlen (str) + 1);
}
weak_alias (__argz_add, argz_add)
