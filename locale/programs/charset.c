/* Copyright (C) 1996, 1997, 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@gnu.ai.mit.edu>, 1996.

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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <alloca.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "charset.h"


unsigned int
charset_find_value (const hash_table *ht, const char *name, size_t len)
{
  void *result;

  if (find_entry ((hash_table *) ht, name, len, &result) < 0)
    return ILLEGAL_CHAR_VALUE;

  return (unsigned int) ((unsigned long int) result);
}


void
charset_new_char (struct linereader *lr, hash_table *ht, int bytes,
		  unsigned int value, const char *from, const char *to)
{
  char *from_end;
  char *to_end;
  const char *cp;
  char *buf;
  int prefix_len, len1, len2;
  unsigned int from_nr, to_nr, cnt;

  if (to == NULL)
    {
      if (insert_entry (ht, from, strlen (from),
			(void *) (unsigned long int) value)
	  < 0)
	lr_error (lr, _("duplicate character name `%s'"), from);

      return;
    }

  /* We have a range: the names must have names with equal prefixes
     and an equal number of digits, where the second number is greater
     or equal than the first.  */
  len1 = strlen (from);
  len2 = strlen (to);

  if (len1 != len2)
    {
    illegal_range:
      lr_error (lr, _("illegal names for character range"));
      return;
    }

  cp = &from[len1 - 1];
  while (isdigit (*cp) && cp >= from)
    --cp;

  prefix_len = (cp - from) + 1;

  if (cp == &from[len1 - 1] || strncmp (from, to, prefix_len) != 0)
    goto illegal_range;

  errno = 0;
  from_nr = strtoul (&from[prefix_len], &from_end, 10);
  if (*from_end != '\0' || (from_nr == ULONG_MAX && errno == ERANGE)
      || ((to_nr = strtoul (&to[prefix_len], &to_end, 10)) == ULONG_MAX
	  && errno == ERANGE)
      || *to_end != '\0')
    {
      lr_error (lr, _("<%s> and <%s> are illegal names for range"));
      return;
    }

  if (from_nr > to_nr)
    {
      lr_error (lr, _("upper limit in range is not smaller then lower limit"));
      return;
    }

  buf = alloca (len1 + 1);
  memcpy (buf, from, prefix_len);

  for (cnt = from_nr; cnt <= to_nr; ++cnt)
    {
      sprintf (&buf[prefix_len], "%0*d", len1 - prefix_len, cnt);

      if (insert_entry (ht, buf, len1,
			(void *) (unsigned long int) (value + (cnt - from_nr)))
	  < 0)
	lr_error (lr, _("duplicate character name `%s'"), buf);
    }
}
