/* Copyright (C) 1991, 92, 93, 95, 96 Free Software Foundation, Inc.
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
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#ifndef	_FEATURES_H

#define	_FEATURES_H	1

/* These are defined by the user (or the compiler)
   to specify the desired environment:

   __STRICT_ANSI__	ANSI Standard C.
   _POSIX_SOURCE	IEEE Std 1003.1.
   _POSIX_C_SOURCE	If ==1, like _POSIX_SOURCE; if ==2 add IEEE Std 1003.2.
   _BSD_SOURCE		ANSI, POSIX, and 4.3BSD things.
   _SVID_SOURCE		ANSI, POSIX, and SVID things.
   _GNU_SOURCE		All of the above, plus GNU extensions.
   _REENTRANT		Select additionally reentrant object.
   _THREAD_SAFE		Same as _REENTRANT, often used by other systems.

   The `-ansi' switch to the GNU C compiler defines __STRICT_ANSI__.
   If none of these are defined, the default is all but _GNU_SOURCE.
   If more than one of these are defined, they accumulate.
   For example __STRICT_ANSI__, _POSIX_SOURCE and _POSIX_C_SOURCE
   together give you ANSI C, 1003.1, and 1003.2, but nothing else.

   These are defined by this file and are used by the
   header files to decide what to declare or define:

   __USE_POSIX		Define IEEE Std 1003.1 things.
   __USE_POSIX2		Define IEEE Std 1003.2 things.
   __USE_BSD		Define 4.3BSD things.
   __USE_SVID		Define SVID things.
   __USE_MISC		Define things common to BSD and System V Unix.
   __USE_GNU		Define GNU extensions.
   __USE_REENTRANT	Define reentrant/thread-safe *_r functions.
   __FAVOR_BSD		Favor 4.3BSD things in cases of conflict.

   The macro `__GNU_LIBRARY__' is defined by this file unconditionally.

   All macros listed above as possibly being defined by this file are
   explicitly undefined if they are not explicitly defined.
   Feature-test macros that are not defined by the user or compiler
   but are implied by the other feature-test macros defined (or by the
   lack of any definitions) are defined by the file.  */


/* Undefine everything, so we get a clean slate.  */
#undef	__USE_POSIX
#undef	__USE_POSIX2
#undef	__USE_BSD
#undef	__USE_SVID
#undef	__USE_MISC
#undef	__USE_GNU
#undef	__USE_REENTRANT
#undef	__FAVOR_BSD
#undef	__KERNEL_STRICT_NAMES

/* Suppress kernel-name space pollution unless user expressedly asks
   for it: */
#ifndef _LOOSE_KERNEL_NAMES
# define __KERNEL_STRICT_NAMES
#endif

/* Always use ANSI things.  */
#define	__USE_ANSI	1


/* If _BSD_SOURCE was defined by the user, favor BSD over POSIX.  */
#if defined (_BSD_SOURCE) && \
    !(defined (_POSIX_SOURCE) || defined (_POSIX_C_SOURCE) || \
      defined (_GNU_SOURCE) || defined (_SVID_SOURCE))
#define	__FAVOR_BSD	1
#endif

/* If _GNU_SOURCE was defined by the user, turn on all the other features.  */
#ifdef _GNU_SOURCE
#undef	_POSIX_SOURCE
#define	_POSIX_SOURCE	1
#undef	_POSIX_C_SOURCE
#define	_POSIX_C_SOURCE	2
#undef	_BSD_SOURCE
#define	_BSD_SOURCE	1
#undef	_SVID_SOURCE
#define	_SVID_SOURCE	1
#endif

/* If nothing (other than _GNU_SOURCE) is defined,
   define _BSD_SOURCE and _SVID_SOURCE.  */
#if (!defined (__STRICT_ANSI__) && !defined (_POSIX_SOURCE) && \
     !defined (_POSIX_C_SOURCE) && !defined (_BSD_SOURCE) && \
     !defined (_SVID_SOURCE))
#define	_BSD_SOURCE	1
#define	_SVID_SOURCE	1
#endif

/* If none of the ANSI/POSIX macros are defined, use POSIX.1 and POSIX.2.  */
#if (!defined (__STRICT_ANSI__) && !defined (_POSIX_SOURCE) && \
     !defined (_POSIX_C_SOURCE))
#define	_POSIX_SOURCE	1
#define	_POSIX_C_SOURCE	2
#endif

#if	defined (_POSIX_SOURCE) || _POSIX_C_SOURCE >= 1
#define	__USE_POSIX	1
#endif

#if	defined (_POSIX_C_SOURCE) && _POSIX_C_SOURCE >= 2
#define	__USE_POSIX2	1
#endif

#if defined (_BSD_SOURCE) || defined (_SVID_SOURCE)
#define	__USE_MISC	1
#endif

#ifdef	_BSD_SOURCE
#define	__USE_BSD	1
#endif

#ifdef	_SVID_SOURCE
#define	__USE_SVID	1
#endif

#ifdef	_GNU_SOURCE
#define	__USE_GNU	1
#endif

#if defined (_REENTRANT) || defined (_THREAD_SAFE)
#define __USE_REENTRANT	1
#endif


/* This macro indicates that the installed library is the GNU C Library.
   Its value must be incremented whenever any existing library interface
   changes such that callers must be recompiled with the new header files.
   This value should always coincide with the major version number used for
   the shared C library corresponding to this set of header files.  */

#undef	__GNU_LIBRARY__
#define	__GNU_LIBRARY__	6


#if	!defined (__GNUC__) || __GNUC__ < 2
/* In GCC version 2, (__extension__ EXPR) will not complain
   about GCC extensions used in EXPR under -ansi or -pedantic.  */
#define	__extension__
#endif


/* This is here only because every header file already includes this one.  */
#ifndef __ASSEMBLER__
#include <sys/cdefs.h>
#endif

/* This is here only because every header file already includes this one.  */
#ifndef _LIBC
/* Get the definitions of all the appropriate `__stub_FUNCTION' symbols.
   <stubs.h> contains `#define __stub_FUNCTION' when FUNCTION is a stub
   which will always return failure (and set errno to ENOSYS).

   We avoid including <stubs.h> when compiling the C library itself to
   avoid a dependency loop.  stubs.h depends on every object file.  If
   this #include were done for the library source code, then every object
   file would depend on stubs.h.  */

#include <stubs.h>
#endif

#endif	/* __features.h  */
