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

#ifndef	_POSIX2_LIMITS_H

#define	_POSIX2_LIMITS_H	1


/* The maximum `ibase' and `obase' values allowed by the `bc' utility.  */
#define	_POSIX2_BC_BASE_MAX	99

/* The maximum number of elements allowed in an array by the `bc' utility.  */
#define	_POSIX2_BC_DIM_MAX	2048

/* The maximum `scale' value allowed by the `bc' utility.  */
#define	_POSIX2_BC_SCALE_MAX	99

/* The maximum length of a string constant accepted by the `bc' utility.  */
#define	_POSIX2_BC_STRING_MAX	1000

/* The maximum number of weights that can be assigned to an entry of
   the LC_COLLATE category `order' keyword in a locale definition.  */
#define	_POSIX2_EQUIV_CLASS_MAX	2

/* The maximum number of expressions that can be nested
   within parentheses by the `expr' utility.  */
#define	_POSIX2_EXPR_NEST_MAX	32

/* The maximum length, in bytes, of an input line.  */
#define	_POSIX2_LINE_MAX	2048

/* The maximum number of repeated occurrences of a regular expression
   permitted when using the interval notation `\{M,N\}'.  */
#define	_POSIX2_RE_DUP_MAX	255


/* These values are implementation-specific,
   and may vary within the implementation.
   Their precise values can be obtained from sysconf.  */

#ifndef	BC_BASE_MAX
#define	BC_BASE_MAX	_POSIX2_BC_BASE_MAX
#endif
#ifndef	BC_DIM_MAX
#define	BC_DIM_MAX	_POSIX2_BC_DIM_MAX
#endif
#ifndef	BC_SCALE_MAX
#define	BC_SCALE_MAX	_POSIX2_BC_SCALE_MAX
#endif
#ifndef	BC_STRING_MAX
#define	BC_STRING_MAX	_POSIX2_BC_STRING_MAX
#endif
#ifndef	EQUIV_CLASS_MAX
#define	EQUIV_CLASS_MAX	_POSIX2_EQUIV_CLASS_MAX
#endif
#ifndef	EXPR_NEST_MAX
#define	EXPR_NEST_MAX	_POSIX2_EXPR_NEST_MAX
#endif
#ifndef	LINE_MAX
#define	LINE_MAX	_POSIX2_LINE_MAX
#endif
#ifndef	RE_DUP_MAX
#define	RE_DUP_MAX	_POSIX2_RE_DUP_MAX
#endif


#endif	/* posix2_limits.h */
