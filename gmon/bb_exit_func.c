/* Copyright (C) 1996-2015 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by David Mosberger (davidm@cs.arizona.edu).

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

/* __bb_exit_func() dumps all the basic-block statistics linked into
   the __bb_head chain to .d files.  */

#include <sys/gmon.h>
#include <sys/gmon_out.h>
#include <sys/types.h>

#include <stdio.h>
#include <stdio_ext.h>
#include <string.h>

#define OUT_NAME	"gmon.out"


extern struct __bb *__bb_head attribute_hidden;


void
__bb_exit_func (void)
{
  const int version = GMON_VERSION;
  struct gmon_hdr ghdr;
  struct __bb *ptr;
  FILE *fp;
  fp = fopen (OUT_NAME, "wb");
  if (!fp)
    {
      perror (OUT_NAME);
      return;
    }
  /* No threads use this stream.  */
  __fsetlocking (fp, FSETLOCKING_BYCALLER);

  memcpy (&ghdr.cookie[0], GMON_MAGIC, 4);
  memcpy (&ghdr.version, &version, sizeof (version));
  fwrite_unlocked (&ghdr, sizeof (ghdr), 1, fp);

  for (ptr = __bb_head; ptr != 0; ptr = ptr->next)
    {
      u_int ncounts = ptr->ncounts;
      u_char tag;
      u_int i;

      tag = GMON_TAG_BB_COUNT;
      fwrite_unlocked (&tag, sizeof (tag), 1, fp);
      fwrite_unlocked (&ncounts, sizeof (ncounts), 1, fp);

      for (i = 0; i < ncounts; ++i)
	{
	  fwrite_unlocked (&ptr->addresses[i], sizeof (ptr->addresses[0]), 1,
			   fp);
	  fwrite_unlocked (&ptr->counts[i], sizeof (ptr->counts[0]), 1, fp);
	}
    }
  fclose (fp);
}
