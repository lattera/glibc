/* Copyright (C) 1991 Free Software Foundation, Inc.
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

#include <ansidecl.h>

#ifndef	FPCONST
#define	FPCONST(hi0, lo0, hi1, lo1)	{ (lo0), (hi0), (lo1), (hi1) }
#endif

static CONST short int ln10[] = FPCONST(0x4113, 0x5d8d, 0xddaa, 0xa8ac);
#define	LN10	(*(CONST double *) ln10)

#include <../sysdeps/generic/log10.c>
