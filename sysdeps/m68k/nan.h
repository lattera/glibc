/* `NAN' constants for m68k.
   Copyright (C) 1997 Free Software Foundation, Inc.
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

#define NAN							\
  (__extension__						\
   ((union { unsigned long long __l; double __d; })		\
    { __l: 0x7fffffffffffffffULL }).__d)

#define NANF							\
  (__extension__						\
   ((union { unsigned long __l; float __f; })			\
    { __l: 0x7fffffffUL }).__f)

#define NANL							\
  (__extension__						\
   ((union { unsigned long __l[3]; long double __ld; })		\
    { __l: { 0x7fff0000UL, 0xffffffffUL, 0xffffffffUL } }).__ld)

#else

static union { unsigned char __c[8]; double __d; } __nan =
  { { 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff } };
#define	NAN	(__nan.__d)

static union { unsigned char __c[4]; float __f; } __nanf =
  { { 0x7f, 0xff, 0xff, 0xff } };
#define	NANF	(__nanf.__f)

static union { unsigned char __c[12]; long double __ld; } __nanl =
  { { 0x7f, 0xff, 0, 0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff } };
#define	NANL	(__nanl.__ld)

#endif	/* GCC.  */

#endif	/* nan.h */
