/* Replacement for mach_msg used in interruptible Hurd RPCs.
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

#include <mach.h>
#include <mach/mig_errors.h>
#include <hurd/signal.h>

#include "intr-msg.h"


error_t
_hurd_intr_rpc_mach_msg (mach_msg_header_t *msg,
			 mach_msg_option_t option,
			 mach_msg_size_t send_size,
			 mach_msg_size_t rcv_size,
			 mach_port_t rcv_name,
			 mach_msg_timeout_t timeout,
			 mach_port_t notify)
{
  struct hurd_sigstate *ss = _hurd_self_sigstate ();
  error_t err;

  /* Tell the signal thread that we are doing an interruptible RPC on
     this port.  If we get a signal and should return EINTR, the signal
     thread will set this variable to MACH_PORT_NULL.  The RPC might
     return EINTR when some other thread gets a signal, in which case we
     want to restart our call.  */
  ss->intr_port = msg->msgh_remote_port;
  
  /* A signal may arrive here, after intr_port is set, but before
     the mach_msg system call.  The signal handler might do an
     interruptible RPC, and clobber intr_port; then it would not be
     set properly when we actually did send the RPC, and a later
     signal wouldn't interrupt that RPC.  So,
     _hurd_setup_sighandler saves intr_port in the sigcontext, and
     sigreturn restores it.  */

 message:

  if (ss->cancel)
    {
      err = EINTR;
      ss->cancel = 0;
    }
  else 
    err = INTR_MSG_TRAP (msg, option, send_size,
			 rcv_size, rcv_name, timeout, notify);

  switch (err)
    {
    case MACH_SEND_INTERRUPTED: /* RPC didn't get out.  */
      if (ss->intr_port != MACH_PORT_NULL)
	/* If this signal was for us and it should interrupt calls, the
	   signal thread will have cleared SS->intr_port.
	   Since it's not cleared, the signal was for another thread,
	   or SA_RESTART is set.  Restart the interrupted call.  */
	goto message;
      /* FALLTHROUGH */

    case MACH_RCV_PORT_DIED:
      /* Server didn't respond to interrupt_operation,
	 so the signal thread destroyed the reply port.  */
      /* FALLTHROUGH */

    case MACH_RCV_TIMED_OUT:
      /* The operation was supposedly interrupted, but still has
	 not returned.  Declare it interrupted.  */

      err = EINTR;

      /* The EINTR return indicates cancellation, so clear the flag.  */
      ss->cancel = 0;
      break;

    case MACH_RCV_INTERRUPTED:	/* RPC sent; no reply.  */
      option &= ~MACH_SEND_MSG;	/* Don't send again.  */
      if (ss->intr_port == MACH_PORT_NULL)
	{
	  /* This signal or cancellation was for us.  We need to wait for
             the reply, but not hang forever.  */
	  option |= MACH_RCV_TIMEOUT;
	  timeout = _hurd_interrupted_rpc_timeout;
	}
      goto message;		/* Retry the receive.  */

    case MACH_MSG_SUCCESS:
      if (option & MACH_RCV_MSG)
	{
	  /* We got a reply.  Was it EINTR?  */
	  mig_reply_header_t *const reply = (void *) msg;
	  const union
	    {
	      mach_msg_type_t t;
	      int i;
	    } check =
	      { t: {
		MACH_MSG_TYPE_INTEGER_32,
		32,
		1,
		TRUE,
		FALSE,
		FALSE,
		0
	      } };
	  if (msg->msgh_size == sizeof *reply &&
	      !(msg->msgh_bits & MACH_MSGH_BITS_COMPLEX) &&
	      *(int *) &reply->RetCodeType == check.i &&
	      reply->RetCode == EINTR)
	    {
	      /* It is indeed EINTR.  Is the interrupt for us?  */
	      if (ss->intr_port != MACH_PORT_NULL)
		/* Nope; repeat the RPC.
		   XXX Resources moved? */
		goto message;
	      else
		/* The EINTR return indicates cancellation, so clear the
                   flag.  */
		ss->cancel = 0;
	    }
	}
      break;

    default:			/* Quiet -Wswitch-enum.  */
    }

  ss->intr_port = MACH_PORT_NULL;

  return err;
}
