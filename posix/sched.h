/* Definitions for POSIX 1003.1b-1993 (aka POSIX.4) scheduling interface.
   Copyright (C) 1996 Free Software Foundation, Inc.
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

#ifndef	_SCHED_H
#define	_SCHED_H	1

#include <features.h>

/* Get type definitions.  */
#include <bits/types.h>
#include <sys/time.h>		/* for struct timespec */

/* Get system specific constant and data structure definitions.  */
#include <bits/sched.h>

__BEGIN_DECLS

/* Set scheduling parameters for a process.  */
extern int __sched_setparam __P ((__pid_t __pid,
				  __const struct sched_param *__param));
extern int sched_setparam __P ((__pid_t __pid,
				__const struct sched_param *__param));

/* Retrieve scheduling parameters for a particular process.  */
extern int __sched_getparam __P ((__pid_t __pid, struct sched_param *__param));
extern int sched_getparam __P ((__pid_t __pid, struct sched_param *__param));

/* Set scheduling algorithm and/or parameters for a process.  */
extern int __sched_setscheduler __P ((__pid_t __pid, int __policy,
				      __const struct sched_param *__param));
extern int sched_setscheduler __P ((__pid_t __pid, int __policy,
				    __const struct sched_param *__param));

/* Retrieve scheduling algorithm for a particular purpose.  */
extern int __sched_getscheduler __P ((__pid_t __pid));
extern int sched_getscheduler __P ((__pid_t __pid));

/* Yield the processor.  */
extern int __sched_yield __P ((void));
extern int sched_yield __P ((void));

/* Get maximum priority value for a scheduler.  */
extern int __sched_get_priority_max __P ((int __algorithm));
extern int sched_get_priority_max __P ((int __algorithm));

/* Get minimum priority value for a scheduler.  */
extern int __sched_get_priority_min __P ((int __algorithm));
extern int sched_get_priority_min __P ((int __algorithm));

/* Get the SCHED_RR interval for the named process.  */
extern int __sched_rr_get_interval __P ((__pid_t __pid, struct timespec *__t));
extern int sched_rr_get_interval __P ((__pid_t __pid, struct timespec *__t));

__END_DECLS

#endif /* sched.h */
