/* Copyright (C) 1991, 1992, 1996, 1998 Free Software Foundation, Inc.
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
#include <stdio.h>
#include <grp.h>

#ifdef USE_IN_LIBIO
# define flockfile(s) _IO_flockfile (s)
# define funlockfile(s) _IO_funlockfile (s)
#endif

#define _S(x)	x ? x : ""

/* Write an entry to the given stream.
   This must know the format of the group file.  */
int
putgrent (gr, stream)
     const struct group *gr;
     FILE *stream;
{
  int retval;

  if (gr == NULL || stream == NULL)
    {
      __set_errno (EINVAL);
      return -1;
    }

  flockfile (stream);

  retval = fprintf (stream, "%s:%s:%u:",
		    gr->gr_name, _S (gr->gr_passwd), gr->gr_gid);
  if (retval < 0)
    return -1;

  if (gr->gr_mem != NULL)
    {
      int i;

      for (i = 0 ; gr->gr_mem[i] != NULL; i++)
	if (fprintf (stream, "%c%s", i == 0 ? ':' : ',', gr->gr_mem[i]) < 0)
	  {
	    /* What else can we do?  */
	    funlockfile (stream);
	    return -1;
	  }
    }

  retval = fputc_unlocked ('\n', stream);

  funlockfile (stream);

  return retval < 0 ? -1 : 0;
}
