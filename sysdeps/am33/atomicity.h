/* Low-level functions for atomic operations.  AM33 version.
   Copyright 1999, 2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Alexandre Oliva <aoliva@redhat.com>.
   Based on ../sparc/sparc32/atomicity.h

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

#ifndef _ATOMICITY_H
#define _ATOMICITY_H	1

#include <inttypes.h>

#define __acquire_lock(lock) \
  __asm__ __volatile__("1:	bset	%1, (%0)\n\t"		\
		       "	beq	1b"			\
		       : : "a" (&(lock)), "d" (1)		\
		       : "memory")

#define __release_lock(lock) lock = 0

static int
__attribute__ ((unused))
exchange_and_add (volatile uint32_t *mem, int val)
{
  static unsigned char lock;
  int result;

  __acquire_lock (lock);

  result = *mem;
  *mem += val;

  __release_lock (lock);

  return result;
}

static void
__attribute__ ((unused))
atomic_add (volatile uint32_t *mem, int val)
{
  static unsigned char lock;

  __acquire_lock (lock);

  *mem += val;

  __release_lock (lock);
}

static int
__attribute__ ((unused))
compare_and_swap (volatile long int *p, long int oldval, long int newval)
{
  static unsigned char lock;
  int ret;

  __acquire_lock (lock);

  if (*p != oldval)
    ret = 0;
  else
    {
      *p = newval;
      ret = 1;
    }

  __release_lock (lock);

  return ret;
}

#endif /* atomicity.h */
