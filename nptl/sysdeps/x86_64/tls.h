/* Definition for thread-local data handling.  nptl/x86_64 version.
   Copyright (C) 2002, 2003 Free Software Foundation, Inc.
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

#ifndef _TLS_H
#define _TLS_H	1

#include <asm/prctl.h>	/* For ARCH_SET_FS.  */
#ifndef __ASSEMBLER__
# include <stddef.h>
# include <stdint.h>
# include <stdlib.h>


/* Type for the dtv.  */
typedef union dtv
{
  size_t counter;
  void *pointer;
} dtv_t;


typedef struct
{
  void *tcb;		/* Pointer to the TCB.  Not necessary the
			   thread descriptor used by libpthread.  */
  dtv_t *dtv;
  void *self;		/* Pointer to the thread descriptor.  */
  int multiple_threads;
} tcbhead_t;

#else /* __ASSEMBLER__ */
# include <tcb-offsets.h>
#endif


/* We require TLS support in the tools.  */
#ifndef HAVE_TLS_SUPPORT
# error "TLS support is required."
#endif

/* Signal that TLS support is available.  */
#define USE_TLS	1

/* Alignment requirement for the stack.  */
#define STACK_ALIGN	16


#ifndef __ASSEMBLER__
/* Get system call information.  */
# include <sysdep.h>


/* Get the thread descriptor definition.  */
# include <nptl/descr.h>

#ifndef LOCK_PREFIX
# ifdef UP
#  define LOCK_PREFIX	/* nothing */
# else
#  define LOCK_PREFIX	"lock;"
# endif
#endif

/* This is the size of the initial TCB.  */
# define TLS_INIT_TCB_SIZE sizeof (tcbhead_t)

/* Alignment requirements for the initial TCB.  */
# define TLS_INIT_TCB_ALIGN __alignof__ (tcbhead_t)

/* This is the size of the TCB.  */
# define TLS_TCB_SIZE sizeof (struct pthread)

/* Alignment requirements for the TCB.  */
# define TLS_TCB_ALIGN __alignof__ (struct pthread)

/* The TCB can have any size and the memory following the address the
   thread pointer points to is unspecified.  Allocate the TCB there.  */
# define TLS_TCB_AT_TP	1


/* Install the dtv pointer.  The pointer passed is to the element with
   index -1 which contain the length.  */
# define INSTALL_DTV(descr, dtvp) \
  ((tcbhead_t *) (descr))->dtv = (dtvp) + 1

/* Install new dtv for current thread.  */
# define INSTALL_NEW_DTV(dtvp) \
  ({ struct pthread *__pd;						      \
     THREAD_SETMEM (__pd, header.dtv, (dtvp)); })

/* Return dtv of given thread descriptor.  */
# define GET_DTV(descr) \
  (((tcbhead_t *) (descr))->dtv)


/* Macros to load from and store into segment registers.  */
# define TLS_GET_FS() \
  ({ int __seg; __asm ("movl %%fs, %0" : "=q" (__seg)); __seg; })
# define TLS_SET_FS(val) \
  __asm ("movl %0, %%fs" :: "q" (val))


/* Code to initially initialize the thread pointer.  This might need
   special attention since 'errno' is not yet available and if the
   operation can cause a failure 'errno' must not be touched.

   We have to make the syscall for both uses of the macro since the
   address might be (and probably is) different.  */
# define TLS_INIT_TP(thrdescr, secondcall) \
  ({ void *_thrdescr = (thrdescr);					      \
     tcbhead_t *_head = _thrdescr;					      \
     int _result;							      \
									      \
     _head->tcb = _thrdescr;						      \
     /* For now the thread descriptor is at the same address.  */	      \
     _head->self = _thrdescr;						      \
									      \
     /* It is a simple syscall to set the %fs value for the thread.  */	      \
     asm volatile ("syscall"						      \
		   : "=a" (_result)					      \
		   : "0" ((unsigned long int) __NR_arch_prctl),		      \
		     "D" ((unsigned long int) ARCH_SET_FS),		      \
		     "S" (_thrdescr)					      \
		   : "memory", "cc", "r11", "cx");			      \
									      \
    _result ? "cannot set %fs base address for thread-local storage" : 0;     \
  })


/* Return the address of the dtv for the current thread.  */
# define THREAD_DTV() \
  ({ struct pthread *__pd;						      \
     THREAD_GETMEM (__pd, header.dtv); })


/* Return the thread descriptor for the current thread.

   The contained asm must *not* be marked volatile since otherwise
   assignments like
        pthread_descr self = thread_self();
   do not get optimized away.  */
# define THREAD_SELF \
  ({ struct pthread *__self;						      \
     asm ("movq %%fs:%c1,%q0" : "=r" (__self)				      \
	  : "i" (offsetof (struct pthread, header.self)));	 	      \
     __self;})

/* Magic for libthread_db to know how to do THREAD_SELF.  */
# define DB_THREAD_SELF_INCLUDE  <sys/reg.h> /* For the FS constant.  */
# define DB_THREAD_SELF CONST_THREAD_AREA (64, FS)

/* Read member of the thread descriptor directly.  */
# define THREAD_GETMEM(descr, member) \
  ({ __typeof (descr->member) __value;					      \
     if (sizeof (__value) == 1)						      \
       asm volatile ("movb %%fs:%P2,%b0"				      \
		     : "=q" (__value)					      \
		     : "0" (0), "i" (offsetof (struct pthread, member)));     \
     else if (sizeof (__value) == 4)					      \
       asm volatile ("movl %%fs:%P1,%0"					      \
		     : "=r" (__value)					      \
		     : "i" (offsetof (struct pthread, member)));	      \
     else								      \
       {								      \
	 if (sizeof (__value) != 8)					      \
	   /* There should not be any value with a size other than 1,	      \
	      4 or 8.  */						      \
	   abort ();							      \
									      \
	 asm volatile ("movq %%fs:%P1,%q0"				      \
		       : "=r" (__value)					      \
		       : "i" (offsetof (struct pthread, member)));	      \
       }								      \
     __value; })


/* Same as THREAD_GETMEM, but the member offset can be non-constant.  */
# define THREAD_GETMEM_NC(descr, member, idx) \
  ({ __typeof (descr->member[0]) __value;				      \
     if (sizeof (__value) == 1)						      \
       asm volatile ("movb %%fs:%P2(%q3),%b0"				      \
		     : "=q" (__value)					      \
		     : "0" (0), "i" (offsetof (struct pthread, member[0])),   \
		       "r" (idx));					      \
     else if (sizeof (__value) == 4)					      \
       asm volatile ("movl %%fs:%P1(,%q2,4),%0"				      \
		     : "=r" (__value)					      \
		     : "i" (offsetof (struct pthread, member[0])), "r" (idx));\
     else								      \
       {								      \
	 if (sizeof (__value) != 8)					      \
	   /* There should not be any value with a size other than 1,	      \
	      4 or 8.  */						      \
	   abort ();							      \
									      \
	 asm volatile ("movq %%fs:%P1(,%q2,8),%q0"			      \
		       : "=r" (__value)					      \
		       : "i" (offsetof (struct pthread, member[0])),	      \
			 "r" (idx));					      \
       }								      \
     __value; })


/* Loading addresses of objects on x86-64 needs to be treated special
   when generating PIC code.  */
#ifdef __pic__
# define IMM_MODE "nr"
#else
# define IMM_MODE "ir"
#endif


/* Same as THREAD_SETMEM, but the member offset can be non-constant.  */
# define THREAD_SETMEM(descr, member, value) \
  ({ if (sizeof (descr->member) == 1)					      \
       asm volatile ("movb %b0,%%fs:%P1" :				      \
		     : "iq" (value),					      \
		       "i" (offsetof (struct pthread, member)));	      \
     else if (sizeof (descr->member) == 4)				      \
       asm volatile ("movl %0,%%fs:%P1" :				      \
		     : IMM_MODE (value),				      \
		       "i" (offsetof (struct pthread, member)));	      \
     else								      \
       {								      \
	 if (sizeof (descr->member) != 8)				      \
	   /* There should not be any value with a size other than 1,	      \
	      4 or 8.  */						      \
	   abort ();							      \
									      \
	 asm volatile ("movq %q0,%%fs:%P1" :				      \
		       : IMM_MODE ((unsigned long int) value),		      \
			 "i" (offsetof (struct pthread, member)));	      \
       }})


/* Set member of the thread descriptor directly.  */
# define THREAD_SETMEM_NC(descr, member, idx, value) \
  ({ if (sizeof (descr->member[0]) == 1)				      \
       asm volatile ("movb %b0,%%fs:%P1(%q2)" :				      \
		     : "iq" (value),					      \
		       "i" (offsetof (struct pthread, member[0])),	      \
		       "r" (idx));					      \
     else if (sizeof (descr->member[0]) == 4)				      \
       asm volatile ("movl %0,%%fs:%P1(,%q2,4)" :			      \
		     : IMM_MODE (value),				      \
		       "i" (offsetof (struct pthread, member[0])),	      \
		       "r" (idx));					      \
     else								      \
       {								      \
	 if (sizeof (descr->member[0]) != 8)				      \
	   /* There should not be any value with a size other than 1,	      \
	      4 or 8.  */						      \
	   abort ();							      \
									      \
	 asm volatile ("movq %q0,%%fs:%P1(,%q2,8)" :			      \
		       : IMM_MODE ((unsigned long int) value),		      \
			 "i" (offsetof (struct pthread, member[0])),	      \
			 "r" (idx));					      \
       }})


/* Atomic compare and exchange on TLS, returning old value.  */
#define THREAD_ATOMIC_CMPXCHG_VAL(descr, member, newval, oldval) \
  ({ __typeof (descr->member) __ret;					      \
     __typeof (oldval) __old = (oldval);				      \
     if (sizeof (descr->member) == 4)					      \
       asm volatile (LOCK_PREFIX "cmpxchgl %2, %%fs:%P3"		      \
		     : "=a" (__ret)					      \
		     : "0" (__old), "r" (newval),			      \
		       "i" (offsetof (struct pthread, member)));	      \
     else								      \
       /* Not necessary for other sizes in the moment.  */		      \
       abort ();							      \
     __ret; })


/* Atomic set bit.  */
#define THREAD_ATOMIC_BIT_SET(descr, member, bit) \
  (void) ({ if (sizeof ((descr)->member) == 4)				      \
	      asm volatile (LOCK_PREFIX "orl %1, %%fs:%P0"		      \
			    :: "i" (offsetof (struct pthread, member)),	      \
			       "ir" (1 << (bit)));			      \
	    else							      \
	      /* Not necessary for other sizes in the moment.  */	      \
	      abort (); })


#define CALL_THREAD_FCT(descr) \
  ({ void *__res;							      \
     asm volatile ("movq %%fs:%P2, %%rdi\n\t"				      \
		   "callq *%%fs:%P1"					      \
		   : "=a" (__res)					      \
		   : "i" (offsetof (struct pthread, start_routine)),	      \
		     "i" (offsetof (struct pthread, arg))		      \
		   : "di", "si", "cx", "dx", "r8", "r9", "r10", "r11",	      \
		     "memory", "cc");					      \
     __res; })


#endif /* __ASSEMBLER__ */

#endif	/* tls.h */
