/* Copyright (C) 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Martin Schwidefsky <schwidefsky@de.ibm.com>, 2003.

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


#define __arch_compare_and_exchange_bool_8_acq(mem, newval, oldval) \
  (abort (), 0)

#define __arch_compare_and_exchange_bool_16_acq(mem, newval, oldval) \
  (abort (), 0)

#define __arch_compare_and_exchange_bool_32_acq(mem, newval, oldval) \
  ({ unsigned int *__mem = (unsigned int *) (mem);			      \
     unsigned int __old = (unsigned int) (oldval);			      \
     unsigned int __cmp = __old;					      \
     __asm __volatile ("cs %0,%2,%1"					      \
		       : "+d" (__old), "=Q" (*__mem)			      \
		       : "d" (newval), "m" (*__mem) : "cc" );		      \
     __cmp != __old; })

#ifdef __s390x__
# define __arch_compare_and_exchange_bool_64_acq(mem, newval, oldval) \
  ({ unsigned long int *__mem = (unsigned long int *) (mem);		      \
     unsigned long int __old = (unsigned long int) (oldval);		      \
     unsigned long int __cmp = __old;					      \
     __asm __volatile ("csg %0,%2,%1"					      \
		       : "+d" (__old), "=Q" (*__mem)			      \
		       : "d" (newval), "m" (*__mem) : "cc" );		      \
     __cmp != __old; })
#else
/* For 31 bit we do not really need 64-bit compare-and-exchange. We can
   implement them by use of the csd instruction. The straightforward
   implementation causes warnings so we skip the definition for now.  */
# define __arch_compare_and_exchange_bool_64_acq(mem, newval, oldval) \
  (abort (), 0)
#endif
