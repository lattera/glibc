/* Low-level functions for atomic operations. Mips version.

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
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, write to the Free Software Foundation, Inc.,
   59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#ifndef _MIPS_ATOMICITY_H
#define _MIPS_ATOMICITY_H    1

#include <sgidefs.h>
#include <inttypes.h>

#if (_MIPS_ISA >= _MIPS_ISA_MIPS2)

static inline int
__attribute__ ((unused))
exchange_and_add (volatile uint32_t *mem, int val)
{
  int result, tmp;

  __asm__ __volatile__
    ("/* Inline exchange & add */\n\t"
     "1:\n\t"
     "ll	%0,%3\n\t"
     "addu	%1,%4,%0\n\t"
     "sc	%1,%2\n\t"
     "beqz	%1,1b\n\t"
     "/* End exchange & add */"
     : "=&r"(result), "=&r"(tmp), "=m"(*mem)
     : "m" (*mem), "r"(val)
     : "memory");

  return result;
}

static inline void
__attribute__ ((unused))
atomic_add (volatile uint32_t *mem, int val)
{
  int result;

  __asm__ __volatile__
    ("/* Inline atomic add */\n\t"
     "1:\n\t"
     "ll	%0,%2\n\t"
     "addu	%0,%3,%0\n\t"
     "sc	%0,%1\n\t"
     "beqz	%0,1b\n\t"
     "/* End atomic add */"
     : "=&r"(result), "=m"(*mem)
     : "m" (*mem), "r"(val)
     : "memory");
}

static inline int
__attribute__ ((unused))
compare_and_swap (volatile long int *p, long int oldval, long int newval)
{
  long int ret;

  __asm__ __volatile__
    ("/* Inline compare & swap */\n\t"
     "1:\n\t"
     "ll	%0,%4\n\t"
     ".set	push\n"
     ".set	noreorder\n\t"
     "bne	%0,%2,2f\n\t"
     "move	%0,%3\n\t"
     ".set	pop\n\t"
     "sc	%0,%1\n\t"
     "beqz	%0,1b\n"
     "2:\n\t"
     "/* End compare & swap */"
     : "=&r" (ret), "=m" (*p)
     : "r" (oldval), "r" (newval), "m" (*p)
     : "memory");

  return ret;
}

#else /* (_MIPS_ISA >= _MIPS_ISA_MIPS2) */

#warning MIPS I atomicity functions are not atomic

static inline int
__attribute__ ((unused))
exchange_and_add (volatile uint32_t *mem, int val)
{
  int result = *mem;
  *mem += val;
  return result;
}

static inline void
__attribute__ ((unused))
atomic_add (volatile uint32_t *mem, int val)
{
  *mem += val;
}

static inline int
__attribute__ ((unused))
compare_and_swap (volatile long int *p, long int oldval, long int newval)
{
  if (*p != oldval)
    return 0;

  *p = newval;
  return 1;
}

#endif /* !(_MIPS_ISA >= _MIPS_ISA_MIPS2) */

#endif /* atomicity.h */
