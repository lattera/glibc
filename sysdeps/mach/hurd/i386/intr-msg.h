/* Machine-dependent details of interruptible RPC messaging.  i386 version.
Copyright (C) 1995 Free Software Foundation, Inc.
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


#define INTR_MSG_TRAP(msg, option, send_size, rcv_size, rcv_name, timeout, notify) \
({									      \
  error_t err;								      \
  asm (".globl _hurd_intr_rpc_msg_do_trap\n" 				      \
       ".globl _hurd_intr_rpc_msg_in_trap\n"				      \
       ".globl _hurd_intr_rpc_msg_cx_sp\n"				      \
       ".globl _hurd_intr_rpc_msg_sp_restored\n"			      \
       "				movl %%esp, %%ecx\n"		      \
       "				leal %1, %%esp\n"		      \
       "_hurd_intr_rpc_msg_cx_sp:	movl $-25, %%eax\n"		      \
       "_hurd_intr_rpc_msg_do_trap:	lcall $7, $0 # status in %0\n"	      \
       "_hurd_intr_rpc_msg_in_trap:	movl %%ecx, %%esp\n"		      \
       "_hurd_intr_rpc_msg_sp_restored:"				      \
       : "=a" (err) : "m" ((&msg)[-1]) : "%ecx");			      \
  err;									      \
})


static void inline
INTR_MSG_BACK_OUT (struct i386_thread_state *state)
{
  extern const void _hurd_intr_rpc_msg_cx_sp;
  if (state->eip >= (natural_t) &_hurd_intr_rpc_msg_cx_sp)
    state->uesp = state->ecx;
  else
    state->ecx = state->uesp;
}
