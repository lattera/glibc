/* `NAN' constant for IEEE 754 machines.
   Copyright (C) 1992, 1996, 1997 Free Software Foundation, Inc.
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

#ifndef	_NAN_H

#define	_NAN_H	1

/* IEEE Not A Number.  */

#ifdef	__GNUC__

#define NAN                                                                 \
  (__extension__                                                            \
   ((union { unsigned __l __attribute__((__mode__(__DI__))); double __d; }) \
    { __l: 0x7ff8000000000000ULL }).__d)

#define NANF                                                                \
  (__extension__                                                            \
   ((union { unsigned __l __attribute__((__mode__(__SI__))); float __d; })  \
    { __l: 0x7fc00000UL }).__d)

#else

#include <endian.h>

#if __BYTE_ORDER == __BIG_ENDIAN
#define	__nan_bytes		{ 0x7f, 0xf8, 0, 0, 0, 0, 0, 0 }
#define	__nanf_bytes		{ 0x7f, 0xc0, 0, 0 }
#endif
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define	__nan_bytes		{ 0, 0, 0, 0, 0, 0, 0xf8, 0x7f }
#define	__nanf_bytes		{ 0, 0, 0xc0, 0x7f }
#endif

static union { unsigned char __c[8]; double __d; } __nan = { __nan_bytes };
#define	NAN	(__nan.__d)

static union { unsigned char __c[4]; double __d; } __nanf = { __nanf_bytes };
#define	NANF	(__nanf.__d)

#endif	/* GCC.  */

/* Generally there is no separate `long double' format and it is the
   same as `double'.  */

#define NANL  NAN

#endif	/* nan.h */
