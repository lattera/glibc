/* `fd_set' type and related macros, and `select'/`pselect' declarations.
   Copyright (C) 1996, 1997, 1998 Free Software Foundation, Inc.
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

/*	POSIX 1003.1g: 6.2 Select from File Descriptor Sets <sys/select.h>  */

#ifndef _SYS_SELECT_H
#define _SYS_SELECT_H	1

#include <features.h>

/* Get definition of needed basic types.  */
#include <bits/types.h>

/* Get __FD_* definitions.  */
#include <bits/select.h>

/* Get definition of timer specification structures.  */
#define __need_timespec
#include <time.h>

__BEGIN_DECLS

/* This declaration puts `struct timeval' in global scope even if
   <sys/time.h> has not been included to define it.  That way the
   `select' prototype below will not conflict with a later definition
   of `struct timeval'.  */
struct timeval;

typedef __fd_mask fd_mask;

/* Representation of a set of file descriptors.  */
typedef __fd_set fd_set;

/* Maximum number of file descriptors in `fd_set'.  */
#define	FD_SETSIZE		__FD_SETSIZE

#ifdef __USE_MISC
/* Number of bits per word of `fd_set' (some code assumes this is 32).  */
# define NFDBITS		__NFDBITS
#endif


/* Access macros for `fd_set'.  */
#define	FD_SET(fd, fdsetp)	__FD_SET ((fd), (fdsetp))
#define	FD_CLR(fd, fdsetp)	__FD_CLR ((fd), (fdsetp))
#define	FD_ISSET(fd, fdsetp)	__FD_ISSET ((fd), (fdsetp))
#define	FD_ZERO(fdsetp)		__FD_ZERO (fdsetp)


/* Check the first NFDS descriptors each in READFDS (if not NULL) for read
   readiness, in WRITEFDS (if not NULL) for write readiness, and in EXCEPTFDS
   (if not NULL) for exceptional conditions.  If TIMEOUT is not NULL, time out
   after waiting the interval specified therein.  Returns the number of ready
   descriptors, or -1 for errors.  */
extern int __select __P ((int __nfds, __fd_set *__readfds,
			  __fd_set *__writefds, __fd_set *__exceptfds,
			  struct timeval *__timeout));
extern int select __P ((int __nfds, __fd_set *__readfds,
			__fd_set *__writefds, __fd_set *__exceptfds,
			struct timeval *__timeout));

#ifdef __USE_GNU
/* XXX Once/if POSIX.1g gets official this prototype will be available
   when defining __USE_POSIX.  */
/* Same as above only that the TIMEOUT value is given with higher
   resolution.  This version should be used.  */
extern int pselect __P ((int __nfds, __fd_set *__readfds,
			 __fd_set *__writefds, __fd_set *__exceptfds,
			 struct timespec *__timeout));
#endif

__END_DECLS

#endif /* sys/select.h */
