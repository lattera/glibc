/* __sig_atomic_t, __sigset_t, and related definitions.  SVR4 version.
   Copyright (C) 1994, 1995, 1996 Free Software Foundation, Inc.
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

#ifndef	_SIGSET_H_types
#define	_SIGSET_H_types	1

typedef int __sig_atomic_t;

/* A `sigset_t' has a bit for each signal.  */
typedef struct
  {
    unsigned long int __sigbits[4];
  } __sigset_t;

#endif	/* ! _SIGSET_H_types */

/* We only want to define these functions if <signal.h> was actually
   included; otherwise we were included just to define the types.  Since we
   are namespace-clean, it wouldn't hurt to define extra macros.  But
   trouble can be caused by functions being defined (e.g., any global
   register vars declared later will cause compilation errors).  */

#if !defined (_SIGSET_H_fns) && defined (_SIGNAL_H)
#define _SIGSET_H_fns 1

/* Return a mask that includes SIG only.  */
#define	__sigmask(sig)	(1 << ((sig) - 1))


/* It's easier to assume 8-bit bytes than to get CHAR_BIT.  */
#define	__NSSBITS	(sizeof (__sigset_t) * 8)
#define	__SSELT(s)	((s) / __NSSBITS)
#define	__SSMASK(s)	(1 << ((s) % __NSSBITS))

#ifdef __USE_EXTERN_INLINES
# ifndef _EXTERN_INLINE
#  define _EXTERN_INLINE	extern __inline
# endif

_EXTERN_INLINE int
__sigemptyset (__sigset_t *__set)
{
  __set->__sigbits[0] = __set->__sigbits[1] =
    __set->__sigbits[2] = __set->__sigbits[3] = 0L;
  return 0;
}

_EXTERN_INLINE int
__sigfillset (__sigset_t *__set)
{
  /* SVR4 has a system call for `sigfillset' (!), and it only sets the bits
     for signals [1,31].  Setting bits for unimplemented signals seems
     harmless (and we will find out if it really is).  */
  __set->__sigbits[0] = __set->__sigbits[1] =
    __set->__sigbits[2] = __set->__sigbits[3] = ~0L;
  return 0;
}

_EXTERN_INLINE int
__sigaddset (__sigset_t *__set, int __sig)
{
  __set->__sigbits[__SSELT (__sig)] |= __SSMASK (__sig);
  return 0;
}

_EXTERN_INLINE int
__sigdelset (__sigset_t *__set, int __sig)
{
  __set->__sigbits[__SSELT (__sig)] &= ~__SSMASK (__sig);
  return 0;
}

_EXTERN_INLINE int
__sigismember (__const __sigset_t *__set, int __sig)
{
  if (__set->__sigbits[__SSELT (__sig)] & __SSMASK (__sig))
    return 1;
  return 0;
}
#endif	/* use extern inlines.  */

#endif /* ! _SIGSET_H_fns */
