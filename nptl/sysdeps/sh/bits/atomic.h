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
  ({ unsigned char __result; \
     __asm __volatile ("\
	.align 2\n\
	mova 1f,r0\n\
	nop\n\
	mov r15,r1\n\
	mov #-8,r15\n\
     0: mov.b @%1,r2\n\
	cmp/eq r2,%3\n\
	bf 1f\n\
	mov.b %2,@%1\n\
     1: mov r1,r15\n\
	mov #-1,%0\n\
	negc %0,%0"\
	: "=r" (__result) : "r" (mem), "r" (newval), "r" (oldval) \
	: "r0", "r1", "r2", "t", "memory"); \
     __result; })

#define __arch_compare_and_exchange_16_acq(mem, newval, oldval) \
  ({ unsigned char __result; \
     __asm __volatile ("\
	.align 2\n\
	mova 1f,r0\n\
	nop\n\
	mov r15,r1\n\
	mov #-8,r15\n\
     0: mov.w @%1,r2\n\
	cmp/eq r2,%3\n\
	bf 1f\n\
	mov.w %2,@%1\n\
     1: mov r1,r15\n\
	mov #-1,%0\n\
	negc %0,%0"\
	: "=r" (__result) : "r" (mem), "r" (newval), "r" (oldval) \
	: "r0", "r1", "r2", "t", "memory"); \
     __result; })

#define __arch_compare_and_exchange_32_acq(mem, newval, oldval) \
  ({ unsigned char __result; \
     __asm __volatile ("\
	.align 2\n\
	mova 1f,r0\n\
	nop\n\
	mov r15,r1\n\
	mov #-8,r15\n\
     0: mov.l @%1,r2\n\
	cmp/eq r2,%3\n\
	bf 1f\n\
	mov.l %2,@%1\n\
     1: mov r1,r15\n\
	mov #-1,%0\n\
	negc %0,%0"\
	: "=r" (__result) : "r" (mem), "r" (newval), "r" (oldval) \
	: "r0", "r1", "r2", "t", "memory"); \
     __result; })

/* XXX We do not really need 64-bit compare-and-exchange.  At least
   not in the moment.  Using it would mean causing portability
   problems since not many other 32-bit architectures have support for
   such an operation.  So don't define any code for now.  */

# define __arch_compare_and_exchange_64_acq(mem, newval, oldval) \
  (abort (), 0)

#define atomic_exchange_and_add(mem, value) \
  ({ __typeof (*mem) __result; \
     if (sizeof (*mem) == 1) \
       __asm __volatile ("\
	  .align 2\n\
	  mova 1f,r0\n\
	  mov r15,r1\n\
	  mov #-6,r15\n\
       0: mov.b @%2,%0\n\
	  add %0,%1\n\
	  mov.b %1,@%2\n\
       1: mov r1,r15"\
	: "=&r" (__result), "=&r" (value) : "r" (mem), "1" (value) \
	: "r0", "r1", "memory"); \
     else if (sizeof (*mem) == 2) \
       __asm __volatile ("\
	  .align 2\n\
	  mova 1f,r0\n\
	  mov r15,r1\n\
	  mov #-6,r15\n\
       0: mov.w @%2,%0\n\
	  add %0,%1\n\
	  mov.w %1,@%2\n\
       1: mov r1,r15"\
	: "=&r" (__result), "=&r" (value) : "r" (mem), "1" (value) \
	: "r0", "r1", "memory"); \
     else if (sizeof (*mem) == 4) \
       __asm __volatile ("\
	  .align 2\n\
	  mova 1f,r0\n\
	  mov r15,r1\n\
	  mov #-6,r15\n\
       0: mov.l @%2,%0\n\
	  add %0,%1\n\
	  mov.l %1,@%2\n\
       1: mov r1,r15"\
	: "=&r" (__result), "=&r" (value) : "r" (mem), "1" (value) \
	: "r0", "r1", "memory"); \
     else \
       { \
	 __typeof (value) addval = (value); \
	 __typeof (*mem) oldval; \
	 __typeof (mem) memp = (mem); \
	 do \
	   __result = (oldval = *memp) + addval; \
	 while (!__arch_compare_and_exchange_64_acq (memp, __result, oldval));\
	 (void) addval; \
       } \
     __result; })

#define atomic_add(mem, value) \
  (void) ({ if (sizeof (*mem) == 1) \
	      __asm __volatile ("\
		.align 2\n\
		mova 1f,r0\n\
		mov r15,r1\n\
		mov #-6,r15\n\
	     0: mov.b @%1,r2\n\
		add r2,%0\n\
		mov.b %0,@%1\n\
	     1: mov r1,r15"\
		: "=&r" (value) : "r" (mem), "0" (value) \
		: "r0", "r1", "r2", "memory"); \
	    else if (sizeof (*mem) == 2) \
	      __asm __volatile ("\
		.align 2\n\
		mova 1f,r0\n\
		mov r15,r1\n\
		mov #-6,r15\n\
	     0: mov.w @%1,r2\n\
		add r2,%0\n\
		mov.w %0,@%1\n\
	     1: mov r1,r15"\
		: "=&r" (value) : "r" (mem), "0" (value) \
		: "r0", "r1", "r2", "memory"); \
	    else if (sizeof (*mem) == 4) \
	      __asm __volatile ("\
		.align 2\n\
		mova 1f,r0\n\
		mov r15,r1\n\
		mov #-6,r15\n\
	     0: mov.l @%1,r2\n\
		add r2,%0\n\
		mov.l %0,@%1\n\
	     1: mov r1,r15"\
		: "=&r" (value) : "r" (mem), "0" (value) \
		: "r0", "r1", "r2", "memory"); \
	    else \
	      { \
		__typeof (value) addval = (value); \
		__typeof (*mem) oldval; \
		__typeof (mem) memp = (mem); \
		do \
		  oldval = *memp; \
		while (! __arch_compare_and_exchange_64_acq (memp, \
							     oldval + addval, \
							     oldval)); \
		(void) addval; \
	      } \
	    })

#define atomic_add_negative(mem, value) \
  ({ unsigned char __result; \
     if (sizeof (*mem) == 1) \
       __asm __volatile ("\
	  .align 2\n\
	  mova 1f,r0\n\
	  mov r15,r1\n\
	  mov #-6,r15\n\
       0: mov.b @%2,r2\n\
	  add r2,%1\n\
	  mov.b %1,@%2\n\
       1: mov r1,r15\n\
	  shal %1\n\
	  movt %0"\
	: "=r" (__result), "=&r" (value) : "r" (mem), "1" (value) \
	: "r0", "r1", "r2", "t", "memory"); \
     else if (sizeof (*mem) == 2) \
       __asm __volatile ("\
	  .align 2\n\
	  mova 1f,r0\n\
	  mov r15,r1\n\
	  mov #-6,r15\n\
       0: mov.w @%2,r2\n\
	  add r2,%1\n\
	  mov.w %1,@%2\n\
       1: mov r1,r15\n\
	  shal %1\n\
	  movt %0"\
	: "=r" (__result), "=&r" (value) : "r" (mem), "1" (value) \
	: "r0", "r1", "r2", "t", "memory"); \
     else if (sizeof (*mem) == 4) \
       __asm __volatile ("\
	  .align 2\n\
	  mova 1f,r0\n\
	  mov r15,r1\n\
	  mov #-6,r15\n\
       0: mov.l @%2,r2\n\
	  add r2,%1\n\
	  mov.l %1,@%2\n\
       1: mov r1,r15\n\
	  shal %1\n\
	  movt %0"\
	: "=r" (__result), "=&r" (value) : "r" (mem), "1" (value) \
	: "r0", "r1", "r2", "t", "memory"); \
     else \
       abort (); \
     __result; })

#define atomic_add_zero(mem, value) \
  ({ unsigned char __result; \
     if (sizeof (*mem) == 1) \
       __asm __volatile ("\
	  .align 2\n\
	  mova 1f,r0\n\
	  mov r15,r1\n\
	  mov #-6,r15\n\
       0: mov.b @%2,r2\n\
	  add r2,%1\n\
	  mov.b %1,@%2\n\
       1: mov r1,r15\n\
	  tst %1,%1\n\
	  movt %0"\
	: "=r" (__result), "=&r" (value) : "r" (mem), "1" (value) \
	: "r0", "r1", "r2", "t", "memory"); \
     else if (sizeof (*mem) == 2) \
       __asm __volatile ("\
	  .align 2\n\
	  mova 1f,r0\n\
	  mov r15,r1\n\
	  mov #-6,r15\n\
       0: mov.w @%2,r2\n\
	  add r2,%1\n\
	  mov.w %1,@%2\n\
       1: mov r1,r15\n\
	  tst %1,%1\n\
	  movt %0"\
	: "=r" (__result), "=&r" (value) : "r" (mem), "1" (value) \
	: "r0", "r1", "r2", "t", "memory"); \
     else if (sizeof (*mem) == 4) \
       __asm __volatile ("\
	  .align 2\n\
	  mova 1f,r0\n\
	  mov r15,r1\n\
	  mov #-6,r15\n\
       0: mov.l @%2,r2\n\
	  add r2,%1\n\
	  mov.l %1,@%2\n\
       1: mov r1,r15\n\
	  tst %1,%1\n\
	  movt %0"\
	: "=r" (__result), "=&r" (value) : "r" (mem), "1" (value) \
	: "r0", "r1", "r2", "t", "memory"); \
     else \
       abort (); \
     __result; })

#define atomic_increment_and_test(mem) atomic_add_zero((mem), 1)
#define atomic_decrement_and_test(mem) atomic_add_zero((mem), -1)

#define atomic_bit_set(mem, bit) \
  (void) ({ unsigned int __mask = 1 << (bit); \
	    if (sizeof (*mem) == 1) \
	      __asm __volatile ("\
		.align 2\n\
		mova 1f,r0\n\
		mov r15,r1\n\
		mov #-6,r15\n\
	     0: mov.b @%0,r2\n\
		or %1,r2\n\
		mov.b r2,@%0\n\
	     1: mov r1,r15"\
		: : "r" (mem), "r" (__mask) \
		: "r0", "r1", "r2", "memory"); \
	    else if (sizeof (*mem) == 2) \
	      __asm __volatile ("\
		.align 2\n\
		mova 1f,r0\n\
		mov r15,r1\n\
		mov #-6,r15\n\
	     0: mov.w @%0,r2\n\
		or %1,r2\n\
		mov.w r2,@%0\n\
	     1: mov r1,r15"\
		: : "r" (mem), "r" (__mask) \
		: "r0", "r1", "r2", "memory"); \
	    else if (sizeof (*mem) == 4) \
	      __asm __volatile ("\
		.align 2\n\
		mova 1f,r0\n\
		mov r15,r1\n\
		mov #-6,r15\n\
	     0: mov.l @%0,r2\n\
		or %1,r2\n\
		mov.l r2,@%0\n\
	     1: mov r1,r15"\
		: : "r" (mem), "r" (__mask) \
		: "r0", "r1", "r2", "memory"); \
	    else \
	      abort (); \
	    })

#define atomic_bit_test_set(mem, bit) \
  ({ unsigned int __mask = 1 << (bit); \
     unsigned int __result = __mask; \
     if (sizeof (*mem) == 1) \
       __asm __volatile ("\
	  .align 2\n\
	  mova 1f,r0\n\
	  nop\n\
	  mov r15,r1\n\
	  mov #-8,r15\n\
       0: mov.b @%2,r2\n\
	  or r2,%1\n\
	  and r2,%0\n\
	  mov.b %1,@%2\n\
       1: mov r1,r15"\
	: "=&r" (__result), "=&r" (__mask) \
	: "r" (mem), "0" (__result), "1" (__mask) \
	: "r0", "r1", "r2", "memory"); \
     else if (sizeof (*mem) == 2) \
       __asm __volatile ("\
	  .align 2\n\
	  mova 1f,r0\n\
	  nop\n\
	  mov r15,r1\n\
	  mov #-8,r15\n\
       0: mov.w @%2,r2\n\
	  or r2,%1\n\
	  and r2,%0\n\
	  mov.w %1,@%2\n\
       1: mov r1,r15"\
	: "=&r" (__result), "=&r" (__mask) \
	: "r" (mem), "0" (__result), "1" (__mask) \
	: "r0", "r1", "r2", "memory"); \
     else if (sizeof (*mem) == 4) \
       __asm __volatile ("\
	  .align 2\n\
	  mova 1f,r0\n\
	  nop\n\
	  mov r15,r1\n\
	  mov #-8,r15\n\
       0: mov.l @%2,r2\n\
	  or r2,%1\n\
	  and r2,%0\n\
	  mov.l %1,@%2\n\
       1: mov r1,r15"\
	: "=&r" (__result), "=&r" (__mask) \
	: "r" (mem), "0" (__result), "1" (__mask) \
	: "r0", "r1", "r2", "memory"); \
     else \
       abort (); \
     __result; })
