/* Copyright (C) 1993, 1994, 1995, 1997, 1999 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

/* This file is deprecated and is provided only for compatibility with
   Unix systems.  It is unwise to include this file on programs which
   are intended only for GNU systems.

   Parts from:

 * Copyright (c) 1982, 1986, 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)param.h	7.23 (Berkeley) 5/6/91
 */


#ifndef	_SYS_PARAM_H

#define	_SYS_PARAM_H	1
#include <features.h>

#define __need_NULL
#include <stddef.h>

#include <sys/types.h>
#include <errno.h>
#include <signal.h>
#include <endian.h>
#include <limits.h>
#ifdef notyet
# include <ufs/param.h>
#endif


/* What versions of BSD we are compatible with.  */
#define	BSD	199306		/* System version (year & month). */
#define BSD4_3	1
#define BSD4_4	1

#define	GNU	1994100		/* GNU version (year, month, and release).  */


/* BSD names for some <limits.h> values.  We do not define the BSD names
   for the values which are not statically limited, such as NOFILE.  */

#define	NBBY		CHAR_BIT
#define	NGROUPS		NGROUPS_MAX
#define	MAXSYMLINKS	SYMLOOP_MAX
#define	CANBSIZ		MAX_CANON /* XXX ? */

/* ARG_MAX is unlimited, but we define NCARGS for BSD programs that want to
   compare against some fixed limit.  */
#define	NCARGS		INT_MAX

/* There is nothing quite equivalent in GNU to Unix "mounts", but there is
   no limit on the number of simultaneously attached filesystems.  */
#define NMOUNT		INT_MAX


/* Magical constants.  */
#define	NOGROUP	65535		/* Marker for empty group set member.  */
#define	NODEV	((dev_t) -1)	/* Non-existent device.  */


/* Bit map related macros.  */
#define	setbit(a,i)	((a)[(i)/NBBY] |= 1<<((i)%NBBY))
#define	clrbit(a,i)	((a)[(i)/NBBY] &= ~(1<<((i)%NBBY)))
#define	isset(a,i)	((a)[(i)/NBBY] & (1<<((i)%NBBY)))
#define	isclr(a,i)	(((a)[(i)/NBBY] & (1<<((i)%NBBY))) == 0)

/* Macros for counting and rounding.  */
#ifndef howmany
# define howmany(x, y)	(((x)+((y)-1))/(y))
#endif
#define	roundup(x, y)	((((x)+((y)-1))/(y))*(y))
#define powerof2(x)	((((x)-1)&(x))==0)

/* Macros for min/max.  */
#define	MIN(a,b) (((a)<(b))?(a):(b))
#define	MAX(a,b) (((a)>(b))?(a):(b))


/* Scale factor for scaled integers used to count %cpu time and load avgs.

   The number of CPU `tick's that map to a unique `%age' can be expressed
   by the formula (1 / (2 ^ (FSHIFT - 11))).  The maximum load average that
   can be calculated (assuming 32 bits) can be closely approximated using
   the formula (2 ^ (2 * (16 - FSHIFT))) for (FSHIFT < 15).  */

#define	FSHIFT	11		/* Bits to right of fixed binary point.  */
#define FSCALE	(1<<FSHIFT)

#endif	 /* sys/param.h */
