/* Copyright (C) 1991, 92, 93, 94, 95, 96 Free Software Foundation, Inc.
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

/*
 *	ANSI Standard: 4.6 NON-LOCAL JUMPS <setjmp.h>
 */

#ifndef	_SETJMP_H

#define	_SETJMP_H	1
#include <features.h>

__BEGIN_DECLS

/* Get the machine-dependent definition of `__jmp_buf'.  */
#include <jmp_buf.h>
#include <sigset.h>		/* Get `__sigset_t'.  */

/* Calling environment, plus possibly a saved signal mask.  */
typedef struct __jmp_buf	/* C++ doesn't like tagless structs.  */
  {
    /* NOTE: The machine-dependent definitions of `__sigsetjmp'
       assume that a `jmp_buf' begins with a `__jmp_buf'.
       Do not move this member or add others before it.  */
    __jmp_buf __jmpbuf;		/* Calling environment.  */
    int __mask_was_saved;	/* Saved the signal mask?  */
    __sigset_t __saved_mask;	/* Saved signal mask.  */
  } jmp_buf[1];


/* Store the calling environment in ENV, also saving the
   signal mask if SAVEMASK is nonzero.  Return 0.
   This is the internal name for `sigsetjmp'.  */
extern int __sigsetjmp __P ((jmp_buf __env, int __savemask));

#ifndef	__FAVOR_BSD
/* Set ENV to the current position and return 0, not saving the signal mask.
   This is just like `sigsetjmp (ENV, 0)'.
   The ANSI C standard says `setjmp' is a macro.  */
#define	setjmp(env)	__sigsetjmp ((env), 0)
#else
/* We are in 4.3 BSD-compatibility mode in which `setjmp'
   saves the signal mask like `sigsetjmp (ENV, 1)'.  */
#define	setjmp(env)	__sigsetjmp ((env), 1)
#endif /* Favor BSD.  */

#ifdef __USE_BSD
/* Set ENV to the current position and return 0, not saving the signal mask.
   This is the 4.3 BSD name for ANSI `setjmp'.  */
#define _setjmp(env)	__sigsetjmp ((env), 0)
#endif


/* Jump to the environment saved in ENV, making the
   `setjmp' call there return VAL, or 1 if VAL is 0.  */
extern void longjmp __P ((jmp_buf __env, int __val))
     __attribute__ ((__noreturn__));
#ifdef	__USE_BSD
/* Same.  Usually `_longjmp' is used with `_setjmp', which does not save
   the signal mask.  But it is how ENV was saved that determines whether
   `longjmp' restores the mask; `_longjmp' is just an alias.  */
extern void _longjmp __P ((jmp_buf __env, int __val))
     __attribute__ ((__noreturn__));
#endif /* Use BSD.  */

/* Internal machine-dependent function to restore context sans signal mask.  */
extern void __longjmp __P ((__jmp_buf __env, int __val))
     __attribute__ ((__noreturn__));

/* Internal function to possibly save the current mask of blocked signals
   in ENV, and always set the flag saying whether or not it was saved.
   This is used by the machine-dependent definition of `__sigsetjmp'.
   Always returns zero, for convenience.  */
extern int __sigjmp_save __P ((jmp_buf __env, int __savemask));


#ifdef	__USE_POSIX
/* Use the same type for `jmp_buf' and `sigjmp_buf'.
   The `__mask_was_saved' flag determines whether
   or not `longjmp' will restore the signal mask.  */
typedef jmp_buf sigjmp_buf;

/* Store the calling environment in ENV, also saving the
   signal mask if SAVEMASK is nonzero.  Return 0.  */
#define	sigsetjmp(env, savemask)	__sigsetjmp ((env), (savemask))

/* Jump to the environment saved in ENV, making the
   sigsetjmp call there return VAL, or 1 if VAL is 0.
   Restore the signal mask if that sigsetjmp call saved it.
   This is just an alias `longjmp'.  */
extern void siglongjmp __P ((sigjmp_buf __env, int __val))
     __attribute__ ((__noreturn__));
#endif /* Use POSIX.  */

__END_DECLS

#endif /* setjmp.h  */
