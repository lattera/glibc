/* Machine-specific function to return the stack pointer.  PowerPC version.
   Copyright (C) 2001, 2007 Free Software Foundation, Inc.
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

#ifndef _MACHINE_SP_H
#define _MACHINE_SP_H

/* Return the current stack pointer.  */

#ifndef _EXTERN_INLINE
/* Make sure this function is included in hurd/threadvar-inlines.c.  */
# ifdef _HURD_THREADVAR_H_EXTERN_INLINE
#  define _EXTERN_INLINE _HURD_THREADVAR_H_EXTERN_INLINE
# else
#  define _EXTERN_INLINE __extern_inline
# endif
#endif

_EXTERN_INLINE void *
__thread_stack_pointer (void)
{
  register void *__sp__;
  __asm__ ("mr %0, 1" : "=r" (__sp__));
  return __sp__;
}

#endif	/* machine-sp.h */
