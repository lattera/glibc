/* Resolve function pointers to VDSO functions.
   Copyright (C) 2010 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Maxim Kuvyrkov <maxim@codesourcery.com>, 2010.

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


#ifndef _M68K_VDSO_H
#define _M68K_VDSO_H

#ifdef SHARED

# ifdef IS_IN_rtld
# define M68K_VDSO_SYMBOL(name) __rtld_##name
# define STR_M68K_VDSO_SYMBOL(name) "__rtld_" #name
# else
# define M68K_VDSO_SYMBOL(name) name
# define STR_M68K_VDSO_SYMBOL(name) #name
# endif

# ifndef __ASSEMBLER__

/* We define __rtld_* copies for rtld.
   We need them visible in libc to initialize.  */
#  if defined IS_IN_rtld || !defined NOT_IN_libc
extern void *__rtld___vdso_read_tp;
extern void *__rtld___vdso_atomic_cmpxchg_32;
extern void *__rtld___vdso_atomic_barrier;

/* These stubs are meant to be invoked only from the assembly.  */
extern void __vdso_read_tp_stub (void);
extern void __vdso_atomic_cmpxchg_32_stub (void);
extern void __vdso_atomic_barrier_stub (void);
#  endif /* IS_IN_rtld || !NOT_IN_libc */

/* RTLD should only use its own copies.  */
#  ifndef IS_IN_rtld
extern void *__vdso_read_tp;
extern void *__vdso_atomic_cmpxchg_32;
extern void *__vdso_atomic_barrier;
#  endif /* !IS_IN_rtld */

# endif /* !__ASSEMBLER__ */

#endif /* SHARED */

#endif /* _M68K_VDSO_H */
