/* Copyright (C) 2002, 2003, 2004, 2005 Free Software Foundation, Inc.
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

#define atomic_exchange_acq(mem, newvalue)				      \
  ({ __typeof (*mem) result;						      \
     if (sizeof (*mem) == 1)						      \
       __asm__ __volatile__ ("swpb %0, %1, [%2]"			      \
			     : "=&r,&r" (result)			      \
			     : "r,0" (newvalue), "r,r" (mem) : "memory");     \
     else if (sizeof (*mem) == 4)					      \
       __asm__ __volatile__ ("swp %0, %1, [%2]"				      \
			     : "=&r,&r" (result)			      \
			     : "r,0" (newvalue), "r,r" (mem) : "memory");     \
     else								      \
       {								      \
	 result = 0;							      \
	 abort ();							      \
       }								      \
     result; })

/* Atomic compare and exchange.  This sequence relies on the kernel to
   provide a compare and exchange operation which is atomic on the
   current architecture, either via cleverness on pre-ARMv6 or via
   ldrex / strex on ARMv6.  */

#define __arch_compare_and_exchange_val_8_acq(mem, newval, oldval) \
  ({ __arm_link_error (); oldval; })

#define __arch_compare_and_exchange_val_16_acq(mem, newval, oldval) \
  ({ __arm_link_error (); oldval; })

/* It doesn't matter what register is used for a_oldval2, but we must
   specify one to work around GCC PR rtl-optimization/21223.  Otherwise
   it may cause a_oldval or a_tmp to be moved to a different register.  */

#define __arch_compare_and_exchange_val_32_acq(mem, newval, oldval) \
  ({ register __typeof (oldval) a_oldval asm ("r0");			      \
     register __typeof (oldval) a_newval asm ("r1") = (newval);		      \
     register __typeof (mem) a_ptr asm ("r2") = (mem);			      \
     register __typeof (oldval) a_tmp asm ("r3");			      \
     register __typeof (oldval) a_oldval2 asm ("r4") = (oldval);	      \
     __asm__ __volatile__						      \
	     ("0:\tldr\t%1,[%3]\n\t"					      \
	      "cmp\t%1, %4\n\t"						      \
	      "bne\t1f\n\t"						      \
	      "mov\t%0, %4\n\t"						      \
	      "mov\t%1, #0xffff0fff\n\t"				      \
	      "mov\tlr, pc\n\t"						      \
	      "add\tpc, %1, #(0xffff0fc0 - 0xffff0fff)\n\t"		      \
	      "bcc\t0b\n\t"						      \
	      "mov\t%1, %4\n\t"						      \
	      "1:"							      \
	      : "=&r" (a_oldval), "=&r" (a_tmp)				      \
	      : "r" (a_newval), "r" (a_ptr), "r" (a_oldval2)		      \
	      : "ip", "lr", "cc", "memory");				      \
     a_tmp; })

#define __arch_compare_and_exchange_val_64_acq(mem, newval, oldval) \
  ({ __arm_link_error (); oldval; })
