/* Copyright (C) 2002-2012 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <stdint.h>
#include <sysdep.h>


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

void __arm_link_error (void);

/* Use the atomic builtins provided by GCC in case the backend provides
   a pattern to do this efficiently.  */

#ifdef __GCC_HAVE_SYNC_COMPARE_AND_SWAP_4
#define atomic_full_barrier() __sync_synchronize ()
#elif defined __thumb2__
#define atomic_full_barrier() \
     __asm__ __volatile__						      \
	     ("movw\tip, #0x0fa0\n\t"					      \
	      "movt\tip, #0xffff\n\t"					      \
	      "blx\tip"							      \
	      : : : "ip", "lr", "cc", "memory");
#else
#define atomic_full_barrier() \
     __asm__ __volatile__						      \
	     ("mov\tip, #0xffff0fff\n\t"				      \
	      "mov\tlr, pc\n\t"						      \
	      "add\tpc, ip, #(0xffff0fa0 - 0xffff0fff)"			      \
	      : : : "ip", "lr", "cc", "memory");
#endif

/* Atomic compare and exchange.  This sequence relies on the kernel to
   provide a compare and exchange operation which is atomic on the
   current architecture, either via cleverness on pre-ARMv6 or via
   ldrex / strex on ARMv6.  */

#define __arch_compare_and_exchange_val_8_acq(mem, newval, oldval) \
  ({ __arm_link_error (); oldval; })

#define __arch_compare_and_exchange_val_16_acq(mem, newval, oldval) \
  ({ __arm_link_error (); oldval; })

#ifdef __GCC_HAVE_SYNC_COMPARE_AND_SWAP_4
#define __arch_compare_and_exchange_val_32_acq(mem, newval, oldval) \
  __sync_val_compare_and_swap ((mem), (oldval), (newval))

/* It doesn't matter what register is used for a_oldval2, but we must
   specify one to work around GCC PR rtl-optimization/21223.  Otherwise
   it may cause a_oldval or a_tmp to be moved to a different register.

   We use the union trick rather than simply using __typeof (...) in the
   declarations of A_OLDVAL et al because when NEWVAL or OLDVAL is of the
   form *PTR and PTR has a 'volatile ... *' type, then __typeof (*PTR) has
   a 'volatile ...' type and this triggers -Wvolatile-register-var to
   complain about 'register volatile ... asm ("reg")'.  */
#elif defined __thumb2__
/* Thumb-2 has ldrex/strex.  However it does not have barrier instructions,
   so we still need to use the kernel helper.  */
#define __arch_compare_and_exchange_val_32_acq(mem, newval, oldval) \
  ({ union { __typeof (oldval) a; uint32_t v; } oldval_arg = { .a = (oldval) };\
     union { __typeof (newval) a; uint32_t v; } newval_arg = { .a = (newval) };\
     register uint32_t a_oldval asm ("r0");				      \
     register uint32_t a_newval asm ("r1") = newval_arg.v;		      \
     register __typeof (mem) a_ptr asm ("r2") = (mem);			      \
     register uint32_t a_tmp asm ("r3");				      \
     register uint32_t a_oldval2 asm ("r4") = oldval_arg.v;		      \
     __asm__ __volatile__						      \
	     ("0:\tldr\t%[tmp],[%[ptr]]\n\t"				      \
	      "cmp\t%[tmp], %[old2]\n\t"				      \
	      "bne\t1f\n\t"						      \
	      "mov\t%[old], %[old2]\n\t"				      \
	      "movw\t%[tmp], #0x0fc0\n\t"				      \
	      "movt\t%[tmp], #0xffff\n\t"				      \
	      "blx\t%[tmp]\n\t"						      \
	      "bcc\t0b\n\t"						      \
	      "mov\t%[tmp], %[old2]\n\t"				      \
	      "1:"							      \
	      : [old] "=&r" (a_oldval), [tmp] "=&r" (a_tmp)		      \
	      : [new] "r" (a_newval), [ptr] "r" (a_ptr),		      \
		[old2] "r" (a_oldval2)					      \
	      : "ip", "lr", "cc", "memory");				      \
     (__typeof (oldval)) a_tmp; })
#else
#define __arch_compare_and_exchange_val_32_acq(mem, newval, oldval)	      \
  ({ union { __typeof (oldval) a; uint32_t v; } oldval_arg = { .a = (oldval) };\
     union { __typeof (newval) a; uint32_t v; } newval_arg = { .a = (newval) };\
     register uint32_t a_oldval asm ("r0");				      \
     register uint32_t a_newval asm ("r1") = newval_arg.v;		      \
     register __typeof (mem) a_ptr asm ("r2") = (mem);			      \
     register uint32_t a_tmp asm ("r3");				      \
     register uint32_t a_oldval2 asm ("r4") = oldval_arg.v;		      \
     __asm__ __volatile__						      \
	     ("0:\tldr\t%[tmp],[%[ptr]]\n\t"				      \
	      "cmp\t%[tmp], %[old2]\n\t"				      \
	      "bne\t1f\n\t"						      \
	      "mov\t%[old], %[old2]\n\t"				      \
	      "mov\t%[tmp], #0xffff0fff\n\t"				      \
	      "mov\tlr, pc\n\t"						      \
	      "add\tpc, %[tmp], #(0xffff0fc0 - 0xffff0fff)\n\t"		      \
	      "bcc\t0b\n\t"						      \
	      "mov\t%[tmp], %[old2]\n\t"				      \
	      "1:"							      \
	      : [old] "=&r" (a_oldval), [tmp] "=&r" (a_tmp)		      \
	      : [new] "r" (a_newval), [ptr] "r" (a_ptr),		      \
		[old2] "r" (a_oldval2)					      \
	      : "ip", "lr", "cc", "memory");				      \
     (__typeof (oldval)) a_tmp; })
#endif

#define __arch_compare_and_exchange_val_64_acq(mem, newval, oldval) \
  ({ __arm_link_error (); oldval; })
