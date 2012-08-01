/* Compatibility header for old-style Unix parameters and limits.  Stub version.
   Copyright (C) 2012 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _SYS_PARAM_H

#define _SYS_PARAM_H    1
#include <features.h>

#define __need_NULL
#include <stddef.h>

#include <sys/types.h>
#include <errno.h>
#include <signal.h>
#include <endian.h>
#include <limits.h>


/* BSD names for some <limits.h> values.  We do not define the BSD names
   for the values which are not statically limited, such as NOFILE.  */

#define NBBY            CHAR_BIT
#ifdef NGROUPS_MAX
# define NGROUPS         NGROUPS_MAX
#endif
#ifdef SYMLOOP_MAX
# define MAXSYMLINKS     SYMLOOP_MAX
#endif
#define CANBSIZ         MAX_CANON /* XXX ? */

/* ARG_MAX is unlimited, but we define NCARGS for BSD programs that want to
   compare against some fixed limit.  */
#define NCARGS          INT_MAX


/* Magical constants.  */
#define NOGROUP 65535           /* Marker for empty group set member.  */
#define NODEV   ((dev_t) -1)    /* Non-existent device.  */


/* Bit map related macros.  */
#define setbit(a,i)     ((a)[(i)/NBBY] |= 1<<((i)%NBBY))
#define clrbit(a,i)     ((a)[(i)/NBBY] &= ~(1<<((i)%NBBY)))
#define isset(a,i)      ((a)[(i)/NBBY] & (1<<((i)%NBBY)))
#define isclr(a,i)      (((a)[(i)/NBBY] & (1<<((i)%NBBY))) == 0)

/* Macros for counting and rounding.  */
#ifndef howmany
# define howmany(x, y)  (((x)+((y)-1))/(y))
#endif
#define roundup(x, y)   ((((x)+((y)-1))/(y))*(y))
#define powerof2(x)     ((((x)-1)&(x))==0)

/* Macros for min/max.  */
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))


/* Unit of `st_blocks'.  */
#define DEV_BSIZE       512


#endif   /* sys/param.h */
