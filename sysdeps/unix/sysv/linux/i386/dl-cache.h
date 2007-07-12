/* Support for reading /etc/ld.so.cache files written by Linux ldconfig.
   Copyright (C) 2004 Free Software Foundation, Inc.
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

static inline int
is_ia64 (void)
{
  unsigned int fl1, fl2;

  /* See if we can use cpuid.  */
  __asm__ ("pushfl; pushfl; popl %0; movl %0,%1; xorl %2,%0;"
           "pushl %0; popfl; pushfl; popl %0; popfl"
           : "=&r" (fl1), "=&r" (fl2)
           : "i" (0x00200000));
  if (((fl1 ^ fl2) & 0x00200000) == 0)
    return 0;

  /* Host supports cpuid.  See if cpuid gives capabilities, try
     CPUID(0).  Preserve %ebx and %ecx; cpuid insn clobbers these, we
     don't need their CPUID values here, and %ebx may be the PIC
     register.  */
  __asm__ ("pushl %%ecx; pushl %%ebx; cpuid; popl %%ebx; popl %%ecx"
           : "=a" (fl1) : "0" (0) : "edx", "cc");
  if (fl1 == 0)
    return 0;

  /* Invoke CPUID(1), return %edx; caller can examine bits to
     determine what's supported.  */
  __asm__ ("pushl %%ecx; pushl %%ebx; cpuid; popl %%ebx; popl %%ecx"
           : "=d" (fl2), "=a" (fl1) : "1" (1) : "cc");
  return (fl2 & (1 << 30)) != 0;
}

#define arch_startup(argc, argv) \
  do {									\
    /* On IA-64, try to execute 64-bit ldconfig if possible.		\
       This is because the badly designed /emul/ia32-linux hack		\
       will cause 32-bit ldconfig to do all sorts of weird things.  */	\
    if (is_ia64 ())							\
      execv ("/emul/ia32-linux/../../sbin/ldconfig",			\
	     (char *const *) argv);					\
  } while (0)

#include_next <dl-cache.h>
