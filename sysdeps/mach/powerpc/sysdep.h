/* system call details for Mach on PowerPC
   Copyright (C) 2001,02 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, see <http://www.gnu.org/licenses/>.  */

#ifndef _MACH_POWERPC_SYSDEP_H
#define _MACH_POWERPC_SYSDEP_H

#define START_ARGS char **sparg
#define SNARF_ARGS(argc, argv, envp) \
  do {                               \
    argv = &sparg[1];                \
    argc = *(int *)sparg;            \
    envp = &argv[argc + 1];          \
  } while (0)

#define CALL_WITH_SP(fn, sp) \
  do {                                                 \
    register long __sp = (long) sp, __fn = (long) fn; \
    asm volatile ("mr 1, %0; mtlr %1; blr"            \
		  : : "r" (__sp), "r" (__fn));        \
  } while (0)

#define STACK_GROWTH_DOWN

#define RETURN_TO(sp, pc, retval) \
     asm volatile ("mr 1, %0; mtctr %1; mr 3, %2; bctr" \
		   : : "r" (sp), "r" (pc), "r" (retval))

/* Get the machine-independent Mach definitions.  */
#define _MACH_MACHINE_ASM_H 1	/* Kludge to avoid bad Darwin header file.  */
#include <sysdeps/mach/sysdep.h>

#undef ENTRY
#include <sysdeps/unix/powerpc/sysdep.h>

#endif  /* _MACH_POWERPC_SYSDEP_H */
