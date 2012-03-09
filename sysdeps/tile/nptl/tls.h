/* Copyright (C) 2011 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Chris Metcalf <cmetcalf@tilera.com>, 2011.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _TLS_H
#define _TLS_H	1

# include <dl-sysdep.h>

#ifndef __ASSEMBLER__
# include <stdbool.h>
# include <stddef.h>
# include <stdint.h>

/* Type for the dtv.  */
typedef union dtv
{
  size_t counter;
  struct
  {
    void *val;
    bool is_static;
  } pointer;
} dtv_t;

#else /* __ASSEMBLER__ */
# include <tcb-offsets.h>
#endif /* __ASSEMBLER__ */


#ifndef __ASSEMBLER__

/* Get system call information.  */
# include <sysdep.h>

/* The TP points to the start of the thread blocks.  */
# define TLS_DTV_AT_TP	1

/* We use the multiple_threads field in the pthread struct */
#define TLS_MULTIPLE_THREADS_IN_TCB	1

/* Get the thread descriptor definition.  */
# include <nptl/descr.h>

/* The stack_guard is accessed directly by GCC -fstack-protector code,
   so it is a part of public ABI.  The dtv and pointer_guard fields
   are private.  */
typedef struct
{
  void *feedback_data;
  uintptr_t pointer_guard;
  uintptr_t stack_guard;
  dtv_t *dtv;
} tcbhead_t;

/* This is the size of the initial TCB.  Because our TCB is before the thread
   pointer, we don't need this.  */
# define TLS_INIT_TCB_SIZE	0

/* Alignment requirements for the initial TCB.  */
# define TLS_INIT_TCB_ALIGN	__alignof__ (struct pthread)

/* This is the size of the TCB.  Because our TCB is before the thread
   pointer, we don't need this.  */
# define TLS_TCB_SIZE		0

/* Alignment requirements for the TCB.  */
# define TLS_TCB_ALIGN		__alignof__ (struct pthread)

/* This is the size we need before TCB - actually, it includes the TCB.  */
# define TLS_PRE_TCB_SIZE \
  (sizeof (struct pthread)						      \
   + ((sizeof (tcbhead_t) + TLS_TCB_ALIGN - 1) & ~(TLS_TCB_ALIGN - 1)))

/* Return the thread descriptor (tp) for the current thread.  */
register void *__thread_pointer asm ("tp");

/* The thread pointer (in hardware register tp) points to the end of
   the TCB.  The pthread_descr structure is immediately in front of the TCB.  */
# define TLS_TCB_OFFSET	0

/* Install the dtv pointer.  The pointer passed is to the element with
   index -1 which contain the length.  */
# define INSTALL_DTV(tcbp, dtvp) \
  (((tcbhead_t *) (tcbp))[-1].dtv = (dtvp) + 1)

/* Install new dtv for current thread.  */
# define INSTALL_NEW_DTV(dtv) (THREAD_DTV() = (dtv))

/* Return dtv of given thread descriptor.  */
# define GET_DTV(tcbp)	(((tcbhead_t *) (tcbp))[-1].dtv)

/* Code to initially initialize the thread pointer (tp).  */
# define TLS_INIT_TP(tcbp, secondcall) \
    (__thread_pointer = (char *)(tcbp) + TLS_TCB_OFFSET, NULL)

/* Return the address of the dtv for the current thread.  */
# define THREAD_DTV() \
    (((tcbhead_t *) (__thread_pointer - TLS_TCB_OFFSET))[-1].dtv)

/* Return the thread descriptor for the current thread.  */
# define THREAD_SELF \
    ((struct pthread *) (__thread_pointer \
			 - TLS_TCB_OFFSET - TLS_PRE_TCB_SIZE))

/* Magic for libthread_db to know how to do THREAD_SELF.  */
#ifdef __tilegx__
# define DB_THREAD_SELF \
  REGISTER (64, 64, REG_TP * 8, - TLS_TCB_OFFSET - TLS_PRE_TCB_SIZE)
#else
# define DB_THREAD_SELF \
  REGISTER (32, 32, REG_TP * 4, - TLS_TCB_OFFSET - TLS_PRE_TCB_SIZE)
#endif

/* Read member of the thread descriptor directly.  */
# define THREAD_GETMEM(descr, member) (descr->member)

/* Same as THREAD_GETMEM, but the member offset can be non-constant.  */
# define THREAD_GETMEM_NC(descr, member, idx) \
    (descr->member[idx])

/* Set member of the thread descriptor directly.  */
# define THREAD_SETMEM(descr, member, value) \
    (descr->member = (value))

/* Same as THREAD_SETMEM, but the member offset can be non-constant.  */
# define THREAD_SETMEM_NC(descr, member, idx, value) \
    (descr->member[idx] = (value))

/* Set the stack guard field in TCB head.  */
# define THREAD_SET_STACK_GUARD(value) \
    (((tcbhead_t *) ((char *) __thread_pointer				      \
		     - TLS_TCB_OFFSET))[-1].stack_guard = (value))
# define THREAD_COPY_STACK_GUARD(descr) \
    (((tcbhead_t *) ((char *) (descr)					      \
		     + TLS_PRE_TCB_SIZE))[-1].stack_guard		      \
     = ((tcbhead_t *) ((char *) __thread_pointer			      \
		       - TLS_TCB_OFFSET))[-1].stack_guard)

/* Set the pointer guard field in TCB head.  */
# define THREAD_GET_POINTER_GUARD() \
    (((tcbhead_t *) ((char *) __thread_pointer				      \
		     - TLS_TCB_OFFSET))[-1].pointer_guard)
# define THREAD_SET_POINTER_GUARD(value) \
    (THREAD_GET_POINTER_GUARD () = (value))
# define THREAD_COPY_POINTER_GUARD(descr) \
    (((tcbhead_t *) ((char *) (descr)					      \
		     + TLS_PRE_TCB_SIZE))[-1].pointer_guard		      \
     = THREAD_GET_POINTER_GUARD())

/* l_tls_offset == 0 is perfectly valid on Tile, so we have to use some
   different value to mean unset l_tls_offset.  */
# define NO_TLS_OFFSET		-1

/* Get and set the global scope generation counter in struct pthread.  */
#define THREAD_GSCOPE_FLAG_UNUSED 0
#define THREAD_GSCOPE_FLAG_USED   1
#define THREAD_GSCOPE_FLAG_WAIT   2
#define THREAD_GSCOPE_RESET_FLAG() \
  do									     \
    { int __res								     \
	= atomic_exchange_rel (&THREAD_SELF->header.gscope_flag,	     \
			       THREAD_GSCOPE_FLAG_UNUSED);		     \
      if (__res == THREAD_GSCOPE_FLAG_WAIT)				     \
	lll_futex_wake (&THREAD_SELF->header.gscope_flag, 1, LLL_PRIVATE);   \
    }									     \
  while (0)
#define THREAD_GSCOPE_SET_FLAG() \
  do									     \
    {									     \
      THREAD_SELF->header.gscope_flag = THREAD_GSCOPE_FLAG_USED;	     \
      atomic_write_barrier ();						     \
    }									     \
  while (0)
#define THREAD_GSCOPE_WAIT() \
  GL(dl_wait_lookup_done) ()

#endif /* __ASSEMBLER__ */

#endif /* tls.h */
