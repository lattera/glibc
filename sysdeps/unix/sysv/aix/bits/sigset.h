/* __sig_atomic_t, __sigset_t, and related definitions.  AIX version.
   Copyright (C) 1991,1992,1994,1996,1997,2000 Free Software Foundation, Inc.
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
# define _SIGSET_H_types	1

typedef int __sig_atomic_t;

/* A `sigset_t' has a bit for each signal.  */

typedef struct
  {
    unsigned int __losigs;
    unsigned int __hisigs;
  } __sigset_t;

#endif


/* We only want to define these functions if <signal.h> was actually
   included; otherwise we were included just to define the types.  Since we
   are namespace-clean, it wouldn't hurt to define extra macros.  But
   trouble can be caused by functions being defined (e.g., any global
   register vars declared later will cause compilation errors).  */

#if !defined _SIGSET_H_fns && defined _SIGNAL_H
# define _SIGSET_H_fns 1

# ifndef _EXTERN_INLINE
#  define _EXTERN_INLINE extern __inline
# endif

/* Return a mask that includes the bit for SIG only.  */
# define __sigmask(sig) \
  (((unsigned long int) 1) << (((sig) - 1) % (8 * sizeof (unsigned int))))

# if defined __GNUC__ && __GNUC__ >= 2
#  define __sigemptyset(set) \
  (__extension__ ({ sigset_t *__set = (set);				      \
		    __set->__losigs = __set->__hisigs = 0;		      \
		    0; }))
#  define __sigfillset(set) \
  (__extension__ ({ sigset_t *__set = (set);				      \
		    __set->__losigs = __set->__hisigs = ~0u;		      \
		    0; }))

#  ifdef __USE_GNU
/* The POSIX does not specify for handling the whole signal set in one
   command.  This is often wanted and so we define three more functions
   here.  */
#   define __sigisemptyset(set) \
  (__extension__ ({ const sigset_t *__set = (set);			      \
		    (__set->__losigs | __set->__hisigs) == 0; }))
#   define __sigandset(dest, left, right) \
  (__extension__ ({ sigset_t *__dest = (dest);				      \
		    const sigset_t *__left = (left);			      \
		    const sigset_t *__right = (right);			      \
		    __dest->__losigs = __left->__losigs & __right->__losigs;  \
		    __dest->__hisigs = __left->__hisigs & __right->__hisigs;  \
		    0; }))
#   define __sigorset(dest, left, right) \
  (__extension__ ({ sigset_t *__dest = (dest);				      \
		    const sigset_t *__left = (left);			      \
		    const sigset_t *__right = (right);			      \
		    __dest->__losigs = __left->__losigs | __right->__losigs;  \
		    __dest->__hisigs = __left->__hisigs | __right->__hisigs;  \
		    0; }))
#  endif
# endif

/* These functions needn't check for a bogus signal number -- error
   checking is done in the non __ versions.  */

extern int __sigismember (__const __sigset_t *, int);
extern int __sigaddset (__sigset_t *, int);
extern int __sigdelset (__sigset_t *, int);

# ifdef __USE_EXTERN_INLINES
_EXTERN_INLINE int
__sigismember (__const __sigset_t *__set, int __sig)
{
  unsigned int __mask = __sigmask (__sig);

  return ((__sig < 33 ? __set->__losigs : __set->__hisigs) & __mask ) ? 1 : 0;
}

_EXTERN_INLINE int
__sigaddset (__sigset_t *__set, int __sig)
{
  unsigned int __mask = __sigmask (__sig);

  (__sig < 33 ? __set->__losigs : __set->__hisigs) |= __mask;

  return 0;
}

_EXTERN_INLINE int
__sigdelset (__sigset_t *__set, int __sig)
{
  unsigned int __mask = __sigmask (__sig);

  (__sig < 33 ? __set->__losigs : __set->__hisigs) &= ~__mask;

  return 0;
}
# endif


#endif /* ! _SIGSET_H_fns.  */
