/* Copyright (C) 1996 Free Software Foundation, Inc.
This file is part of the GNU C Library.
Contributed by Ulrich Drepper, <drepper@gnu.ai.mit.edu>.

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
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <alloca.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "charset.h"


static void
insert_char (struct linereader *lr, struct charset_t *cs, int bytes,
	     unsigned int value, const char *from, const char *to);


void
charset_new_char (struct linereader *lr, struct charset_t *cs, int bytes,
		  unsigned int value, const char *from, const char *to)
{
  if (bytes < cs->mb_cur_min)
    lr_error (lr, _("too few bytes in character encoding"));
  else if (bytes > cs->mb_cur_max)
    lr_error (lr, _("too many bytes in character encoding"));
  else
    insert_char (lr, cs, bytes, value, from, to);
}


void
charset_new_unicode (struct linereader *lr, struct charset_t *cs, int bytes,
		     unsigned int value, const char *from, const char *to)
{
  /* For now: perhaps <Uxxxx> support will be removed again... */
  insert_char (lr, cs, bytes, value, from, to);
}


unsigned int
charset_find_value (const struct charset_t *cs, const char *name, size_t len)
{
  void *result;

  if (find_entry ((hash_table *) &cs->char_table, name, len, &result) < 0)
    return ILLEGAL_CHAR_VALUE;

  return (unsigned int) result;
}


static void
insert_char (struct linereader *lr, struct charset_t *cs, int bytes,
	     unsigned int value, const char *from, const char *to)
{
  const char *cp;
  char *buf;
  int prefix_len, len1, len2;
  unsigned int from_nr, to_nr, cnt;

  if (to == NULL)
    {
      if (insert_entry (&cs->char_table, from, strlen (from), (void *) value)
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

  from_nr = strtoul (&from[prefix_len], NULL, 10);
  to_nr = strtoul (&to[prefix_len], NULL, 10);

  if (from_nr > to_nr)
    {
      lr_error (lr, _("upper limit in range is not smaller then lower limit"));
      return;
    }

  buf = alloca (len1 + 1);
  memcpy (buf, from, prefix_len);

  for (cnt = from_nr; cnt <= to_nr; ++cnt)
    {
      sprintf (&buf[prefix_len], "%0d", cnt);

      if (insert_entry (&cs->char_table, buf, len1, (void *) cnt) < 0)
	lr_error (lr, _("duplicate character name `%s'"), buf);
    }
}
