/* Copyright (C) 1997, 1998 Free Software Foundation, Inc.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* System V ABI compliant user-level context switching support.  */

#ifndef _UCONTEXT_H
#define _UCONTEXT_H	1

#include <features.h>

/* Get machine dependent definition of data structures.  */
#include <sys/ucontext.h>

__BEGIN_DECLS

/* Get user context and store it in variable pointed to by UCP.  */
extern int getcontext __P ((ucontext_t *__ucp));

/* Set user context from information of variable pointed to by UCP.  */
extern int setcontext __P ((__const ucontext_t *__ucp));

/* Save current context in context variable pointed to by OUCP and set
   context from variable pointed to by UCP.  */
extern int swapcontext __P ((ucontext_t *__oucp, __const ucontext_t *__ucp));

/* Manipulate user context UCP to continue with calling functions FUNC
   and the ARGC-1 parameters following ARGC when the context is used
   the next time in `setcontext' or `swapcontext'.

   We cannot say anything about the parameters FUNC takes; `void'
   is as good as any other choice.  */
extern void makecontext __P ((ucontext_t *__ucp, void (*__func) (void),
			      int __argc, ...));

__END_DECLS

#endif /* ucontext.h */
