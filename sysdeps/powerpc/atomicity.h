/* Low-level functions for atomic operations.  PowerPC version.
   Copyright (C) 1997, 1998 Free Software Foundation, Inc.
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

#if BROKEN_PPC_ASM_CR0
# define __ATOMICITY_INLINE /* nothing */
#else
# define __ATOMICITY_INLINE inline
#endif

static __ATOMICITY_INLINE int
__attribute__ ((unused))
exchange_and_add (volatile uint32_t *mem, int val)
{
  int tmp, result;
  __asm__ __volatile__ ("\
0:	lwarx	%0,0,%2
	add	%1,%3,%0
	stwcx.	%1,0,%2
	bne-	0b
" : "=&r"(result), "=&r"(tmp) : "r" (mem), "r"(val) : "cr0");
  return result;
}

static __ATOMICITY_INLINE void
__attribute__ ((unused))
atomic_add (volatile uint32_t *mem, int val)
{
  int tmp;
  __asm__ __volatile__("\
0:	lwarx	%0,0,%1
	add	%0,%2,%0
	stwcx.	%0,0,%1
	bne-	0b
" : "=&r"(tmp) : "r" (mem), "r"(val) : "cr0");
}

static __ATOMICITY_INLINE int
__attribute__ ((unused))
compare_and_swap (volatile long int *p, long int oldval, long int newval)
{
  int result;
  __asm__ __volatile__ ("\
0:	lwarx	%0,0,%1
	xor.	%0,%0,%2
	cntlzw	%0,%0
	bne-	1f
	stwcx.	%3,0,%1
	bne-	0b
1:	srwi	%0,%0,5
" : "=&r"(result) : "r"(p), "r"(oldval), "r"(newval) : "cr0");
  return result;
}

static __ATOMICITY_INLINE long int
__attribute__ ((unused))
always_swap (volatile long int *p, long int newval)
{
  long int result;
  __asm__ __volatile__ ("\
0:	lwarx	%0,0,%1
	stwcx.	%2,0,%1
	bne-	0b
" : "=&r"(result) : "r"(p), "r"(newval) : "cr0");
  return result;
}

static __ATOMICITY_INLINE int
__attribute__ ((unused))
test_and_set (volatile long int *p, long int oldval, long int newval)
{
  int result;
  __asm__ __volatile__ ("\
0:	lwarx	%0,0,%1
	xor.	%0,%0,%2
	cntlzw	%0,%0
	bne-	1f
	stwcx.	%3,0,%1
	bne-	0b
1:	srwi	%0,%0,5
" : "=&r"(result) : "r"(p), "r"(oldval), "r"(newval) : "cr0");
  return result;
}

#endif /* atomicity.h */
