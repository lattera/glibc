/* Copyright (C) 2002-2016 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <stdint.h>
#include <tls.h>	/* For tcbhead_t.  */
#include <libc-internal.h>


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


#ifndef LOCK_PREFIX
# ifdef UP
#  define LOCK_PREFIX	/* nothing */
# else
#  define LOCK_PREFIX "lock;"
# endif
#endif

#define __HAVE_64B_ATOMICS 1
#define USE_ATOMIC_COMPILER_BUILTINS 1

#define atomic_compare_and_exchange_val_acq(mem, newval, oldval) \
  __sync_val_compare_and_swap (mem, oldval, newval)
#define atomic_compare_and_exchange_bool_acq(mem, newval, oldval) \
  (! __sync_bool_compare_and_swap (mem, oldval, newval))


#define __arch_c_compare_and_exchange_val_8_acq(mem, newval, oldval) \
  ({ __typeof (*mem) ret;						      \
    __asm __volatile ("cmpl $0, %%fs:%P5\n\t"				      \
		      "je 0f\n\t"					      \
		      "lock\n"						      \
		       "0:\tcmpxchgb %b2, %1"				      \
		       : "=a" (ret), "=m" (*mem)			      \
		       : "q" (newval), "m" (*mem), "0" (oldval),	      \
			 "i" (offsetof (tcbhead_t, multiple_threads)));	      \
     ret; })

#define __arch_c_compare_and_exchange_val_16_acq(mem, newval, oldval) \
  ({ __typeof (*mem) ret;						      \
    __asm __volatile ("cmpl $0, %%fs:%P5\n\t"				      \
		      "je 0f\n\t"					      \
		      "lock\n"						      \
		       "0:\tcmpxchgw %w2, %1"				      \
		       : "=a" (ret), "=m" (*mem)			      \
		       : "q" (newval), "m" (*mem), "0" (oldval),	      \
			 "i" (offsetof (tcbhead_t, multiple_threads)));	      \
     ret; })

#define __arch_c_compare_and_exchange_val_32_acq(mem, newval, oldval) \
  ({ __typeof (*mem) ret;						      \
    __asm __volatile ("cmpl $0, %%fs:%P5\n\t"				      \
		      "je 0f\n\t"					      \
		      "lock\n"						      \
		       "0:\tcmpxchgl %2, %1"				      \
		       : "=a" (ret), "=m" (*mem)			      \
		       : "q" (newval), "m" (*mem), "0" (oldval),	      \
			 "i" (offsetof (tcbhead_t, multiple_threads)));	      \
     ret; })

#define __arch_c_compare_and_exchange_val_64_acq(mem, newval, oldval) \
  ({ __typeof (*mem) ret;						      \
     __asm __volatile ("cmpl $0, %%fs:%P5\n\t"				      \
		       "je 0f\n\t"					      \
		       "lock\n"						      \
		       "0:\tcmpxchgq %q2, %1"				      \
		       : "=a" (ret), "=m" (*mem)			      \
		       : "q" ((atomic64_t) cast_to_integer (newval)),	      \
			 "m" (*mem),					      \
			 "0" ((atomic64_t) cast_to_integer (oldval)),	      \
			 "i" (offsetof (tcbhead_t, multiple_threads)));	      \
     ret; })


/* Note that we need no lock prefix.  */
#define atomic_exchange_acq(mem, newvalue) \
  ({ __typeof (*mem) result;						      \
     if (sizeof (*mem) == 1)						      \
       __asm __volatile ("xchgb %b0, %1"				      \
			 : "=q" (result), "=m" (*mem)			      \
			 : "0" (newvalue), "m" (*mem));			      \
     else if (sizeof (*mem) == 2)					      \
       __asm __volatile ("xchgw %w0, %1"				      \
			 : "=r" (result), "=m" (*mem)			      \
			 : "0" (newvalue), "m" (*mem));			      \
     else if (sizeof (*mem) == 4)					      \
       __asm __volatile ("xchgl %0, %1"					      \
			 : "=r" (result), "=m" (*mem)			      \
			 : "0" (newvalue), "m" (*mem));			      \
     else								      \
       __asm __volatile ("xchgq %q0, %1"				      \
			 : "=r" (result), "=m" (*mem)			      \
			 : "0" ((atomic64_t) cast_to_integer (newvalue)),     \
			   "m" (*mem));					      \
     result; })


#define __arch_exchange_and_add_body(lock, mem, value)			      \
  ({ __typeof (*mem) result;						      \
     if (sizeof (*mem) == 1)						      \
       __asm __volatile (lock "xaddb %b0, %1"				      \
			 : "=q" (result), "=m" (*mem)			      \
			 : "0" (value), "m" (*mem),			      \
			   "i" (offsetof (tcbhead_t, multiple_threads)));     \
     else if (sizeof (*mem) == 2)					      \
       __asm __volatile (lock "xaddw %w0, %1"				      \
			 : "=r" (result), "=m" (*mem)			      \
			 : "0" (value), "m" (*mem),			      \
			   "i" (offsetof (tcbhead_t, multiple_threads)));     \
     else if (sizeof (*mem) == 4)					      \
       __asm __volatile (lock "xaddl %0, %1"				      \
			 : "=r" (result), "=m" (*mem)			      \
			 : "0" (value), "m" (*mem),			      \
			   "i" (offsetof (tcbhead_t, multiple_threads)));     \
     else								      \
       __asm __volatile (lock "xaddq %q0, %1"				      \
			 : "=r" (result), "=m" (*mem)			      \
			 : "0" ((atomic64_t) cast_to_integer (value)),	      \
			   "m" (*mem),					      \
			   "i" (offsetof (tcbhead_t, multiple_threads)));     \
     result; })

#define atomic_exchange_and_add(mem, value) \
  __sync_fetch_and_add (mem, value)

#define __arch_exchange_and_add_cprefix \
  "cmpl $0, %%fs:%P4\n\tje 0f\n\tlock\n0:\t"

#define catomic_exchange_and_add(mem, value) \
  __arch_exchange_and_add_body (__arch_exchange_and_add_cprefix, mem, value)


#define __arch_add_body(lock, pfx, mem, value)				      \
  do {									      \
    if (__builtin_constant_p (value) && (value) == 1)			      \
      pfx##_increment (mem);						      \
    else if (__builtin_constant_p (value) && (value) == -1)		      \
      pfx##_decrement (mem);						      \
    else if (sizeof (*mem) == 1)					      \
      __asm __volatile (lock "addb %b1, %0"				      \
			: "=m" (*mem)					      \
			: "iq" (value), "m" (*mem),			      \
			  "i" (offsetof (tcbhead_t, multiple_threads)));      \
    else if (sizeof (*mem) == 2)					      \
      __asm __volatile (lock "addw %w1, %0"				      \
			: "=m" (*mem)					      \
			: "ir" (value), "m" (*mem),			      \
			  "i" (offsetof (tcbhead_t, multiple_threads)));      \
    else if (sizeof (*mem) == 4)					      \
      __asm __volatile (lock "addl %1, %0"				      \
			: "=m" (*mem)					      \
			: "ir" (value), "m" (*mem),			      \
			  "i" (offsetof (tcbhead_t, multiple_threads)));      \
    else								      \
      __asm __volatile (lock "addq %q1, %0"				      \
			: "=m" (*mem)					      \
			: "ir" ((atomic64_t) cast_to_integer (value)),	      \
			  "m" (*mem),					      \
			  "i" (offsetof (tcbhead_t, multiple_threads)));      \
  } while (0)

#define atomic_add(mem, value) \
  __arch_add_body (LOCK_PREFIX, atomic, mem, value)

#define __arch_add_cprefix \
  "cmpl $0, %%fs:%P3\n\tje 0f\n\tlock\n0:\t"

#define catomic_add(mem, value) \
  __arch_add_body (__arch_add_cprefix, catomic, mem, value)


#define atomic_add_negative(mem, value) \
  ({ unsigned char __result;						      \
     if (sizeof (*mem) == 1)						      \
       __asm __volatile (LOCK_PREFIX "addb %b2, %0; sets %1"		      \
			 : "=m" (*mem), "=qm" (__result)		      \
			 : "iq" (value), "m" (*mem));			      \
     else if (sizeof (*mem) == 2)					      \
       __asm __volatile (LOCK_PREFIX "addw %w2, %0; sets %1"		      \
			 : "=m" (*mem), "=qm" (__result)		      \
			 : "ir" (value), "m" (*mem));			      \
     else if (sizeof (*mem) == 4)					      \
       __asm __volatile (LOCK_PREFIX "addl %2, %0; sets %1"		      \
			 : "=m" (*mem), "=qm" (__result)		      \
			 : "ir" (value), "m" (*mem));			      \
     else								      \
       __asm __volatile (LOCK_PREFIX "addq %q2, %0; sets %1"		      \
			 : "=m" (*mem), "=qm" (__result)		      \
			 : "ir" ((atomic64_t) cast_to_integer (value)),	      \
			   "m" (*mem));					      \
     __result; })


#define atomic_add_zero(mem, value) \
  ({ unsigned char __result;						      \
     if (sizeof (*mem) == 1)						      \
       __asm __volatile (LOCK_PREFIX "addb %b2, %0; setz %1"		      \
			 : "=m" (*mem), "=qm" (__result)		      \
			 : "iq" (value), "m" (*mem));			      \
     else if (sizeof (*mem) == 2)					      \
       __asm __volatile (LOCK_PREFIX "addw %w2, %0; setz %1"		      \
			 : "=m" (*mem), "=qm" (__result)		      \
			 : "ir" (value), "m" (*mem));			      \
     else if (sizeof (*mem) == 4)					      \
       __asm __volatile (LOCK_PREFIX "addl %2, %0; setz %1"		      \
			 : "=m" (*mem), "=qm" (__result)		      \
			 : "ir" (value), "m" (*mem));			      \
     else								      \
       __asm __volatile (LOCK_PREFIX "addq %q2, %0; setz %1"		      \
			 : "=m" (*mem), "=qm" (__result)		      \
			 : "ir" ((atomic64_t) cast_to_integer (value)),	      \
			   "m" (*mem));					      \
     __result; })


#define __arch_increment_body(lock, mem) \
  do {									      \
    if (sizeof (*mem) == 1)						      \
      __asm __volatile (lock "incb %b0"					      \
			: "=m" (*mem)					      \
			: "m" (*mem),					      \
			  "i" (offsetof (tcbhead_t, multiple_threads)));      \
    else if (sizeof (*mem) == 2)					      \
      __asm __volatile (lock "incw %w0"					      \
			: "=m" (*mem)					      \
			: "m" (*mem),					      \
			  "i" (offsetof (tcbhead_t, multiple_threads)));      \
    else if (sizeof (*mem) == 4)					      \
      __asm __volatile (lock "incl %0"					      \
			: "=m" (*mem)					      \
			: "m" (*mem),					      \
			  "i" (offsetof (tcbhead_t, multiple_threads)));      \
    else								      \
      __asm __volatile (lock "incq %q0"					      \
			: "=m" (*mem)					      \
			: "m" (*mem),					      \
			  "i" (offsetof (tcbhead_t, multiple_threads)));      \
  } while (0)

#define atomic_increment(mem) __arch_increment_body (LOCK_PREFIX, mem)

#define __arch_increment_cprefix \
  "cmpl $0, %%fs:%P2\n\tje 0f\n\tlock\n0:\t"

#define catomic_increment(mem) \
  __arch_increment_body (__arch_increment_cprefix, mem)


#define atomic_increment_and_test(mem) \
  ({ unsigned char __result;						      \
     if (sizeof (*mem) == 1)						      \
       __asm __volatile (LOCK_PREFIX "incb %b0; sete %1"		      \
			 : "=m" (*mem), "=qm" (__result)		      \
			 : "m" (*mem));					      \
     else if (sizeof (*mem) == 2)					      \
       __asm __volatile (LOCK_PREFIX "incw %w0; sete %1"		      \
			 : "=m" (*mem), "=qm" (__result)		      \
			 : "m" (*mem));					      \
     else if (sizeof (*mem) == 4)					      \
       __asm __volatile (LOCK_PREFIX "incl %0; sete %1"			      \
			 : "=m" (*mem), "=qm" (__result)		      \
			 : "m" (*mem));					      \
     else								      \
       __asm __volatile (LOCK_PREFIX "incq %q0; sete %1"		      \
			 : "=m" (*mem), "=qm" (__result)		      \
			 : "m" (*mem));					      \
     __result; })


#define __arch_decrement_body(lock, mem) \
  do {									      \
    if (sizeof (*mem) == 1)						      \
      __asm __volatile (lock "decb %b0"					      \
			: "=m" (*mem)					      \
			: "m" (*mem),					      \
			  "i" (offsetof (tcbhead_t, multiple_threads)));      \
    else if (sizeof (*mem) == 2)					      \
      __asm __volatile (lock "decw %w0"					      \
			: "=m" (*mem)					      \
			: "m" (*mem),					      \
			  "i" (offsetof (tcbhead_t, multiple_threads)));      \
    else if (sizeof (*mem) == 4)					      \
      __asm __volatile (lock "decl %0"					      \
			: "=m" (*mem)					      \
			: "m" (*mem),					      \
			  "i" (offsetof (tcbhead_t, multiple_threads)));      \
    else								      \
      __asm __volatile (lock "decq %q0"					      \
			: "=m" (*mem)					      \
			: "m" (*mem),					      \
			  "i" (offsetof (tcbhead_t, multiple_threads)));      \
  } while (0)

#define atomic_decrement(mem) __arch_decrement_body (LOCK_PREFIX, mem)

#define __arch_decrement_cprefix \
  "cmpl $0, %%fs:%P2\n\tje 0f\n\tlock\n0:\t"

#define catomic_decrement(mem) \
  __arch_decrement_body (__arch_decrement_cprefix, mem)


#define atomic_decrement_and_test(mem) \
  ({ unsigned char __result;						      \
     if (sizeof (*mem) == 1)						      \
       __asm __volatile (LOCK_PREFIX "decb %b0; sete %1"		      \
			 : "=m" (*mem), "=qm" (__result)		      \
			 : "m" (*mem));					      \
     else if (sizeof (*mem) == 2)					      \
       __asm __volatile (LOCK_PREFIX "decw %w0; sete %1"		      \
			 : "=m" (*mem), "=qm" (__result)		      \
			 : "m" (*mem));					      \
     else if (sizeof (*mem) == 4)					      \
       __asm __volatile (LOCK_PREFIX "decl %0; sete %1"			      \
			 : "=m" (*mem), "=qm" (__result)		      \
			 : "m" (*mem));					      \
     else								      \
       __asm __volatile (LOCK_PREFIX "decq %q0; sete %1"		      \
			 : "=m" (*mem), "=qm" (__result)		      \
			 : "m" (*mem));					      \
     __result; })


#define atomic_bit_set(mem, bit) \
  do {									      \
    if (sizeof (*mem) == 1)						      \
      __asm __volatile (LOCK_PREFIX "orb %b2, %0"			      \
			: "=m" (*mem)					      \
			: "m" (*mem), "iq" (1L << (bit)));		      \
    else if (sizeof (*mem) == 2)					      \
      __asm __volatile (LOCK_PREFIX "orw %w2, %0"			      \
			: "=m" (*mem)					      \
			: "m" (*mem), "ir" (1L << (bit)));		      \
    else if (sizeof (*mem) == 4)					      \
      __asm __volatile (LOCK_PREFIX "orl %2, %0"			      \
			: "=m" (*mem)					      \
			: "m" (*mem), "ir" (1L << (bit)));		      \
    else if (__builtin_constant_p (bit) && (bit) < 32)			      \
      __asm __volatile (LOCK_PREFIX "orq %2, %0"			      \
			: "=m" (*mem)					      \
			: "m" (*mem), "i" (1L << (bit)));		      \
    else								      \
      __asm __volatile (LOCK_PREFIX "orq %q2, %0"			      \
			: "=m" (*mem)					      \
			: "m" (*mem), "r" (1UL << (bit)));		      \
  } while (0)


#define atomic_bit_test_set(mem, bit) \
  ({ unsigned char __result;						      \
     if (sizeof (*mem) == 1)						      \
       __asm __volatile (LOCK_PREFIX "btsb %3, %1; setc %0"		      \
			 : "=q" (__result), "=m" (*mem)			      \
			 : "m" (*mem), "iq" (bit));			      \
     else if (sizeof (*mem) == 2)					      \
       __asm __volatile (LOCK_PREFIX "btsw %3, %1; setc %0"		      \
			 : "=q" (__result), "=m" (*mem)			      \
			 : "m" (*mem), "ir" (bit));			      \
     else if (sizeof (*mem) == 4)					      \
       __asm __volatile (LOCK_PREFIX "btsl %3, %1; setc %0"		      \
			 : "=q" (__result), "=m" (*mem)			      \
			 : "m" (*mem), "ir" (bit));			      \
     else							      	      \
       __asm __volatile (LOCK_PREFIX "btsq %3, %1; setc %0"		      \
			 : "=q" (__result), "=m" (*mem)			      \
			 : "m" (*mem), "ir" (bit));			      \
     __result; })


#define atomic_spin_nop() asm ("rep; nop")


#define __arch_and_body(lock, mem, mask) \
  do {									      \
    if (sizeof (*mem) == 1)						      \
      __asm __volatile (lock "andb %b1, %0"				      \
			: "=m" (*mem)					      \
			: "iq" (mask), "m" (*mem),			      \
			  "i" (offsetof (tcbhead_t, multiple_threads)));      \
    else if (sizeof (*mem) == 2)					      \
      __asm __volatile (lock "andw %w1, %0"				      \
			: "=m" (*mem)					      \
			: "ir" (mask), "m" (*mem),			      \
			  "i" (offsetof (tcbhead_t, multiple_threads)));      \
    else if (sizeof (*mem) == 4)					      \
      __asm __volatile (lock "andl %1, %0"				      \
			: "=m" (*mem)					      \
			: "ir" (mask), "m" (*mem),			      \
			  "i" (offsetof (tcbhead_t, multiple_threads)));      \
    else								      \
      __asm __volatile (lock "andq %q1, %0"				      \
			: "=m" (*mem)					      \
			: "ir" (mask), "m" (*mem),			      \
			  "i" (offsetof (tcbhead_t, multiple_threads)));      \
  } while (0)

#define __arch_cprefix \
  "cmpl $0, %%fs:%P3\n\tje 0f\n\tlock\n0:\t"

#define atomic_and(mem, mask) __arch_and_body (LOCK_PREFIX, mem, mask)

#define catomic_and(mem, mask) __arch_and_body (__arch_cprefix, mem, mask)


#define __arch_or_body(lock, mem, mask)					      \
  do {									      \
    if (sizeof (*mem) == 1)						      \
      __asm __volatile (lock "orb %b1, %0"				      \
			: "=m" (*mem)					      \
			: "iq" (mask), "m" (*mem),			      \
			  "i" (offsetof (tcbhead_t, multiple_threads)));      \
    else if (sizeof (*mem) == 2)					      \
      __asm __volatile (lock "orw %w1, %0"				      \
			: "=m" (*mem)					      \
			: "ir" (mask), "m" (*mem),			      \
			  "i" (offsetof (tcbhead_t, multiple_threads)));      \
    else if (sizeof (*mem) == 4)					      \
      __asm __volatile (lock "orl %1, %0"				      \
			: "=m" (*mem)					      \
			: "ir" (mask), "m" (*mem),			      \
			  "i" (offsetof (tcbhead_t, multiple_threads)));      \
    else								      \
      __asm __volatile (lock "orq %q1, %0"				      \
			: "=m" (*mem)					      \
			: "ir" (mask), "m" (*mem),			      \
			  "i" (offsetof (tcbhead_t, multiple_threads)));      \
  } while (0)

#define atomic_or(mem, mask) __arch_or_body (LOCK_PREFIX, mem, mask)

#define catomic_or(mem, mask) __arch_or_body (__arch_cprefix, mem, mask)

/* We don't use mfence because it is supposedly slower due to having to
   provide stronger guarantees (e.g., regarding self-modifying code).  */
#define atomic_full_barrier() \
    __asm __volatile (LOCK_PREFIX "orl $0, (%%rsp)" ::: "memory")
#define atomic_read_barrier() __asm ("" ::: "memory")
#define atomic_write_barrier() __asm ("" ::: "memory")
