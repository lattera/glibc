/* Copyright (C) 1992, 1994, 1996 Free Software Foundation, Inc.
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

__BEGIN_DECLS

/* Get the system-dependent definitions of structures and bit values.  */
#include <bits/resource.h>

/* Put the soft and hard limits for RESOURCE in *RLIMITS.
   Returns 0 if successful, -1 if not (and sets errno).  */
extern int __getrlimit __P ((enum __rlimit_resource __resource,
			     struct rlimit *__rlimits));
extern int getrlimit __P ((enum __rlimit_resource __resource,
			   struct rlimit *__rlimits));

/* Set the soft and hard limits for RESOURCE to *RLIMITS.
   Only the super-user can increase hard limits.
   Return 0 if successful, -1 if not (and sets errno).  */
extern int setrlimit __P ((enum __rlimit_resource __resource,
			   struct rlimit *__rlimits));

/* Return resource usage information on process indicated by WHO
   and put it in *USAGE.  Returns 0 for success, -1 for failure.  */
extern int __getrusage __P ((enum __rusage_who __who, struct rusage *__usage));
extern int getrusage __P ((enum __rusage_who __who, struct rusage *__usage));

/* Function depends on CMD:
   1 = Return the limit on the size of a file, in units of 512 bytes.
   2 = Set the limit on the size of a file to NEWLIMIT.  Only the
       super-user can increase the limit.
   3 = Return the maximum possible address of the data segment.
   4 = Return the maximum number of files that the calling process can open.
   Returns -1 on errors.  */
extern long int __ulimit __P ((int __cmd, long int __newlimit));
extern long int ulimit __P ((int __cmd, long int __newlimit));

/* Return the highest priority of any process specified by WHICH and WHO
   (see above); if WHO is zero, the current process, process group, or user
   (as specified by WHO) is used.  A lower priority number means higher
   priority.  Priorities range from PRIO_MIN to PRIO_MAX (above).  */
extern int getpriority __P ((enum __priority_which __which, int __who));

/* Set the priority of all processes specified by WHICH and WHO (see above)
   to PRIO.  Returns 0 on success, -1 on errors.  */
extern int setpriority __P ((enum __priority_which __which, int __who,
			     int __prio));


__END_DECLS

#endif	/* sys/resource.h  */
