/* Definitions for thread-local data handling.  linuxthreads/PPC version.
   Copyright (C) 2003 Free Software Foundation, Inc.
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
#define _TLS_H

#ifndef __ASSEMBLER__

# include <pt-machine.h>
# include <stddef.h>

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
#endif /* __ASSEMBLER__ */

#ifdef HAVE_TLS_SUPPORT

/* Signal that TLS support is available.  */
# define USE_TLS	1

# ifndef __ASSEMBLER__

/* This is the size of the initial TCB.  */
#  define TLS_INIT_TCB_SIZE	sizeof (tcbhead_t)

/* Alignment requirements for the initial TCB.  */
#  define TLS_INIT_TCB_ALIGN	__alignof__ (tcbhead_t)

/* This is the size of the TCB.  */
#  define TLS_TCB_SIZE		sizeof (tcbhead_t)

/* Alignment requirements for the TCB.  */
#  define TLS_TCB_ALIGN		__alignof__ (tcbhead_t)

/* This is the size we need before TCB.  */
#  define TLS_PRE_TCB_SIZE	sizeof (struct _pthread_descr_struct)

/* The following assumes that TP (R13) is points to the end of the
   TCB + 0x7000 (per the ABI).  This implies that TCB address is
   R13-(TLS_TCB_SIZE + 0x7000).  As we define TLS_DTV_AT_TP we can
   assume that the pthread_descr is allocated immediately ahead of the
   TCB.  This implies that the pthread_descr address is
   R13-(TLS_PRE_TCB_SIZE + TLS_TCB_SIZE + 0x7000).  */
#  define TLS_TCB_OFFSET	0x7000

/* The DTV is allocated at the TP; the TCB is placed elsewhere.  */
/* This is not really true for powerpc64.  We are following alpha
   where the DTV pointer is first doubleword in the TCB.  */
#  define TLS_DTV_AT_TP 1

/* Install the dtv pointer.  The pointer passed is to the element with
   index -1 which contain the length.  */
#  define INSTALL_DTV(TCBP, DTVP) \
  (((tcbhead_t *) (TCBP))->dtv = (DTVP) + 1)

/* Install new dtv for current thread.  */
#  define INSTALL_NEW_DTV(DTV) (THREAD_DTV() = (DTV))

/* Return dtv of given thread descriptor.  */
#  define GET_DTV(TCBP)	(((tcbhead_t *) (TCBP))->dtv)

/* The global register variable is declared in pt-machine.h with
   the wrong type, but the compiler doesn't like us declaring another.  */
#  define __thread_register ((void *) __thread_self)

/* Code to initially initialize the thread pointer.  This might need
   special attention since 'errno' is not yet available and if the
   operation can cause a failure 'errno' must not be touched.  */
# define TLS_INIT_TP(TCBP, SECONDCALL) \
    (__thread_register = (void *) (TCBP) + TLS_TCB_OFFSET + TLS_TCB_SIZE, 0)

/* Return the address of the dtv for the current thread.  */
#  define THREAD_DTV() \
     (((tcbhead_t *) (__thread_register - TLS_TCB_OFFSET - TLS_TCB_SIZE))->dtv)

/* Return the thread descriptor for the current thread.  */
#  undef THREAD_SELF
#  define THREAD_SELF \
    ((pthread_descr) (__thread_register \
		      - TLS_TCB_OFFSET - TLS_TCB_SIZE - TLS_PRE_TCB_SIZE))

#  undef INIT_THREAD_SELF
#  define INIT_THREAD_SELF(DESCR, NR) \
     (__thread_register = ((void *) (DESCR) \
		           + TLS_TCB_OFFSET + TLS_TCB_SIZE + TLS_PRE_TCB_SIZE))

/* Get the thread descriptor definition.  */
#  include <linuxthreads/descr.h>

/* Generic bits of LinuxThreads may call these macros with
   DESCR set to NULL.  We are expected to be able to reference
   the "current" value.  */

#  define THREAD_GETMEM(descr, member) \
     ((void) sizeof (descr), THREAD_SELF->member)
#  define THREAD_SETMEM(descr, member, value) \
     ((void) sizeof (descr), THREAD_SELF->member = (value))

#define THREAD_GETMEM_NC(descr, member) THREAD_GETMEM (descr, member)
#define THREAD_SETMEM_NC(descr, member, value) \
  THREAD_SETMEM ((descr), member, (value))

# endif /* __ASSEMBLER__ */

#else /* Not HAVE_TLS_SUPPORT.  */

#define NONTLS_INIT_TP							\
  do {									\
    static const tcbhead_t nontls_init_tp				\
      = { .multiple_threads = 0 };					\
    __thread_self = (__typeof (__thread_self)) &nontls_init_tp;		\
  } while (0)

#endif /* HAVE_TLS_SUPPORT */

#endif	/* tls.h */
