/* Low-level functions for atomic operations.  64 bit S/390 version.
   Copyright (C) 2001 Free Software Foundation, Inc.
   Contributed by Martin Schwidefsky (schwidefsky@de.ibm.com).
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

static inline int
__attribute__ ((unused))
exchange_and_add (volatile uint32_t *mem, int val)
{
  int result;
  __asm__ __volatile__(
        "       L     %0,%2\n"
        "       LA    2,%1\n"
        "0:     LR    0,%0\n"
        "       AR    0,%3\n"
        "       CS    %0,0,0(2)\n"
        "       JL    0b"
        : "=&d" (result), "=m" (*mem)
        : "1" (*mem), "d" (val) : "0", "1", "2" );
  return result;
}

static inline void
__attribute__ ((unused))
atomic_add (volatile uint32_t *mem, int val)
{
  __asm__ __volatile__(
        "       LA    2,%0\n"
	"0:     L     0,%1\n"
        "       LR    1,0\n"
        "       AR    1,%2\n"
        "       CS    0,1,0(2)\n"
        "       JL    0b"
        : "=m" (*mem) : "0" (*mem), "d" (val) : "0", "1", "2" );
}

static inline int
__attribute__ ((unused))
compare_and_swap (volatile long int *p, long int oldval, long int newval)
{
  int retval;

  __asm__ __volatile__(
        "  la   1,%1\n"
        "  lgr  0,%2\n"
        "  csg  0,%3,0(1)\n"
        "  ipm  %0\n"
        "  srl  %0,28\n"
        "0:"
        : "=&r" (retval), "+m" (*p)
        : "d" (oldval) , "d" (newval)
        : "memory", "0", "1", "cc");
  return !retval;
}

#endif /* atomicity.h */
