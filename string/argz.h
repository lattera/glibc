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

#ifndef __ARGZ_H__
#define __ARGZ_H__	1

#include <features.h>

#include <errno.h>		/* Define error_t.  */
#include <string.h>		/* Need size_t, and strchr is called below.  */


__BEGIN_DECLS

/* Make a '\0' separated arg vector from a unix argv vector, returning it in
   ARGZ, and the total length in LEN.  If a memory allocation error occurs,
   ENOMEM is returned, otherwise 0.  The result can be destroyed using free. */
error_t __argz_create __P ((char *const argv[], char **argz, size_t *len));
error_t argz_create __P ((char *const *argv[], char **argz, size_t *len));

/* Make a '\0' separated arg vector from a SEP separated list in
   STRING, returning it in ARGZ, and the total length in LEN.  If a
   memory allocation error occurs, ENOMEM is returned, otherwise 0.
   The result can be destroyed using free.  */
error_t __argz_create_sep __P ((__const char *string, int sep,
				char **argz, size_t *len));
error_t argz_create_sep __P ((__const char *string, int sep,
			      char **argz, size_t *len));

/* Returns the number of strings in ARGZ.  */
size_t __argz_count __P ((__const char *argz, size_t len));
size_t argz_count __P ((__const char *argz, size_t len));

/* Puts pointers to each string in ARGZ into ARGV, which must be large enough
   to hold them all.  */
void __argz_extract __P ((char *argz, size_t len, char **argv));
void argz_extract __P ((char *argz, size_t len, char **argv));

/* Make '\0' separated arg vector ARGZ printable by converting all the '\0's
   except the last into the character SEP.  */
void __argz_stringify __P ((char *argz, size_t len, int sep));
void argz_stringify __P ((char *argz, size_t len, int sep));

/* Append BUF, of length BUF_LEN to the argz vector in ARGZ & ARGZ_LEN.  */
error_t __argz_append __P ((char **argz, size_t *argz_len,
			    __const char *buf, size_t buf_len));
error_t argz_append __P ((char **argz, size_t *argz_len,
			  __const char *buf, size_t buf_len));

/* Append STR to the argz vector in ARGZ & ARGZ_LEN.  */
error_t __argz_add __P ((char **argz, size_t *argz_len,
			 __const char *str));
error_t argz_add __P ((char **argz, size_t *argz_len,
		       __const char *str));

/* Delete ENTRY from ARGZ & ARGZ_LEN, if it appears there.  */
void __argz_delete __P ((char **argz, size_t *argz_len, char *entry));
void argz_delete __P ((char **argz, size_t *argz_len, char *entry));

/* Insert ENTRY into ARGZ & ARGZ_LEN before BEFORE, which should be an
   existing entry in ARGZ; if BEFORE is NULL, ENTRY is appended to the end.
   Since ARGZ's first entry is the same as ARGZ, argz_insert (ARGZ, ARGZ_LEN,
   ARGZ, ENTRY) will insert ENTRY at the beginning of ARGZ.  If BEFORE is not
   in ARGZ, EINVAL is returned, else if memory can't be allocated for the new
   ARGZ, ENOMEM is returned, else 0.  */
error_t __argz_insert __P ((char **argz, size_t *argz_len,
			    char *before, __const char *entry));
error_t argz_insert __P ((char **argz, size_t *argz_len,
			  char *before, __const char *entry));

/* Returns the next entry in ARGZ & ARGZ_LEN after ENTRY, or NULL if there
   are no more.  If entry is NULL, then the first entry is returned.  This
   behavior allows two convenient iteration styles:

    char *entry = 0;
    while ((entry = argz_next (argz, argz_len, entry)))
      ...;

   or

    char *entry;
    for (entry = argz; entry; entry = argz_next (argz, argz_len, entry))
      ...;
*/
extern char *__argz_next __P ((char *argz, size_t __argz_len,
			       __const char *entry));
extern char *argz_next __P ((char *argz, size_t __argz_len,
			     __const char *entry));

#if defined (__OPTIMIZE__) && __GNUC__ >= 2
extern inline char *
__argz_next (char *argz, size_t argz_len, const char *entry)
{
  if (entry)
    {
      if (entry < argz + argz_len)
	entry = strchr (entry, '\0') + 1;

      return entry >= argz + argz_len ? NULL : (char *) entry;
    }
  else
    if (argz_len > 0)
      return argz;
    else
      return 0;
}
extern inline char *
argz_next (char *argz, size_t argz_len, const char *entry)
{
  return __argz_next (argz, argz_len, entry);
}
#endif /* optimizing GCC2 */

#endif /* __ARGZ_H__ */
