/* Copyright (C) 2002 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2002.

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

#ifndef _DESCR_H
#define _DESCR_H	1

#include <limits.h>
#include <sched.h>
#include <setjmp.h>
#include <stdbool.h>
#include <sys/types.h>
#include <hp-timing.h>
#include <list.h>
#include <lowlevellock.h>
#include <pthreaddef.h>
#include "../nptl_db/thread_db.h"


#ifndef TCB_ALIGNMENT
# define TCB_ALIGNMENT	sizeof (double)
#endif


/* We keep thread specific data in a special data structure, a two-level
   array.  The top-level array contains pointers to dynamically allocated
   arrays of a certain number of data pointers.  So we can implement a
   sparse array.  Each dynamic second-level array has
        PTHREAD_KEY_2NDLEVEL_SIZE
   entries.  This value shouldn't be too large.  */
#define PTHREAD_KEY_2NDLEVEL_SIZE       32

/* We need to address PTHREAD_KEYS_MAX key with PTHREAD_KEY_2NDLEVEL_SIZE
   keys in each subarray.  */
#define PTHREAD_KEY_1STLEVEL_SIZE \
  ((PTHREAD_KEYS_MAX + PTHREAD_KEY_2NDLEVEL_SIZE - 1) \
   / PTHREAD_KEY_2NDLEVEL_SIZE)


/* Thread descriptor data structure.  */
struct pthread
{
  /* XXX Remove this union for IA-64 style TLS module */
  union
  {
    /* It is very important to always append new elements.  The offsets
       of some of the elements of the struct are used in assembler code.  */
    struct
    {
      void *tcb;                /* Pointer to the TCB.  This is not always
                                   the address of this thread descriptor.  */
      union dtv *dtvp;
      struct pthread *self;       /* Pointer to this structure */
      list_t list;
      int multiple_threads;
    } data;
    void *__padding[16];
  } header;

  /* Two-level array for the thread-specific data.  */
  struct pthread_key_data
  {
    /* Sequence number.  We use uintptr_t to not require padding on
       32- and 64-bit machines.  On 64-bit machines it helps to avoid
       wrapping, too.  */
    uintptr_t seq;

    /* Data pointer.  */
    void *data;
  } *specific[PTHREAD_KEY_1STLEVEL_SIZE];
  /* We allocate one block of references here.  This should be enough
     to avoid allocating any memory dynamically for most applications.  */
  struct pthread_key_data specific_1stblock[PTHREAD_KEY_2NDLEVEL_SIZE];
  /* Flag which is set when specific data is set.  */
  bool specific_used;

  /* True if the user provided the stack.  */
  bool user_stack;

  /* True if events must be reported.  */
  bool report_events;

  /* Lock to syncronize access to the descriptor.  */
  lll_lock_t lock;

#if HP_TIMING_AVAIL
  /* Offset of the CPU clock at start thread start time.  */
  hp_timing_t cpuclock_offset;
#endif

  /* If the thread waits to join another one the ID of the latter is
     stored here.

     In case a thread is detached this field contains a pointer of the
     TCB if the thread itself.  This is something which cannot happen
     in normal operation.  */
  struct pthread *joinid;
  /* Check whether a thread is detached.  */
#define IS_DETACHED(pd) ((pd)->joinid == (pd))

  /* List of cleanup buffers.  */
  struct _pthread_cleanup_buffer *cleanup;
  /* Flags determining processing of cancellation.  */
  int cancelhandling;
  /* Bit set if cancellation is disabled.  */
#define CANCELSTATE_BIT		0
#define CANCELSTATE_BITMASK	0x01
  /* Bit set if asynchronous cancellation mode is selected.  */
#define CANCELTYPE_BIT		1
#define CANCELTYPE_BITMASK	0x02
  /* Bit set if canceled.  */
#define CANCELED_BIT		2
#define CANCELED_BITMASK	0x04
  /* Bit set if thread is exiting.  */
#define EXITING_BIT		3
#define EXITING_BITMASK		0x08
  /* Bit set if thread terminated and TCB is freed.  */
#define TERMINATED_BIT		4
#define TERMINATED_BITMASK	0x10
  /* Mask for the rest.  Helps the compiler to optimize.  */
#define CANCEL_RESTMASK		0xffffffe0

#define CANCEL_ENABLED_AND_CANCELED(value) \
  (((value) & (CANCELSTATE_BITMASK | CANCELED_BITMASK | EXITING_BITMASK	      \
	       | CANCEL_RESTMASK | TERMINATED_BITMASK)) == CANCELED_BITMASK)
#define CANCEL_ENABLED_AND_CANCELED_AND_ASYNCHRONOUS(value) \
  (((value) & (CANCELSTATE_BITMASK | CANCELTYPE_BITMASK | CANCELED_BITMASK    \
	       | EXITING_BITMASK | CANCEL_RESTMASK | TERMINATED_BITMASK))     \
   == (CANCELTYPE_BITMASK | CANCELED_BITMASK))
  /* Setjmp buffer to be used if try/finally is not available.  */
  sigjmp_buf cancelbuf;
#define HAVE_CANCELBUF	1

  /* Thread ID - which is also a 'is this thread descriptor (and
     therefore stack) used' flag.  */
  pid_t tid;

  /* Flags.  Including those copied from the thread attribute.  */
  int flags;

  /* The result of the thread function.  */
  void *result;

  /* Scheduling parameters for the new thread.  */
  struct sched_param schedparam;
  int schedpolicy;

  /* Start position of the code to be executed and the argument passed
     to the function.  */
  void *(*start_routine) (void *);
  void *arg;

  /* Debug state.  */
  td_eventbuf_t eventbuf;
  /* Next descriptor with a pending event.  */
  struct pthread *nextevent;

  /* If nonzero pointer to area allocated for the stack and its
     size.  */
  void *stackblock;
  size_t stackblock_size;
  /* Size of the included guard area.  */
  size_t guardsize;
} __attribute ((aligned (TCB_ALIGNMENT)));


#endif	/* descr.h */
