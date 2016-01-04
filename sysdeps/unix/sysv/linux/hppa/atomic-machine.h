/* Copyright (C) 2003-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Carlos O'Donell <carlos@baldric.uwo.ca>, 2005.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <stdint.h> /*  Required for type definitions e.g. uint8_t.  */
#include <abort-instr.h> /*  Required for ABORT_INSTRUCTIUON.  */

/* We need EFAULT, ENONSYS */
#if !defined EFAULT && !defined ENOSYS
#define EFAULT	14
#define ENOSYS	251
#endif

#ifndef _ATOMIC_MACHINE_H
#define _ATOMIC_MACHINE_H	1

typedef int8_t atomic8_t;
typedef uint8_t uatomic8_t;
typedef int_fast8_t atomic_fast8_t;
typedef uint_fast8_t uatomic_fast8_t;

typedef int32_t atomic32_t;
typedef uint32_t uatomic32_t;
typedef int_fast32_t atomic_fast32_t;
typedef uint_fast32_t uatomic_fast32_t;

typedef intptr_t atomicptr_t;
typedef uintptr_t uatomicptr_t;
typedef intmax_t atomic_max_t;
typedef uintmax_t uatomic_max_t;

#define __HAVE_64B_ATOMICS 0
#define USE_ATOMIC_COMPILER_BUILTINS 0

/* prev = *addr;
   if (prev == old)
     *addr = new;
   return prev; */

/* Use the kernel atomic light weight syscalls on hppa.  */
#define _LWS "0xb0"
#define _LWS_CAS "0"
/* Note r31 is the link register.  */
#define _LWS_CLOBBER "r1", "r23", "r22", "r20", "r31", "memory"
/* String constant for -EAGAIN.  */
#define _ASM_EAGAIN "-11"
/* String constant for -EDEADLOCK.  */
#define _ASM_EDEADLOCK "-45"

/* The only basic operation needed is compare and exchange.  The mem
   pointer must be word aligned.  */
#define atomic_compare_and_exchange_val_acq(mem, newval, oldval)	\
  ({									\
     register long lws_errno asm("r21");				\
     register unsigned long lws_ret asm("r28");				\
     register unsigned long lws_mem asm("r26") = (unsigned long)(mem);	\
     register unsigned long lws_old asm("r25") = (unsigned long)(oldval);\
     register unsigned long lws_new asm("r24") = (unsigned long)(newval);\
     __asm__ __volatile__(						\
	"0:					\n\t"			\
	"ble	" _LWS "(%%sr2, %%r0)		\n\t"			\
	"ldi	" _LWS_CAS ", %%r20		\n\t"			\
	"ldi	" _ASM_EAGAIN ", %%r20		\n\t"			\
	"cmpb,=,n %%r20, %%r21, 0b		\n\t"			\
	"nop					\n\t"			\
	"ldi	" _ASM_EDEADLOCK ", %%r20	\n\t"			\
	"cmpb,=,n %%r20, %%r21, 0b		\n\t"			\
	"nop					\n\t"			\
	: "=r" (lws_ret), "=r" (lws_errno)				\
	: "r" (lws_mem), "r" (lws_old), "r" (lws_new)			\
	: _LWS_CLOBBER							\
     );									\
									\
     if (lws_errno == -EFAULT || lws_errno == -ENOSYS)			\
	ABORT_INSTRUCTION;						\
									\
     (__typeof (oldval)) lws_ret;					\
   })

#define atomic_compare_and_exchange_bool_acq(mem, newval, oldval)	\
  ({									\
     __typeof__ (*mem) ret;						\
     ret = atomic_compare_and_exchange_val_acq(mem, newval, oldval);	\
     /* Return 1 if it was already acquired.  */			\
     (ret != oldval);							\
   })

#endif
/* _ATOMIC_MACHINE_H */
