/* Bit values for resource limits.  Linux/Alpha version.
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
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

/* These are the values for Linux/Alpha.  */

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
    /* Number of open files.  */
    RLIMIT_OFILE,
#define	RLIMIT_OFILE	RLIMIT_OFILE
    RLIMIT_NOFILE = RLIMIT_OFILE, /* Another name for the same thing.  */
#define	RLIMIT_NOFILE	RLIMIT_NOFILE
    /* Address space limit.  */
    RLIMIT_AS,
#define	RLIMIT_AS	RLIMIT_AS
    /* Number of processes.  */
    RLIMIT_NPROC,
#define	RLIMIT_NPROC	RLIMIT_NPROC
    /* Locked-in-memory address space.  */
    RLIMIT_MEMLOCK,
#define	RLIMIT_MEMLOCK	RLIMIT_MEMLOCK

    RLIMIT_NLIMITS,		/* Number of limit flavors.  */
    RLIM_NLIMITS = RLIMIT_NLIMITS /* Traditional name for same.  */
  };
