/* Copyright (C) 1993, 1995 Free Software Foundation, Inc.
Contributed by Ulrich Drepper <drepper@gnu.ai.mit.edu>
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
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#include <search.h>

/* The non-reenttrent version use a global space for storing the table.  */
static struct hsearch_data htab;


/* Define the non-reentrent function using the reentrent counterparts.  */
ENTRY *
hsearch (item, action)
     ENTRY item;
     ACTION action;
{
  ENTRY *result;

  (void) hsearch_r (item, action, &result, &htab);

  return result;
}


int
hcreate (nel)
     unsigned int nel;
{
  return hcreate_r (nel, &htab);
}


void
hdestroy ()
{
  hdestroy_r (&htab);
}
