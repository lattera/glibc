/* Copyright (C) 2002, 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2002.

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

#ifndef _ATOMIC_H
#define _ATOMIC_H	1

#include <stdlib.h>

#include <bits/atomic.h>


#ifndef atomic_compare_and_exchange_acq
# define atomic_compare_and_exchange_acq(mem, newval, oldval) \
  ({ __typeof (__arch_compare_and_exchange_32_acq (mem, newval, oldval))      \
	 __result;							      \
     if (sizeof (*mem) == 1)						      \
       __result = __arch_compare_and_exchange_8_acq (mem, newval, oldval);    \
     else if (sizeof (*mem) == 2)					      \
       __result = __arch_compare_and_exchange_16_acq (mem, newval, oldval);   \
     else if (sizeof (*mem) == 4)					      \
       __result = __arch_compare_and_exchange_32_acq (mem, newval, oldval);   \
     else if (sizeof (*mem) == 8)					      \
       __result = __arch_compare_and_exchange_64_acq (mem, newval, oldval);   \
     else								      \
       abort ();							      \
     __result; })
#endif


#ifndef atomic_compare_and_exchange_rel
# define atomic_compare_and_exchange_rel(mem, oldval, newval) \
  atomic_compare_and_exchange_acq (mem, oldval, newval)
#endif


/* Store NEWVALUE in *MEM and return the old value.  */
#ifndef atomic_exchange
# define atomic_exchange(mem, newvalue) \
  ({ __typeof (*mem) __oldval;						      \
     __typeof (mem) __memp = (mem);					      \
     __typeof (*mem) __value = (newvalue);				      \
									      \
     do									      \
       __oldval = (*__memp);						      \
     while (__builtin_expect (atomic_compare_and_exchange_acq (__memp,	      \
							       __value,	      \
							       __oldval), 0));\
									      \
     __oldval; })
#endif


/* Add VALUE to *MEM and return the old value of *MEM.  */
#ifndef atomic_exchange_and_add
# define atomic_exchange_and_add(mem, value) \
  ({ __typeof (*mem) __oldval;						      \
     __typeof (mem) __memp = (mem);					      \
     __typeof (*mem) __value = (value);					      \
									      \
     do									      \
       __oldval = (*__memp);						      \
     while (__builtin_expect (atomic_compare_and_exchange_acq (__memp,	      \
							       __oldval	      \
							       + __value,     \
							       __oldval), 0));\
									      \
     __oldval; })
#endif


#ifndef atomic_add
# define atomic_add(mem, value) (void) atomic_exchange_and_add (mem, value)
#endif


#ifndef atomic_increment
# define atomic_increment(mem) atomic_add (mem, 1)
#endif


#ifndef atomic_increment_and_test
# define atomic_increment_and_test(mem) \
  (atomic_exchange_and_add (mem, 1) == 0)
#endif


#ifndef atomic_decrement
# define atomic_decrement(mem) atomic_add (mem, -1)
#endif


#ifndef atomic_decrement_and_test
# define atomic_decrement_and_test(mem) \
  (atomic_exchange_and_add (mem, -1) == 0)
#endif


/* Decrement *MEM if it is > 0, and return the old value.  */
#ifndef atomic_decrement_if_positive
# define atomic_decrement_if_positive(mem) \
  ({ __typeof (*mem) __oldval;						      \
     __typeof (mem) __memp;						      \
									      \
     __val = *__memp;							      \
     do									      \
       {								      \
	 __oldval = *__memp;						      \
	 if (__builtin_expect (__oldval <= 0, 0))			      \
	   break;							      \
       }								      \
     while (__builtin_expect (atomic_compare_and_exchange_acq (__memp,	      \
							       __oldval - 1,  \
							       __oldval), 0));\
     __oldval; })
#endif


#ifndef atomic_add_negative
# define atomic_add_negative(mem, value) \
  (atomic_exchange_and_add (mem, value) < 0)
#endif


#ifndef atomic_add_zero
# define atomic_add_zero(mem, value) \
  (atomic_exchange_and_add (mem, value) == 0)
#endif


#ifndef atomic_bit_set
# define atomic_bit_set(mem, bit) \
  (void) atomic_bit_test_set(mem, bit)
#endif


#ifndef atomic_bit_test_set
# define atomic_bit_test_set(mem, bit) \
  ({ __typeof (*mem) __oldval;						      \
     __typeof (mem) __memp = (mem);					      \
     __typeof (*mem) __mask = (1 << (bit));				      \
									      \
     do									      \
       __oldval = (*__memp);						      \
     while (__builtin_expect (atomic_compare_and_exchange_acq (__memp,	      \
							       __oldval	      \
							       | __mask,      \
							       __oldval), 0));\
									      \
     __oldval & __mask; })
#endif


#ifndef atomic_full_barrier
# define atomic_full_barrier() __asm ("" ::: "memory")
#endif


#ifndef atomic_read_barrier
# define atomic_read_barrier() atomic_full_barrier()
#endif


#ifndef atomic_write_barrier
# define atomic_write_barrier() atomic_full_barrier()
#endif

#endif	/* atomic.h */
