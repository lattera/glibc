/* Copyright (C) 1991, 1992, 1994, 1995 Free Software Foundation, Inc.
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
 *	ANSI Standard: 4.2 DIAGNOSTICS	<assert.h>
 */

#ifdef	_ASSERT_H

#undef	_ASSERT_H
#undef	assert

#endif /* assert.h	*/

#define	_ASSERT_H	1
#include <features.h>

/* void assert (int expression);

   If NDEBUG is defined, do nothing.
   If not, and EXPRESSION is zero, print an error message and abort.  */

#ifdef	NDEBUG

#define	assert(expr)		((void) 0)

/* void assert_perror (int errnum);

   If NDEBUG is defined, do nothing.  If not, and ERRNUM is not zero, print an
   error message with the error text for ERRNUM and abort.
   (This is a GNU extension.) */

#ifdef	__USE_GNU
#define	assert_perror(errnum)	((void) 0)
#endif

#else /* Not NDEBUG.  */

#include <sys/cdefs.h>

__BEGIN_DECLS

/* This prints an "Assertion failed" message and aborts.  */
extern void __assert_fail __P ((__const char *__assertion,
				__const char *__file,
				unsigned int __line,
				__const char *__function))
     __attribute__ ((__noreturn__));

/* Likewise, but prints the error text for ERRNUM.  */
extern void __assert_perror_fail __P ((int __errnum,
				       __const char *__file,
				       unsigned int __line,
				       __const char *__function))
     __attribute__ ((__noreturn__));

__END_DECLS

#define	assert(expr)							      \
  ((void) ((expr) ||							      \
	   (__assert_fail (__STRING(expr),				      \
			   __FILE__, __LINE__, __ASSERT_FUNCTION), 0)))

#ifdef	__USE_GNU
#define assert_perror(errnum)						      \
  ((void) ((errnum) && (__assert_perror_fail ((errnum),			      \
					      __FILE__, __LINE__,	      \
					      __ASSERT_FUNCTION), 0)))
#endif

/* Version 2.4 and later of GCC define a magical variable `__PRETTY_FUNCTION__'
   which contains the name of the function currently being defined.
   This is broken in G++ before version 2.6.  */
#if (!defined (__GNUC__) || __GNUC__ < 2 || \
     __GNUC_MINOR__ < (defined (__cplusplus) ? 6 : 4))
#define __ASSERT_FUNCTION	((__const char *) 0)
#else
#define __ASSERT_FUNCTION	__PRETTY_FUNCTION__
#endif


#endif /* NDEBUG.  */

