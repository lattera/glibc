/* Copyright (C) 2001 Free Software Foundation, Inc.
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

#include <stdio.h>
#include <stdlib.h>

int global;
int *ip;

extern void dummy (void);
extern void foo (void *p);

void
dummy (void)
{
  printf ("This is %s\n", __FUNCTION__);
  *ip = global = 1;
}


void
foo (void *p)
{
  extern void *__dso_handle __attribute__ ((__weak__));
  printf ("This is %s\n", __FUNCTION__);
  atexit (dummy);
  if (&__dso_handle) puts ("have dso handle"); else puts ("no dso handle");
  ip = p;
}
