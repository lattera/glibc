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


#ifndef LOCK
# ifdef UP
#  define LOCK	/* nothing */
# else
#  define LOCK "lock;"
# endif
#endif


#define __arch_compare_and_exchange_val_8_acq(mem, newval, oldval) \
  ({ __typeof (*mem) ret;						      \
     __asm __volatile (LOCK "cmpxchgb %b2, %1; setne %0"		      \
		       : "=a" (ret), "=m" (*mem)			      \
		       : "q" (newval), "1" (*mem), "0" (oldval));	      \
     ret; })

#define __arch_compare_and_exchange_val_16_acq(mem, newval, oldval) \
  ({ __typeof (*mem) ret;						      \
     __asm __volatile (LOCK "cmpxchgw %w2, %1; setne %0"		      \
		       : "=a" (ret), "=m" (*mem)			      \
		       : "r" (newval), "1" (*mem), "0" (oldval));	      \
     ret; })

#define __arch_compare_and_exchange_val_32_acq(mem, newval, oldval) \
  ({ __typeof (*mem) ret;						      \
     __asm __volatile (LOCK "cmpxchgl %2, %1; setne %0"			      \
		       : "=a" (ret), "=m" (*mem)			      \
		       : "r" (newval), "1" (*mem), "0" (oldval));	      \
     ret; })

#define __arch_compare_and_exchange_val_64_acq(mem, newval, oldval) \
  ({ __typeof (*mem) ret;						      \
     __asm __volatile (LOCK "cmpxchgq %q2, %1; setne %0"		      \
		       : "=a" (ret), "=m" (*mem)			      \
		       : "r" (newval), "1" (*mem), "0" (oldval));	      \
     ret; })


/* Note that we need no lock prefix.  */
#define atomic_exchange(mem, newvalue) \
  ({ __typeof (*mem) result;						      \
     if (sizeof (*mem) == 1)						      \
       __asm __volatile ("xchgb %b0, %1"				      \
			 : "=r" (result), "=m" (*mem)			      \
			 : "0" (newvalue), "1" (*mem));			      \
     else if (sizeof (*mem) == 2)					      \
       __asm __volatile ("xchgw %w0, %1"				      \
			 : "=r" (result), "=m" (*mem)			      \
			 : "0" (newvalue), "1" (*mem));			      \
     else if (sizeof (*mem) == 4)					      \
       __asm __volatile ("xchgl %0, %1"					      \
			 : "=r" (result), "=m" (*mem)			      \
			 : "0" (newvalue), "1" (*mem));			      \
     else								      \
       __asm __volatile ("xchgq %q0, %1"				      \
			 : "=r" (result), "=m" (*mem)			      \
			 : "0" (newvalue), "1" (*mem));			      \
     result; })


#define atomic_exchange_and_add(mem, value) \
  ({ __typeof (*mem) result;						      \
     if (sizeof (*mem) == 1)						      \
       __asm __volatile (LOCK "xaddb %b0, %1"				      \
			 : "=r" (result), "=m" (*mem)			      \
			 : "0" (value), "1" (*mem));			      \
     else if (sizeof (*mem) == 2)					      \
       __asm __volatile (LOCK "xaddw %w0, %1"				      \
			 : "=r" (result), "=m" (*mem)			      \
			 : "0" (value), "1" (*mem));			      \
     else if (sizeof (*mem) == 4)					      \
       __asm __volatile (LOCK "xaddl %0, %1"				      \
			 : "=r" (result), "=m" (*mem)			      \
			 : "0" (value), "1" (*mem));			      \
     else								      \
       __asm __volatile (LOCK "xaddq %q0, %1"				      \
			 : "=r" (result), "=m" (*mem)			      \
			 : "0" (value), "1" (*mem));			      \
     result; })


#define atomic_add(mem, value) \
  (void) ({ if (__builtin_constant_p (value) && (value) == 1)		      \
	      atomic_increment (mem);					      \
	    else if (__builtin_constant_p (value) && (value) == 1)	      \
	      atomic_decrement (mem);					      \
	    else if (sizeof (*mem) == 1)				      \
	      __asm __volatile (LOCK "addb %b1, %0"			      \
				: "=m" (*mem)				      \
				: "ir" (value), "0" (*mem));		      \
	    else if (sizeof (*mem) == 2)				      \
	      __asm __volatile (LOCK "addw %w1, %0"			      \
				: "=m" (*mem)				      \
				: "ir" (value), "0" (*mem));		      \
	    else if (sizeof (*mem) == 4)				      \
	      __asm __volatile (LOCK "addl %1, %0"			      \
				: "=m" (*mem)				      \
				: "ir" (value), "0" (*mem));		      \
	    else							      \
	      __asm __volatile (LOCK "addq %q1, %0"			      \
				: "=m" (*mem)				      \
				: "ir" (value), "0" (*mem));		      \
	    })


#define atomic_add_negative(mem, value) \
  ({ unsigned char __result;						      \
     if (sizeof (*mem) == 1)						      \
       __asm __volatile (LOCK "addb %b2, %0; sets %1"			      \
			 : "=m" (*mem), "=qm" (__result)		      \
			 : "ir" (value), "0" (*mem));			      \
     else if (sizeof (*mem) == 2)					      \
       __asm __volatile (LOCK "addw %w2, %0; sets %1"			      \
			 : "=m" (*mem), "=qm" (__result)		      \
			 : "ir" (value), "0" (*mem));			      \
     else if (sizeof (*mem) == 4)					      \
       __asm __volatile (LOCK "addl %2, %0; sets %1"			      \
			 : "=m" (*mem), "=qm" (__result)		      \
			 : "ir" (value), "0" (*mem));			      \
     else								      \
       __asm __volatile (LOCK "addq %q2, %0; sets %1"			      \
			 : "=m" (*mem), "=qm" (__result)		      \
			 : "ir" (value), "0" (*mem));			      \
     __result; })


#define atomic_add_zero(mem, value) \
  ({ unsigned char __result;						      \
     if (sizeof (*mem) == 1)						      \
       __asm __volatile (LOCK "addb %b2, %0; setz %1"			      \
			 : "=m" (*mem), "=qm" (__result)		      \
			 : "ir" (value), "0" (*mem));			      \
     else if (sizeof (*mem) == 2)					      \
       __asm __volatile (LOCK "addw %w2, %0; setz %1"			      \
			 : "=m" (*mem), "=qm" (__result)		      \
			 : "ir" (value), "0" (*mem));			      \
     else if (sizeof (*mem) == 4)					      \
       __asm __volatile (LOCK "addl %2, %0; setz %1"			      \
			 : "=m" (*mem), "=qm" (__result)		      \
			 : "ir" (value), "0" (*mem));			      \
     else								      \
       __asm __volatile (LOCK "addq %q2, %0; setz %1"			      \
			 : "=m" (*mem), "=qm" (__result)		      \
			 : "ir" (value), "0" (*mem));			      \
     __result; })


#define atomic_increment(mem) \
  (void) ({ if (sizeof (*mem) == 1)					      \
	      __asm __volatile (LOCK "incb %b0"				      \
				: "=m" (*mem)				      \
				: "0" (*mem));				      \
	    else if (sizeof (*mem) == 2)				      \
	      __asm __volatile (LOCK "incw %w0"				      \
				: "=m" (*mem)				      \
				: "0" (*mem));				      \
	    else if (sizeof (*mem) == 4)				      \
	      __asm __volatile (LOCK "incl %0"				      \
				: "=m" (*mem)				      \
				: "0" (*mem));				      \
	    else							      \
	      __asm __volatile (LOCK "incq %q0"				      \
				: "=m" (*mem)				      \
				: "0" (*mem));				      \
	    })


#define atomic_increment_and_test(mem) \
  ({ unsigned char __result;						      \
     if (sizeof (*mem) == 1)						      \
       __asm __volatile (LOCK "incb %b0; sete %1"			      \
			 : "=m" (*mem), "=qm" (__result)		      \
			 : "0" (*mem));					      \
     else if (sizeof (*mem) == 2)					      \
       __asm __volatile (LOCK "incw %w0; sete %1"			      \
			 : "=m" (*mem), "=qm" (__result)		      \
			 : "0" (*mem));					      \
     else if (sizeof (*mem) == 4)					      \
       __asm __volatile (LOCK "incl %0; sete %1"			      \
			 : "=m" (*mem), "=qm" (__result)		      \
			 : "0" (*mem));					      \
     else								      \
       __asm __volatile (LOCK "incq %q0; sete %1"			      \
			 : "=m" (*mem), "=qm" (__result)		      \
			 : "0" (*mem));					      \
     __result; })


#define atomic_decrement(mem) \
  (void) ({ if (sizeof (*mem) == 1)					      \
	      __asm __volatile (LOCK "decb %b0"				      \
				: "=m" (*mem)				      \
				: "0" (*mem));				      \
	    else if (sizeof (*mem) == 2)				      \
	      __asm __volatile (LOCK "decw %w0"				      \
				: "=m" (*mem)				      \
				: "0" (*mem));				      \
	    else if (sizeof (*mem) == 4)				      \
	      __asm __volatile (LOCK "decl %0"				      \
				: "=m" (*mem)				      \
				: "0" (*mem));				      \
	    else							      \
	      __asm __volatile (LOCK "decq %q0"				      \
				: "=m" (*mem)				      \
				: "0" (*mem));				      \
	    })


#define atomic_decrement_and_test(mem) \
  ({ unsigned char __result;						      \
     if (sizeof (*mem) == 1)						      \
       __asm __volatile (LOCK "decb %b0; sete %1"			      \
			 : "=m" (*mem), "=qm" (__result)		      \
			 : "0" (*mem));					      \
     else if (sizeof (*mem) == 2)					      \
       __asm __volatile (LOCK "decw %w0; sete %1"			      \
			 : "=m" (*mem), "=qm" (__result)		      \
			 : "0" (*mem));					      \
     else if (sizeof (*mem) == 4)					      \
       __asm __volatile (LOCK "decl %0; sete %1"			      \
			 : "=m" (*mem), "=qm" (__result)		      \
			 : "0" (*mem));					      \
     else								      \
       __asm __volatile (LOCK "decq %q0; sete %1"			      \
			 : "=m" (*mem), "=qm" (__result)		      \
			 : "0" (*mem));					      \
     __result; })


#define atomic_bit_set(mem, bit) \
  (void) ({ if (sizeof (*mem) == 1)					      \
	      __asm __volatile (LOCK "orb %b2, %0"			      \
				: "=m" (*mem)				      \
				: "0" (*mem), "i" (1 << (bit)));	      \
	    else if (sizeof (*mem) == 2)				      \
	      __asm __volatile (LOCK "orw %w2, %0"			      \
				: "=m" (*mem)				      \
				: "0" (*mem), "i" (1 << (bit)));	      \
	    else if (sizeof (*mem) == 4)				      \
	      __asm __volatile (LOCK "orl %2, %0"			      \
				: "=m" (*mem)				      \
				: "0" (*mem), "i" (1 << (bit)));	      \
	    else							      \
	      __asm __volatile (LOCK "orq %q2, %0"			      \
				: "=m" (*mem)				      \
				: "0" (*mem), "i" (1 << (bit)));	      \
	    })


#define atomic_bit_test_set(mem, bit) \
  ({ unsigned char __result;						      \
     if (sizeof (*mem) == 1)						      \
       __asm __volatile (LOCK "btsb %3, %1; setc %0"			      \
			 : "=q" (__result), "=m" (*mem)			      \
			 : "1" (*mem), "i" (bit));			      \
     else if (sizeof (*mem) == 2)					      \
       __asm __volatile (LOCK "btsw %3, %1; setc %0"			      \
			 : "=q" (__result), "=m" (*mem)			      \
			 : "1" (*mem), "i" (bit));			      \
     else if (sizeof (*mem) == 4)					      \
       __asm __volatile (LOCK "btsl %3, %1; setc %0"			      \
			 : "=q" (__result), "=m" (*mem)			      \
			 : "1" (*mem), "i" (bit));			      \
     else							      	      \
       __asm __volatile (LOCK "btsq %3, %1; setc %0"			      \
			 : "=q" (__result), "=m" (*mem)			      \
			 : "1" (*mem), "i" (bit));			      \
     __result; })
