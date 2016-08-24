/* Bit values & structures for resource limits.  4.4 BSD/generic GNU version.
   Copyright (C) 1994-2016 Free Software Foundation, Inc.
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

#ifndef _SYS_RESOURCE_H
# error "Never use <bits/resource.h> directly; include <sys/resource.h> instead."
#endif

#include <bits/types.h>

/* These are the values for 4.4 BSD and GNU.  Earlier BSD systems have a
   subset of these kinds of resource limit.  In systems where `getrlimit'
   and `setrlimit' are not system calls, these are the values used by the C
   library to emulate them.  */

/* Kinds of resource limit.  */
enum __rlimit_resource
  {
    /* Per-process CPU limit, in seconds.  */
    RLIMIT_CPU,
#define	RLIMIT_CPU	RLIMIT_CPU
    /* Largest file that can be created, in bytes.  */
    RLIMIT_FSIZE,
#define	RLIMIT_FSIZE	RLIMIT_FSIZE
    /* Maximum size of data segment, in bytes.  */
    RLIMIT_DATA,
#define	RLIMIT_DATA	RLIMIT_DATA
    /* Maximum size of stack segment, in bytes.  */
    RLIMIT_STACK,
#define	RLIMIT_STACK	RLIMIT_STACK
    /* Largest core file that can be created, in bytes.  */
    RLIMIT_CORE,
#define	RLIMIT_CORE	RLIMIT_CORE
    /* Largest resident set size, in bytes.
       This affects swapping; processes that are exceeding their
       resident set size will be more likely to have physical memory
       taken from them.  */
    RLIMIT_RSS,
#define	RLIMIT_RSS	RLIMIT_RSS
    /* Locked-in-memory address space.  */
    RLIMIT_MEMLOCK,
#define	RLIMIT_MEMLOCK	RLIMIT_MEMLOCK
    /* Number of processes.  */
    RLIMIT_NPROC,
#define	RLIMIT_NPROC	RLIMIT_NPROC
    /* Number of open files.  */
    RLIMIT_OFILE,
    RLIMIT_NOFILE = RLIMIT_OFILE, /* Another name for the same thing.  */
#define	RLIMIT_OFILE	RLIMIT_OFILE
#define	RLIMIT_NOFILE	RLIMIT_NOFILE
    /* Maximum size of all socket buffers.  */
    RLIMIT_SBSIZE,
#define RLIMIT_SBSIZE	RLIMIT_SBSIZE
    /* Maximum size in bytes of the process address space.  */
    RLIMIT_AS,
    RLIMIT_VMEM = RLIMIT_AS,	/* Another name for the same thing.  */
#define RLIMIT_AS	RLIMIT_AS
#define RLIMIT_VMEM	RLIMIT_AS

    RLIMIT_NLIMITS,		/* Number of limit flavors.  */
    RLIM_NLIMITS = RLIMIT_NLIMITS /* Traditional name for same.  */
  };

/* Value to indicate that there is no limit.  */
#ifndef __USE_FILE_OFFSET64
# define RLIM_INFINITY 0x7fffffff
#else
# define RLIM_INFINITY 0x7fffffffffffffffLL
#endif

#ifdef __USE_LARGEFILE64
# define RLIM64_INFINITY 0x7fffffffffffffffLL
#endif


/* Type for resource quantity measurement.  */
#ifndef __USE_FILE_OFFSET64
typedef __rlim_t rlim_t;
#else
typedef __rlim64_t rlim_t;
#endif
#ifdef __USE_LARGEFILE64
typedef __rlim64_t rlim64_t;
#endif

struct rlimit
  {
    /* The current (soft) limit.  */
    rlim_t rlim_cur;
    /* The hard limit.  */
    rlim_t rlim_max;
  };

#ifdef __USE_LARGEFILE64
struct rlimit64
  {
    /* The current (soft) limit.  */
    rlim64_t rlim_cur;
    /* The hard limit.  */
    rlim64_t rlim_max;
 };
#endif

/* Whose usage statistics do you want?  */
enum __rusage_who
/* The macro definitions are necessary because some programs want
   to test for operating system features with #ifdef RUSAGE_SELF.
   In ISO C the reflexive definition is a no-op.  */
  {
    /* The calling process.  */
    RUSAGE_SELF = 0,
#define RUSAGE_SELF     RUSAGE_SELF
    /* All of its terminated child processes.  */
    RUSAGE_CHILDREN = -1
#define RUSAGE_CHILDREN RUSAGE_CHILDREN
  };

#include <bits/types/struct_timeval.h>

/* Structure which says how much of each resource has been used.  */
struct rusage
  {
    /* Total amount of user time used.  */
    struct timeval ru_utime;
    /* Total amount of system time used.  */
    struct timeval ru_stime;
    /* Maximum resident set size (in kilobytes).  */
    long int ru_maxrss;
    /* Amount of sharing of text segment memory
       with other processes (kilobyte-seconds).  */
    long int ru_ixrss;
    /* Amount of data segment memory used (kilobyte-seconds).  */
    long int ru_idrss;
    /* Amount of stack memory used (kilobyte-seconds).  */
    long int ru_isrss;
    /* Number of soft page faults (i.e. those serviced by reclaiming
       a page from the list of pages awaiting reallocation.  */
    long int ru_minflt;
    /* Number of hard page faults (i.e. those that required I/O).  */
    long int ru_majflt;
    /* Number of times a process was swapped out of physical memory.  */
    long int ru_nswap;
    /* Number of input operations via the file system.  Note: This
       and `ru_oublock' do not include operations with the cache.  */
    long int ru_inblock;
    /* Number of output operations via the file system.  */
    long int ru_oublock;
    /* Number of IPC messages sent.  */
    long int ru_msgsnd;
    /* Number of IPC messages received.  */
    long int ru_msgrcv;
    /* Number of signals delivered.  */
    long int ru_nsignals;
    /* Number of voluntary context switches, i.e. because the process
       gave up the process before it had to (usually to wait for some
       resource to be available).  */
    long int ru_nvcsw;
    /* Number of involuntary context switches, i.e. a higher priority process
       became runnable or the current process used up its time slice.  */
    long int ru_nivcsw;
  };

/* Priority limits.  */
#define PRIO_MIN        -20     /* Minimum priority a process can have.  */
#define PRIO_MAX        20      /* Maximum priority a process can have.  */

/* The type of the WHICH argument to `getpriority' and `setpriority',
   indicating what flavor of entity the WHO argument specifies.  */
enum __priority_which
  {
    PRIO_PROCESS = 0,           /* WHO is a process ID.  */
#define PRIO_PROCESS PRIO_PROCESS
    PRIO_PGRP = 1,              /* WHO is a process group ID.  */
#define PRIO_PGRP PRIO_PGRP
    PRIO_USER = 2               /* WHO is a user ID.  */
#define PRIO_USER PRIO_USER
  };
