/* Atomic operations.  sparc64 version.
   Copyright (C) 2003-2015 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jakub Jelinek <jakub@redhat.com>, 2003.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <stdint.h>

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

#define __HAVE_64B_ATOMICS 1
#define USE_ATOMIC_COMPILER_BUILTINS 0


#define __arch_compare_and_exchange_val_8_acq(mem, newval, oldval) \
  (abort (), (__typeof (*mem)) 0)

#define __arch_compare_and_exchange_val_16_acq(mem, newval, oldval) \
  (abort (), (__typeof (*mem)) 0)

#define __arch_compare_and_exchange_val_32_acq(mem, newval, oldval) \
({									      \
  __typeof (*(mem)) __acev_tmp;						      \
  __typeof (mem) __acev_mem = (mem);					      \
  if (__builtin_constant_p (oldval) && (oldval) == 0)			      \
    __asm __volatile ("cas [%3], %%g0, %0"				      \
		      : "=r" (__acev_tmp), "=m" (*__acev_mem)		      \
		      : "m" (*__acev_mem), "r" (__acev_mem),		      \
		        "0" (newval) : "memory");			      \
  else									      \
    __asm __volatile ("cas [%4], %2, %0"				      \
		      : "=r" (__acev_tmp), "=m" (*__acev_mem)		      \
		      : "r" (oldval), "m" (*__acev_mem), "r" (__acev_mem),    \
		        "0" (newval) : "memory");			      \
  __acev_tmp; })

#define __arch_compare_and_exchange_val_64_acq(mem, newval, oldval) \
({									      \
  __typeof (*(mem)) __acev_tmp;						      \
  __typeof (mem) __acev_mem = (mem);					      \
  if (__builtin_constant_p (oldval) && (oldval) == 0)			      \
    __asm __volatile ("casx [%3], %%g0, %0"				      \
		      : "=r" (__acev_tmp), "=m" (*__acev_mem)		      \
		      : "m" (*__acev_mem), "r" (__acev_mem),		      \
		        "0" ((long) (newval)) : "memory");		      \
  else									      \
    __asm __volatile ("casx [%4], %2, %0"				      \
		      : "=r" (__acev_tmp), "=m" (*__acev_mem)		      \
		      : "r" ((long) (oldval)), "m" (*__acev_mem),	      \
		        "r" (__acev_mem), "0" ((long) (newval)) : "memory");  \
  __acev_tmp; })

#define atomic_exchange_acq(mem, newvalue) \
  ({ __typeof (*(mem)) __oldval, __val;					      \
     __typeof (mem) __memp = (mem);					      \
     __typeof (*(mem)) __value = (newvalue);				      \
									      \
     if (sizeof (*(mem)) == 4)						      \
       __asm ("swap %0, %1"						      \
	      : "=m" (*__memp), "=r" (__oldval)				      \
	      : "m" (*__memp), "1" (__value) : "memory");		      \
     else								      \
       {								      \
	 __val = *__memp;						      \
	 do								      \
	   {								      \
	     __oldval = __val;						      \
	     __val = atomic_compare_and_exchange_val_acq (__memp, __value,    \
							  __oldval);	      \
	   }								      \
         while (__builtin_expect (__val != __oldval, 0));		      \
       }								      \
     __oldval; })

#define atomic_compare_and_exchange_val_24_acq(mem, newval, oldval) \
  atomic_compare_and_exchange_val_acq (mem, newval, oldval)

#define atomic_exchange_24_rel(mem, newval) \
  atomic_exchange_rel (mem, newval)

#define atomic_full_barrier() \
  __asm __volatile ("membar #LoadLoad | #LoadStore"			      \
			  " | #StoreLoad | #StoreStore" : : : "memory")
#define atomic_read_barrier() \
  __asm __volatile ("membar #LoadLoad | #LoadStore" : : : "memory")
#define atomic_write_barrier() \
  __asm __volatile ("membar #LoadStore | #StoreStore" : : : "memory")
