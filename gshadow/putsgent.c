/* Copyright (C) 2009 Free Software Foundation, Inc.
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

#include <stdbool.h>
#include <stdio.h>
#include <gshadow.h>

#define _S(x)	x ? x : ""


/* Write an entry to the given stream.
   This must know the format of the group file.  */
int
putsgent (const struct sgrp *g, FILE *stream)
{
  int errors = 0;

  _IO_flockfile (stream);

  if (fprintf (stream, "%s:%s:", g->sg_namp, _S (g->sg_passwd)) < 0)
    ++errors;

  bool first = true;
  char **sp = g->sg_adm;
  if (sp != NULL)
    while (*sp != NULL)
      {
	if (fprintf (stream, "%s%s", first ? "" : ",", *sp++) < 0)
	  {
	    ++errors;
	    break;
	  }
	first = false;
      }
  if (putc_unlocked (':', stream) == EOF)
    ++errors;

  first = true;
  sp = g->sg_mem;
  if (sp != NULL)
    while (*sp != NULL)
      {
	if (fprintf (stream, "%s%s", first ? "" : ",", *sp++) < 0)
	  {
	    ++errors;
	    break;
	  }
	first = false;
      }
  if (putc_unlocked ('\n', stream) == EOF)
    ++errors;

  _IO_funlockfile (stream);

  return errors ? -1 : 0;
}
