/* Machine-dependent pthreads configuration and inline functions.
   hppa version.
   Copyright (C) 2000, 2002, 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Richard Henderson <rth@tamu.edu>.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#ifndef _PT_MACHINE_H
#define _PT_MACHINE_H   1

#include <sys/types.h>
#include <bits/initspin.h>

#ifndef PT_EI
# define PT_EI extern inline __attribute__ ((always_inline))
#endif

extern inline long int testandset (__atomic_lock_t *spinlock);
extern inline int __compare_and_swap (long int *p, long int oldval, long int newval);
extern inline int lock_held (__atomic_lock_t *spinlock); 
extern inline int __load_and_clear (__atomic_lock_t *spinlock);

/* Get some notion of the current stack.  Need not be exactly the top
   of the stack, just something somewhere in the current frame.  */
#define CURRENT_STACK_FRAME  stack_pointer
register char * stack_pointer __asm__ ("%r30");

/* Get/Set thread-specific pointer.  We have to call into the kernel to
 * modify it, but we can read it in user mode.  */
#ifndef THREAD_SELF
#define THREAD_SELF __get_cr27()
#endif

#ifndef SET_THREAD_SELF
#define SET_THREAD_SELF(descr) __set_cr27(descr)
#endif
/* Use this to determine type */
struct _pthread_descr_struct *__thread_self;

static inline struct _pthread_descr_struct * __get_cr27(void)
{
  long cr27;
  asm ("mfctl %%cr27, %0" : "=r" (cr27) : );
  return (struct _pthread_descr_struct *) cr27;
}

#ifndef INIT_THREAD_SELF
#define INIT_THREAD_SELF(descr, nr) __set_cr27(descr)
#endif

static inline void __set_cr27(struct _pthread_descr_struct * cr27)
{
  asm ( "ble	0xe0(%%sr2, %%r0)\n\t"
	"copy	%0, %%r26"
	: : "r" (cr27) : "r26" );
}

/* We want the OS to assign stack addresses.  */
#define FLOATING_STACKS	1
#define ARCH_STACK_MAX_SIZE	8*1024*1024

/* The hppa only has one atomic read and modify memory operation,
   load and clear, so hppa spinlocks must use zero to signify that
   someone is holding the lock.  The address used for the ldcw
   semaphore must be 16-byte aligned.  */
#define __ldcw(a) \
({ 									\
  unsigned int __ret;							\
  __asm__ __volatile__("ldcw 0(%1),%0"					\
                      : "=r" (__ret) : "r" (a) : "memory");		\
  __ret;								\
})

/* Strongly ordered lock reset */
#define __lock_reset(lock_addr, tmp) \
({										\
	__asm__ __volatile__ ("stw,ma %1,0(%0)"					\
				: : "r" (lock_addr), "r" (tmp) : "memory"); 	\
})

/* Because malloc only guarantees 8-byte alignment for malloc'd data,
   and GCC only guarantees 8-byte alignment for stack locals, we can't
   be assured of 16-byte alignment for atomic lock data even if we
   specify "__attribute ((aligned(16)))" in the type declaration.  So,
   we use a struct containing an array of four ints for the atomic lock
   type and dynamically select the 16-byte aligned int from the array
   for the semaphore.  */
#define __PA_LDCW_ALIGNMENT 16
#define __ldcw_align(a) ({ \
  volatile unsigned int __ret = (unsigned int) a;			\
  if ((__ret & ~(__PA_LDCW_ALIGNMENT - 1)) < (unsigned int) a)		\
    __ret = (__ret & ~(__PA_LDCW_ALIGNMENT - 1)) + __PA_LDCW_ALIGNMENT; \
  (unsigned int *) __ret;						\
})

/* Spinlock implementation; required.  */
PT_EI int
__load_and_clear (__atomic_lock_t *spinlock)
{
  volatile unsigned int *a = __ldcw_align (spinlock);

  return __ldcw (a);
}

/* Emulate testandset */
PT_EI long int
testandset (__atomic_lock_t *spinlock)
{
  return (__load_and_clear(spinlock) == 0);
}

PT_EI int
lock_held (__atomic_lock_t *spinlock)
{
  volatile unsigned int *a = __ldcw_align (spinlock);

  return *a == 0;
}
		
#endif /* pt-machine.h */
