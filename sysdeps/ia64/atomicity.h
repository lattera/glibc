/* Low-level functions for atomic operations.  IA-64 version.
   Copyright (C) 2001 Free Software Foundation, Inc.
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

#ifndef _ATOMICITY_H
#define _ATOMICITY_H	1

#include <inttypes.h>
#include <ia64intrin.h>


static inline uint32_t
__attribute__ ((unused))
exchange_and_add (volatile uint32_t *mem, uint32_t val)
{
  return __sync_fetch_and_add (mem, val);
}

static inline void
__attribute__ ((unused))
atomic_add (volatile uint32_t *mem, int val)
{
  __sync_fetch_and_add (mem, val);
}

static inline int
__attribute__ ((unused))
compare_and_swap (volatile long int *p, long int oldval, long int newval)
{
  return __sync_bool_compare_and_swap (p, oldval, newval);
}

#endif /* atomicity.h */
