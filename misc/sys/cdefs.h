/* Copyright (C) 1992, 93, 94, 95, 96, 97, 98 Free Software Foundation, Inc.
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

#ifndef	_SYS_CDEFS_H
#define	_SYS_CDEFS_H	1

#include <features.h>

/* Some user header file might have defined this before.  */
#undef	__P

#ifdef __GNUC__

/* GCC can always grok prototypes.  For C++ programs we add throw()
   to help it optimize the function calls.  But this works only with
   gcc 2.8.x and egcs.  */
# if defined __cplusplus && __GNUC_MINOR__ >= 8
#  define __P(args)	args throw ()
# else
#  define __P(args)	args
# endif
/* This macro will be used for functions which might take C++ callback
   functions.  */
# define __PMT(args)	args
# define __DOTS		, ...

#else	/* Not GCC.  */

# define __inline		/* No inline functions.  */

# if (defined __STDC__ && __STDC__) || defined __cplusplus

#  define __P(args)	args
#  define __PMT(args)	args
#  define __const	const
#  define __signed	signed
#  define __volatile	volatile
#  define __DOTS	, ...

# else	/* Not ANSI C or C++.  */

#  define __P(args)	()	/* No prototypes.  */
#  define __PMT(args)	()
#  define __const		/* No ANSI C keywords.  */
#  define __signed
#  define __volatile
#  define __DOTS

# endif	/* ANSI C or C++.  */

#endif	/* GCC.  */

/* For these things, GCC behaves the ANSI way normally,
   and the non-ANSI way under -traditional.  */

#if defined __STDC__ && __STDC__

# define __CONCAT(x,y)	x ## y
# define __STRING(x)	#x

/* This is not a typedef so `const __ptr_t' does the right thing.  */
# define __ptr_t void *
# define __long_double_t  long double

#else

# define __CONCAT(x,y)	x/**/y
# define __STRING(x)	"x"

# define __ptr_t char *
# define __long_double_t  long double

/* The BSD header files use the ANSI keywords unmodified (this means that
   old programs may lose if they use the new keywords as identifiers), but
   those names are not available under -traditional.  We define them to
   their __ versions, which are taken care of above.  */
#ifdef	__USE_BSD
# define const		__const
# define signed		__signed
# define volatile	__volatile
#endif

#endif	/* __STDC__ */


/* C++ needs to know that types and declarations are C, not C++.  */
#ifdef	__cplusplus
# define __BEGIN_DECLS	extern "C" {
# define __END_DECLS	}
#else
# define __BEGIN_DECLS
# define __END_DECLS
#endif

/* __asm__ ("xyz") is used throughout the headers to rename functions
   at the assembly language level.  This is wrapped by the __REDIRECT
   macro, in order to support compilers that can do this some other
   way.  When compilers don't support asm-names at all, we have to do
   preprocessor tricks instead (which don't have exactly the right
   semantics, but it's the best we can do).

   Example:
   int __REDIRECT(setpgrp, __P((__pid_t pid, __pid_t pgrp)), setpgid); */

#if defined __GNUC__ && __GNUC__ >= 2

# define __REDIRECT(name, proto, alias) name proto __asm__ (__ASMNAME (#alias))
# define __ASMNAME(cname)  __ASMNAME2 (__USER_LABEL_PREFIX__, cname)
# define __ASMNAME2(prefix, cname) __STRING (prefix) cname

/*
#elif __SOME_OTHER_COMPILER__

# define __attribute__(xyz)
# define __REDIRECT(name, proto, alias) name proto; \
	_Pragma("let " #name " = " #alias)
*/
#endif

/* GCC has various useful declarations that can be made with the
   `__attribute__' syntax.  All of the ways we use this do fine if
   they are omitted for compilers that don't understand it. */
#if !defined __GNUC__ || __GNUC__ < 2

# define __attribute__(xyz)	/* Ignore. */

#endif

/* No current version of gcc knows the `restrict' keyword.  Define it
   for now unconditionally to the empty string.  */
#define __restrict

#endif	 /* sys/cdefs.h */
