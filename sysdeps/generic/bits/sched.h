/* Definitions of constants and data structure for POSIX 1003.1b-1993
   scheduling interface.
   Copyright (C) 1996, 1997, 2001, 2003 Free Software Foundation, Inc.
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

#ifndef _SCHED_H
# error "Never include <bits/sched.h> directly; use <sched.h> instead."
#endif


/* Scheduling algorithms.  */
#define SCHED_OTHER	0
#define SCHED_FIFO	1
#define SCHED_RR	2

/* Data structure to describe a process' schedulability.  */
struct sched_param
{
  int __sched_priority;
};


#if defined _SCHED_H && !defined __cpu_set_t_defined
# define __cpu_set_t_defined
/* Size definition for CPU sets.  */
# define __CPU_SETSIZE	1024
# define __NCPUBITS	(8 * sizeof (__cpu_mask))

/* Type for array elements in 'cpu_set'.  */
typedef unsigned long int __cpu_mask;

/* Basic access functions.  */
# define __CPUELT(cpu)	((cpu) / __NCPUBITS)
# define __CPUMASK(cpu)	((__cpu_mask) 1 << ((cpu) % __NCPUBITS))

/* Data structure to describe CPU mask.  */
typedef struct
{
  __cpu_mask __bits[__CPU_SETSIZE / __NCPUBITS];
} cpu_set_t;

/* Access functions for CPU masks.  */
# define __CPU_ZERO(cpusetp) \
  do {									      \
    unsigned int __i;							      \
    cpu_set *__arr = (cpusetp);						      \
    for (__i = 0; __i < sizeof (cpu_set) / sizeof (__cpu_mask); ++__i)	      \
      __arr->__bits[__i] = 0;						      \
  } while (0)
# define __CPU_SET(cpu, cpusetp) \
  ((cpusetp)->__bits[__CPUELT (cpu)] |= __CPUMASK (cpu))
# define __CPU_CLR(cpu, cpusetp) \
  ((cpusetp)->__bits[__CPUELT (cpu)] &= ~__CPUMASK (cpu))
# define __CPU_ISSET(cpu, cpusetp) \
  (((cpusetp)->__bits[__CPUELT (cpu)] & __CPUMASK (cpu)) != 0)
#endif
