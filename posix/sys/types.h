/* Copyright (C) 1991, 1992, 1994, 1995, 1996 Free Software Foundation, Inc.
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

/*
 *	POSIX Standard: 2.6 Primitive System Data Types	<sys/types.h>
 */

#ifndef	_SYS_TYPES_H

#define	_SYS_TYPES_H	1
#include <features.h>

__BEGIN_DECLS

#include <gnu/types.h>

#ifdef	__USE_BSD
typedef __u_char u_char;
typedef __u_short u_short;
typedef __u_int u_int;
typedef __u_long u_long;
typedef __quad_t quad_t;
typedef __u_quad_t u_quad_t;
typedef __fsid_t fsid_t;
#endif

typedef __dev_t dev_t;
typedef __gid_t gid_t;
typedef __ino_t ino_t;
typedef __mode_t mode_t;
typedef __nlink_t nlink_t;
typedef __off_t off_t;
typedef __loff_t loff_t;
typedef __pid_t pid_t;
typedef __uid_t uid_t;

#ifndef ssize_t
typedef __ssize_t ssize_t;
#define ssize_t ssize_t
#endif

#ifdef	__USE_BSD
typedef __daddr_t daddr_t;
typedef __caddr_t caddr_t;
#endif

#ifdef  __USE_SVID
typedef __key_t key_t;
#endif

#define	__need_time_t
#include <time.h>

#define	__need_size_t
#include <stddef.h>

#ifdef __USE_MISC
/* Old compatibility names for C types.  */
typedef unsigned long int ulong;
typedef unsigned short int ushort;
typedef unsigned int uint;
#endif

#ifdef __USE_BSD
/* These size-specific names are used by some of the inet code.  */

#if !defined (__GNUC__) || __GNUC__ < 2 || __GNUC_MINOR__ < 7

typedef	char int8_t;
typedef	unsigned char u_int8_t;
typedef	short int int16_t;
typedef	unsigned short int u_int16_t;
typedef	int int32_t;
typedef	unsigned int u_int32_t;
#ifdef __GNUC__
typedef long long int int64_t;
typedef unsigned long long int u_int64_t;
#endif
typedef int register_t;

#else

/* For GCC 2.7 and later, we can use specific type-size attributes.  */
#define __intN_t(N, MODE) \
  typedef int int##N##_t __attribute__ ((__mode__ (MODE)))
#define __u_intN_t(N, MODE) \
  typedef unsigned int u_int##N##_t __attribute__ ((__mode__ (MODE)))

__intN_t (8, __QI__);
__u_intN_t (8, __QI__);
__intN_t (16, __HI__);
__u_intN_t (16, __HI__);
__intN_t (32, __SI__);
__u_intN_t (32, __SI__);
__intN_t (64, __DI__);
__u_intN_t (64, __DI__);

typedef int register_t __attribute__ ((__mode__ (__word__)));

#endif

/* Some code from BIND tests this macro to see if the types above are
   defined.  */
#define __BIT_TYPES_DEFINED__	1
#endif


#ifdef	__USE_BSD
/* In BSD <sys/types.h> is expected to define BYTE_ORDER.  */
#include <endian.h>

/* It also defines `fd_set' and the FD_* macros for `select'.  */
#include <sys/select.h>
#endif /* Use BSD.  */


__END_DECLS

#endif /* sys/types.h */
