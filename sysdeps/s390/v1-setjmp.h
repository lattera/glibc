/* Copyright (C) 1991-2013 Free Software Foundation, Inc.
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

/*
 *	ISO C99 Standard: 7.13 Nonlocal jumps	<setjmp.h>
 */

#ifndef	_V1_SETJMP_H
#define	_V1_SETJMP_H	1

#include <features.h>

__BEGIN_DECLS

#define __V1_JMPBUF
#define _SETJMP_H
#include <bits/setjmp.h>		/* Get `__jmp_buf'.  */

#ifndef _ASM

#include <bits/sigset.h>		/* Get `__sigset_t'.  */


/* Calling environment, plus possibly a saved signal mask.  */
typedef struct __v1__jmp_buf_tag
  {
    /* NOTE: The machine-dependent definitions of `__sigsetjmp'
       assume that a `jmp_buf' begins with a `__jmp_buf' and that
       `__mask_was_saved' follows it.  Do not move these members
       or add others before it.  */
    __jmp_buf __jmpbuf;		/* Calling environment.  */
    int __mask_was_saved;	/* Saved the signal mask?  */
    __sigset_t __saved_mask;	/* Saved signal mask.  */
  } __v1__jmp_buf[1];


/* Store the calling environment in ENV, also saving the signal mask.
   Return 0.  */
extern int __v1setjmp (__v1__jmp_buf __env);

/* Store the calling environment in ENV, also saving the
   signal mask if SAVEMASK is nonzero.  Return 0.
   This is the internal name for `sigsetjmp'.  */
extern int __v1__sigsetjmp (struct __v1__jmp_buf_tag __env[1],
			       int __savemask);

/* Store the calling environment in ENV, not saving the signal mask.
   Return 0.  */
extern int __v1_setjmp (struct __v1__jmp_buf_tag __env[1]);

/* Jump to the environment saved in ENV, making the
   `setjmp' call there return VAL, or 1 if VAL is 0.  */
extern void __v1longjmp (struct __v1__jmp_buf_tag __env[1], int __val)
     __attribute__ ((__noreturn__));

/* Same.  Usually `_longjmp' is used with `_setjmp', which does not save
   the signal mask.  But it is how ENV was saved that determines whether
   `longjmp' restores the mask; `_longjmp' is just an alias.  */
extern void __v1_longjmp (struct __v1__jmp_buf_tag __env[1], int __val)
     __attribute__ ((__noreturn__));

/* Use the same type for `jmp_buf' and `sigjmp_buf'.
   The `__mask_was_saved' flag determines whether
   or not `longjmp' will restore the signal mask.  */
typedef struct __v1__jmp_buf_tag __v1__sigjmp_buf[1];

/* Jump to the environment saved in ENV, making the
   sigsetjmp call there return VAL, or 1 if VAL is 0.
   Restore the signal mask if that sigsetjmp call saved it.
   This is just an alias `longjmp'.  */
extern void __v1siglongjmp (__v1__sigjmp_buf __env, int __val)
     __attribute__ ((__noreturn__));

/* Internal machine-dependent function to restore context sans signal mask.  */
extern void __v1__longjmp (__jmp_buf __env, int __val)
     __attribute__ ((__noreturn__));

/* Internal function to possibly save the current mask of blocked signals
   in ENV, and always set the flag saying whether or not it was saved.
   This is used by the machine-dependent definition of `__sigsetjmp'.
   Always returns zero, for convenience.  */
extern int __v1__sigjmp_save (__v1__jmp_buf __env, int __savemask);

extern void _longjmp_unwind (__v1__jmp_buf env, int val);

extern void __v1__libc_siglongjmp (__v1__sigjmp_buf env, int val)
          __attribute__ ((noreturn));

extern void __v1__libc_longjmp (__v1__sigjmp_buf env, int val)
     __attribute__ ((noreturn));

libc_hidden_proto (__v1__libc_longjmp)
libc_hidden_proto (__v1_setjmp)
libc_hidden_proto (__v1__sigsetjmp)
#endif /* !_ASM */

#endif /* ifndef _V1_SETJMP_H */
