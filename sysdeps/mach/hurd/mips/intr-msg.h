/* Machine-dependent details of interruptible RPC messaging.  Mips version.
   Copyright (C) 1996, 1997 Free Software Foundation, Inc.
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


#ifdef __mips64
#define INTR_MSG_TRAP(msg, option, send_size, rcv_size, rcv_name, timeout, notify) \
({									      \
  error_t err;								      \
  mach_port_t __rcv_name = (rcv_name);					      \
  mach_msg_timeout_t __timeout = (timeout);	       			      \
  mach_port_t __notify = (notify);					      \
  asm (".globl _hurd_intr_rpc_msg_do_trap\n" 				      \
       ".globl _hurd_intr_rpc_msg_in_trap\n"				      \
       "				move $4, %1\n"			      \
       "				move $5, %2\n"			      \
       "				move $6, %3\n"			      \
       "				move $7, %4\n"			      \
       "				move $8, %5\n"			      \
       "				move $9, %6\n"			      \
       "				move $10, %7\n"			      \
       "				dli $2, -25\n"			      \
       "_hurd_intr_rpc_msg_do_trap:	syscall\n"			      \
       "_hurd_intr_rpc_msg_in_trap:	move %0, $2\n"			      \
       : "=r" (err)							      \
       : "r" (msg), "r" (option), "r" (send_size), "r" (rcv_size),	      \
         "r" (__rcv_name), "r" (__timeout), "r" (__notify)		      \
       : "$1", "$2", "$3", "$4", "$5", "$6", "$7", "$8", "$9", "$10",	      \
         "$11", "$12", "$13", "$14", "$15", "$24", "$25", "$28");	      \
  err;									      \
})
#else
#define INTR_MSG_TRAP(msg, option, send_size, rcv_size, rcv_name, timeout, notify) \
({									      \
  error_t err;								      \
  mach_port_t __rcv_name = (rcv_name);					      \
  mach_msg_timeout_t __timeout = (timeout);	       			      \
  mach_port_t __notify = (notify);					      \
  asm (".globl _hurd_intr_rpc_msg_do_trap\n" 				      \
       ".globl _hurd_intr_rpc_msg_in_trap\n"				      \
       "				move $4, %1\n"			      \
       "				move $5, %2\n"			      \
       "				move $6, %3\n"			      \
       "				move $7, %4\n"			      \
       "				move $8, %5\n"			      \
       "				move $9, %6\n"			      \
       "				move $10, %7\n"			      \
       "				li $2, -25\n"			      \
       "_hurd_intr_rpc_msg_do_trap:	syscall\n"			      \
       "_hurd_intr_rpc_msg_in_trap:	move %0, $2\n"			      \
       : "=r" (err)							      \
       : "r" (msg), "r" (option), "r" (send_size), "r" (rcv_size),	      \
         "r" (__rcv_name), "r" (__timeout), "r" (__notify)		      \
       : "$1", "$2", "$3", "$4", "$5", "$6", "$7", "$8", "$9", "$10",	      \
         "$11", "$12", "$13", "$14", "$15", "$24", "$25", "$28");	      \
  err;									      \
})
#endif

static inline void
INTR_MSG_BACK_OUT (struct mips_thread_state *state)
{
  return;
}

#include "hurdfault.h"

static inline int
SYSCALL_EXAMINE (struct mips_thread_state *state, int *callno)
{
  u_int32_t *p = (void *) (state->pc - 4);
  int result;
  if (_hurdsig_catch_memory_fault (p))
    return 0;
  if (result = (*p == 0x0000000c))
    /* The PC is just after a `syscall' instruction.
       This is a system call in progress; v0($2) holds the call number.  */
    *callno = state->r2;
  _hurdsig_end_catch_fault ();
  return result;
}


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


static inline mach_port_t
MSG_EXAMINE (struct mips_thread_state *state, int *msgid)
{
  mach_msg_header_t *msg;
  mach_port_t send_port;

  msg = (mach_msg_header_t *) state->r4;

  if (_hurdsig_catch_memory_fault (msg))
    return MACH_PORT_NULL;
  send_port = msg->msgh_remote_port;
  *msgid = msg->msgh_id;
  _hurdsig_end_catch_fault ();

  return send_port;
}
