/* Atomic operations.  PowerPC32 version.
   Copyright (C) 2003, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Paul Mackerras <paulus@au.ibm.com>, 2003.

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

/*
 * The 32-bit exchange_bool is different on powerpc64 because the subf
 * does signed 64-bit arthmatic while the lwarx is 32-bit unsigned
 * (a load word and zero (high 32) form).  So powerpc64 has a slightly
 * different version in sysdeps/powerpc/powerpc64/bits/atomic.h.
 */
# define __arch_compare_and_exchange_bool_32_acq(mem, newval, oldval)         \
({									      \
  unsigned int __tmp;							      \
  __asm __volatile (							      \
		    "1:	lwarx	%0,0,%1\n"				      \
		    "	subf.	%0,%2,%0\n"				      \
		    "	bne	2f\n"					      \
		    "	stwcx.	%3,0,%1\n"				      \
		    "	bne-	1b\n"					      \
		    "2:	" __ARCH_ACQ_INSTR				      \
		    : "=&r" (__tmp)					      \
		    : "b" (mem), "r" (oldval), "r" (newval)		      \
		    : "cr0", "memory");					      \
  __tmp != 0;								      \
})

# define __arch_compare_and_exchange_bool_32_rel(mem, newval, oldval)	      \
({									      \
  unsigned int __tmp;							      \
  __asm __volatile (__ARCH_REL_INSTR "\n"				      \
		    "1:	lwarx	%0,0,%1\n"				      \
		    "	subf.	%0,%2,%0\n"				      \
		    "	bne	2f\n"					      \
		    "	stwcx.	%3,0,%1\n"				      \
		    "	bne-	1b\n"					      \
		    "2:	"						      \
		    : "=&r" (__tmp)					      \
		    : "b" (mem), "r" (oldval), "r" (newval)		      \
		    : "cr0", "memory");					      \
  __tmp != 0;								      \
})

/* Powerpc32 processors don't implement the 64-bit (doubleword) forms of
   load and reserve (ldarx) and store conditional (stdcx.) instructions.
   So for powerpc32 we stub out the 64-bit forms.  */
# define __arch_compare_and_exchange_bool_64_acq(mem, newval, oldval) \
  (abort (), 0)

# define __arch_compare_and_exchange_val_64_acq(mem, newval, oldval) \
  (abort (), (__typeof (*mem)) 0)

# define __arch_compare_and_exchange_bool_64_rel(mem, newval, oldval) \
  (abort (), 0)

# define __arch_compare_and_exchange_val_64_rel(mem, newval, oldval) \
  (abort (), (__typeof (*mem)) 0)

# define __arch_atomic_exchange_64_acq(mem, value) \
    ({ abort (); (*mem) = (value); })

# define __arch_atomic_exchange_64_rel(mem, value) \
    ({ abort (); (*mem) = (value); })

# define __arch_atomic_exchange_and_add_64(mem, value) \
    ({ abort (); (*mem) = (value); })

# define __arch_atomic_increment_val_64(mem) \
    ({ abort (); (*mem)++; })

# define __arch_atomic_decrement_val_64(mem) \
    ({ abort (); (*mem)--; })

# define __arch_atomic_decrement_if_positive_64(mem) \
    ({ abort (); (*mem)--; })

/*
 * Older powerpc32 processors don't support the new "light weight"
 * sync (lwsync).  So the only safe option is to use normal sync
 * for all powerpc32 applications.
 */
# define atomic_read_barrier()	__asm ("sync" ::: "memory")

/*
 * Include the rest of the atomic ops macros which are common to both
 * powerpc32 and powerpc64.
 */
#include_next <bits/atomic.h>
