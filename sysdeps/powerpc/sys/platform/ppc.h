/* Facilities specific to the PowerPC architecture
   Copyright (C) 2012 Free Software Foundation, Inc.
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

#ifndef _SYS_PLATFORM_PPC_H
#define _SYS_PLATFORM_PPC_H	1

#include <features.h>
#include <stdint.h>
#include <bits/ppc.h>

/* Read the Time Base Register.   */
static inline uint64_t
__ppc_get_timebase (void)
{
#if __GNUC_PREREQ (4, 8)
  return __builtin_ppc_get_timebase ();
#else
# ifdef __powerpc64__
  uint64_t __tb;
  /* "volatile" is necessary here, because the user expects this assembly
     isn't moved after an optimization.  */
  __asm__ volatile ("mfspr %0, 268" : "=r" (__tb));
  return __tb;
# else  /* not __powerpc64__ */
  uint32_t __tbu, __tbl, __tmp; \
  __asm__ volatile ("0:\n\t"
		    "mftbu %0\n\t"
		    "mftbl %1\n\t"
		    "mftbu %2\n\t"
		    "cmpw %0, %2\n\t"
		    "bne- 0b"
		    : "=r" (__tbu), "=r" (__tbl), "=r" (__tmp));
  return (((uint64_t) __tbu << 32) | __tbl);
# endif  /* not __powerpc64__ */
#endif
}

#endif  /* sys/platform/ppc.h */
