/* __sig_atomic_t, __sigset_t, and related definitions.  Generic/BSD version.
Copyright (C) 1991, 1992, 1994 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the, 1992 Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#ifndef	_SIGSET_H_types
#define	_SIGSET_H_types	1

typedef int __sig_atomic_t;

/* A `sigset_t' has a bit for each signal.  */
typedef unsigned long int __sigset_t;

#endif


/* We only want to define these functions if <signal.h> was actually
   included; otherwise we were included just to define the types.  Since we
   are namespace-clean, it wouldn't hurt to define extra macros.  But
   trouble can be caused by functions being defined (e.g., any global
   register vars declared later will cause compilation errors).  */

#if !defined (_SIGSET_H_fns) && defined (_SIGNAL_H)
#define _SIGSET_H_fns 1

/* Return a mask that includes SIG only.  The cast to `sigset_t' avoids
   overflow if `sigset_t' is wider than `int'.  */
#define	__sigmask(sig)	(((__sigset_t) 1) << ((sig) - 1))

#define	__sigemptyset(set)	((*(set) = (__sigset_t) 0), 0)
#define	__sigfillset(set)	((*(set) = ~(__sigset_t) 0), 0)

/* These functions must check for a bogus signal number.  We detect it by a
   zero sigmask, since a number too low or too high will have shifted the 1
   off the high end of the mask.  If we find an error, we punt to a random
   call we know fails with EINVAL (kludge city!), so as to avoid referring
   to `errno' in this file (sigh).  */

#ifndef _EXTERN_INLINE
#define _EXTERN_INLINE extern __inline
#endif
#define __SIGSETFN(NAME, BODY, CONST)					      \
  _EXTERN_INLINE int							      \
  __##NAME (CONST __sigset_t *__set, int __sig)				      \
  {									      \
    if (__sig < 1 || __sig > sizeof (__sigset_t) * 8)			      \
      {									      \
	extern int raise (int);						      \
	return raise (-1);						      \
      }									      \
    else								      \
      {									      \
	__sigset_t __mask = __sigmask (__sig);				      \
	return BODY;							      \
      }									      \
  }

__SIGSETFN (sigismember, (*__set & __mask) ? 1 : 0, __const)
__SIGSETFN (sigaddset, ((*__set |= __mask), 0), )
__SIGSETFN (sigdelset, ((*__set &= ~__mask), 0), )

#undef __SIGSETFN


#endif /* ! _SIGSET_H_fns.  */
