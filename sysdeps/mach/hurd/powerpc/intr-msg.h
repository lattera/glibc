/* Machine-dependent details of interruptible RPC messaging.  PowerPC version.
   Copyright (C) 1995,96,97,99,2001 Free Software Foundation, Inc.
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

#define INTR_MSG_TRAP(msg, option, send_size, rcv_size, rcv_name, timeout, notify) \
({									\
  error_t err;								\
  asm (".globl _hurd_intr_rpc_msg_do_trap\n"				\
       ".globl _hurd_intr_rpc_msg_in_trap\n"				\
       "				mr 3, %1\n"			\
       "				mr 4, %2\n"			\
       "				mr 5, %3\n"			\
       "				mr 6, %4\n"			\
       "				mr 7, %5\n"			\
       "				mr 8, %6\n"			\
       "				mr 9, %7\n"			\
       "				li 0, -25\n"			\
       "_hurd_intr_rpc_msg_do_trap:	sc\n"				\
       "_hurd_intr_rpc_msg_in_trap:	mr 3, %0\n"			\
       : "=r" (err)							\
       : "r" (msg), "r" (option), "r" (send_size), "r" (rcv_size),	\
	 "r" (rcv_name), "r" (timeout), "r" (notify)			\
       : "0", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12");	\
  err;									\
})

static void inline
INTR_MSG_BACK_OUT (struct ppc_thread_state *state)
{
  return;
}

#include "hurdfault.h"

/* This cannot be an inline function because it calls setjmp.  */
#define SYSCALL_EXAMINE(state, callno)					    \
({									    \
  u_int32_t *p = (void *) ((state)->srr0 - 4);				    \
  int result;								    \
  _hurdsig_catch_memory_fault (p) ? 0 :					    \
  ({									    \
    if (result = (*p == 0x44000002))					    \
      /* The PC is just after an `sc' instruction.			    \
         This is a system call in progress; %r0 holds the call number.  */  \
      *(callno) = (state)->r0;						    \
    _hurdsig_end_catch_fault ();					    \
    result;								    \
  });									    \
})

struct mach_msg_trap_args
  {
    /* This is the order of arguments to mach_msg_trap.  */
    mach_msg_header_t *msg;
    mach_msg_option_t option;
    mach_msg_size_t send_size;
    mach_msg_size_t rcv_size;
    mach_port_t rcv_name;
    mach_msg_timeout_t timeout;
    mach_port_t notify;
  };

/* This cannot be an inline function because it calls setjmp.  */
#define MSG_EXAMINE(state, msgid, rcv_name, send_name, option, timeout)   \
({									  \
  mach_msg_header_t *msg = (mach_msg_header_t *) (state)->r3;		  \
  *(option) = (mach_msg_option_t) (state)->r4;				  \
  *(rcv_name) = (mach_port_t) (state)->r7;				  \
  *(timeout) = (mach_msg_timeout_t) (state)->r8;			  \
  (msg == 0) ?								  \
    ({									  \
      *(send_name) = MACH_PORT_NULL;					  \
      *(msgid) = 0;							  \
      0;								  \
    }) :								  \
    (_hurdsig_catch_memory_fault (msg) ? -1 :				  \
	({								  \
	  *(send_name) = msg->msgh_remote_port;				  \
	  *(msgid) = msg->msgh_id;					  \
	  _hurdsig_end_catch_fault ();					  \
	  0;								  \
	})								  \
    );									  \
})
