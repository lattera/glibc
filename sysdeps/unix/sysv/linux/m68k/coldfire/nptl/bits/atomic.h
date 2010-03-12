/* Copyright (C) 2010 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Maxim Kuvyrkov <maxim@codesourcery.com>, 2010.

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

#ifndef _BITS_ATOMIC_H
#define _BITS_ATOMIC_H	1

#include <stdint.h>
#include <sysdep.h>
#include <bits/m68k-vdso.h>

/* Coldfire has no atomic compare-and-exchange operation, but the
   kernel provides userspace atomicity operations.  Use them.  */

typedef int32_t atomic32_t;
typedef uint32_t uatomic32_t;
typedef int_fast32_t atomic_fast32_t;
typedef uint_fast32_t uatomic_fast32_t;

typedef intptr_t atomicptr_t;
typedef uintptr_t uatomicptr_t;
typedef intmax_t atomic_max_t;
typedef uintmax_t uatomic_max_t;

/* The only basic operation needed is compare and exchange.  */
/* For ColdFire we'll have to trap into the kernel mode anyway,
   so trap from the library rather then from the kernel wrapper.  */
#ifdef SHARED
# define atomic_compare_and_exchange_val_acq(mem, newval, oldval)	\
  ({									\
    /* Use temporary variables to workaround call-clobberness of */	\
    /* the registers.  */						\
    __typeof (mem) _mem = mem;						\
    __typeof (oldval) _oldval = oldval;					\
    __typeof (newval) _newval = newval;					\
    register __typeof (mem) _a0 asm ("a0") = _mem;			\
    register __typeof (oldval) _d0 asm ("d0") = _oldval;		\
    register __typeof (newval) _d1 asm ("d1") = _newval;		\
    void *tmp;								\
									\
    asm ("movel #_GLOBAL_OFFSET_TABLE_@GOTPC, %2\n\t"			\
	 "lea (-6, %%pc, %2), %2\n\t"					\
	 "movel " STR_M68K_VDSO_SYMBOL (__vdso_atomic_cmpxchg_32)	\
	 "@GOT(%2), %2\n\t"						\
	 "movel (%2), %2\n\t"						\
	 "jsr (%2)\n\t"							\
	 : "+d" (_d0), "+m" (*_a0), "=&a" (tmp)				\
	 : "a" (_a0), "d" (_d1));					\
    _d0;								\
  })
#else
# define atomic_compare_and_exchange_val_acq(mem, newval, oldval)	\
  ({									\
    /* Use temporary variables to workaround call-clobberness of */	\
    /* the registers.  */						\
    __typeof (mem) _mem = mem;						\
    __typeof (oldval) _oldval = oldval;					\
    __typeof (newval) _newval = newval;					\
    register __typeof (oldval) _d0 asm ("d0")				\
      = SYS_ify (atomic_cmpxchg_32);					\
    register __typeof (mem) _a0 asm ("a0") = _mem;			\
    register __typeof (oldval) _d2 asm ("d2") = _oldval;		\
    register __typeof (newval) _d1 asm ("d1") = _newval;		\
									\
    asm ("trap #0"							\
	 : "+d" (_d0), "+m" (*_a0)					\
	 : "a" (_a0), "d" (_d2), "d" (_d1));				\
    _d0;								\
  })
#endif

#ifdef SHARED
# define atomic_full_barrier()					 \
  ({								 \
    void *tmp;							 \
								 \
    asm ("movel #_GLOBAL_OFFSET_TABLE_@GOTPC, %0\n\t"		 \
	 "lea (-6, %pc, %0), %0\n\t"				 \
	 "movel " STR_M68K_VDSO_SYMBOL (__vdso_atomic_barrier)	 \
	 "@GOT(%0), %0\n\t"					 \
	 "movel (%0), %0\n\t"					 \
	 "jsr (%0)\n\t"						 \
	 : "=&a" (tmp));					 \
  })
#else
# define atomic_full_barrier()				\
  (INTERNAL_SYSCALL (atomic_barrier, , 0), (void) 0)
#endif

#endif
