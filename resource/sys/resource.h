/* Copyright (C) 1992, 94, 96, 97, 98, 99 Free Software Foundation, Inc.
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

#ifndef	_SYS_RESOURCE_H
#define	_SYS_RESOURCE_H	1

#include <features.h>

/* Get the system-dependent definitions of structures and bit values.  */
#include <bits/resource.h>

__BEGIN_DECLS

/* Put the soft and hard limits for RESOURCE in *RLIMITS.
   Returns 0 if successful, -1 if not (and sets errno).  */
#ifndef __USE_FILE_OFFSET64
extern int getrlimit (enum __rlimit_resource __resource,
		      struct rlimit *__rlimits) __THROW;
#else
# ifdef __REDIRECT
extern int __REDIRECT (getrlimit, (enum __rlimit_resource __resource,
				   struct rlimit *__rlimits) __THROW,
		       getrlimit64);
# else
#  define getrlimit getrlimit64
# endif
#endif
#ifdef __USE_LARGEFILE64
extern int getrlimit64 (enum __rlimit_resource __resource,
			struct rlimit64 *__rlimits) __THROW;
#endif

/* Set the soft and hard limits for RESOURCE to *RLIMITS.
   Only the super-user can increase hard limits.
   Return 0 if successful, -1 if not (and sets errno).  */
#ifndef __USE_FILE_OFFSET64
extern int setrlimit (enum __rlimit_resource __resource,
		      __const struct rlimit *__rlimits) __THROW;
#else
# ifdef __REDIRECT
extern int __REDIRECT (setrlimit, (enum __rlimit_resource __resource,
				   __const struct rlimit *__rlimits) __THROW,
		       setrlimit64);
# else
#  define setrlimit setrlimit64
# endif
#endif
#ifdef __USE_LARGEFILE64
extern int setrlimit64 (enum __rlimit_resource __resource,
			__const struct rlimit64 *__rlimits) __THROW;
#endif

/* Return resource usage information on process indicated by WHO
   and put it in *USAGE.  Returns 0 for success, -1 for failure.  */
extern int getrusage (enum __rusage_who __who, struct rusage *__usage) __THROW;

/* Return the highest priority of any process specified by WHICH and WHO
   (see above); if WHO is zero, the current process, process group, or user
   (as specified by WHO) is used.  A lower priority number means higher
   priority.  Priorities range from PRIO_MIN to PRIO_MAX (above).  */
extern int getpriority (enum __priority_which __which, int __who) __THROW;

/* Set the priority of all processes specified by WHICH and WHO (see above)
   to PRIO.  Returns 0 on success, -1 on errors.  */
extern int setpriority (enum __priority_which __which, int __who, int __prio)
     __THROW;

__END_DECLS

#endif	/* sys/resource.h  */
