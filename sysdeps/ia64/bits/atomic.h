/* Copyright (C) 2003 Free Software Foundation, Inc.
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

#include <stdint.h>
#include <ia64intrin.h>

typedef int8_t atomic8_t;
typedef uint8_t uatomic8_t;
typedef int_fast8_t atomic_fast8_t;
typedef uint_fast8_t uatomic_fast8_t;

typedef int16_t atomic16_t;
typedef uint16_t uatomic16_t;
typedef int_fast16_t atomic_fast16_t;
typedef uint_fast16_t uatomic_fast16_t;

typedef int32_t atomic32_t;
typedef uint32_t uatomic32_t;
typedef int_fast32_t atomic_fast32_t;
typedef uint_fast32_t uatomic_fast32_t;

typedef int64_t atomic64_t;
typedef uint64_t uatomic64_t;
typedef int_fast64_t atomic_fast64_t;
typedef uint_fast64_t uatomic_fast64_t;

typedef intptr_t atomicptr_t;
typedef uintptr_t uatomicptr_t;
typedef intmax_t atomic_max_t;
typedef uintmax_t uatomic_max_t;


#define __arch_compare_and_exchange_8_acq(mem, newval, oldval) \
  (abort (), 0)

#define __arch_compare_and_exchange_16_acq(mem, newval, oldval) \
  (abort (), 0)

#define __arch_compare_and_exchange_32_acq(mem, newval, oldval) \
  (!__sync_bool_compare_and_swap_si ((int *) (mem), (int) (long) (oldval), \
				     (int) (long) (newval)))

# define __arch_compare_and_exchange_64_acq(mem, newval, oldval) \
  (!__sync_bool_compare_and_swap_di ((long *) (mem), (long) (oldval), \
				     (long) (newval)))

#define __arch_compare_and_exchange_32_val_acq(mem, newval, oldval) \
  __sync_val_compare_and_swap_si ((int *) (mem), (int) (long) (oldval), \
				  (int) (long) (newval))

# define __arch_compare_and_exchange_64_val_acq(mem, newval, oldval) \
  __sync_val_compare_and_swap_di ((long *) (mem), (long) (oldval), \
				  (long) (newval))

# define atomic_exchange_and_add(mem, value) \
  ({									      \
    __typeof (*mem) __oldval, __val;					      \
    __typeof (mem) __memp = (mem);					      \
    __typeof (*mem) __value = (value);					      \
									      \
    __val = *__memp;							      \
    if (sizeof (*mem) == 4)						      \
      do								      \
	{								      \
	  __oldval = __val;						      \
	  __val = __arch_compare_and_exchange_32_val_acq (__memp,	      \
							  __oldval + __value, \
							  __oldval);	      \
	}								      \
      while (__builtin_expect (__val != __oldval, 0));			      \
    else if (sizeof (*mem) == 8)					      \
      do								      \
	{								      \
	  __oldval = __val;						      \
	  __val = __arch_compare_and_exchange_64_val_acq (__memp,	      \
							  __oldval + __value, \
							  __oldval);	      \
	}								      \
      while (__builtin_expect (__val != __oldval, 0));			      \
    else								      \
      abort ();								      \
    __oldval + __value; })
