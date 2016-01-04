/* Definition for thread-local data handling.  nptl/x32 version.
   Copyright (C) 2012-2016 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _X32_TLS_H
#define _X32_TLS_H	1

#include_next <tls.h>

#ifndef __ASSEMBLER__

/* X32 doesn't support 32-bit indirect calls via memory.  Instead, we
   load the 32-bit address from memory into the lower 32 bits of the
   return-value register, which will automatically zero-extend the upper
   32 bits of the return-value register.  We then do the indirect call
   via the 64-bit return-value register.  */
# undef CALL_THREAD_FCT
# define CALL_THREAD_FCT(descr) \
  ({ void *__res;							      \
     asm volatile ("movl %%fs:%P2, %%edi\n\t"				      \
		   "movl %%fs:%P1, %k0\n\t"				      \
		   "callq *%q0"						      \
		   : "=a" (__res)					      \
		   : "i" (offsetof (struct pthread, start_routine)),	      \
		     "i" (offsetof (struct pthread, arg))		      \
		   : "di", "si", "cx", "dx", "r8", "r9", "r10", "r11",	      \
		     "memory", "cc");					      \
     __res; })

#endif /* __ASSEMBLER__ */

#endif	/* x32/tls.h */
