/* Copyright (C) 1991, 1992, 1993, 1994, 1995, 1996 Free Software Foundation, Inc.
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

#include <ansidecl.h>
#include <sys/types.h>
#include <hurd.h>
#include <hurd/fd.h>
#include <stdlib.h>
#include <string.h>

/* All user select types.  */
#define SELECT_ALL (SELECT_READ | SELECT_WRITE | SELECT_URG)

/* Used to record that a particular select rpc returned. Must be distinct
   from SELECT_ALL (which better not have the high bit set).  */
#define SELECT_RETURNED ((SELECT_ALL << 1) & ~SELECT_ALL)

/* Check the first NFDS descriptors each in READFDS (if not NULL) for read
   readiness, in WRITEFDS (if not NULL) for write readiness, and in EXCEPTFDS
   (if not NULL) for exceptional conditions.  If TIMEOUT is not NULL, time out
   after waiting the interval specified therein.  Returns the number of ready
   descriptors, or -1 for errors.  */
int
DEFUN(__select, (nfds, readfds, writefds, exceptfds, timeout),
      int nfds AND fd_set *readfds AND fd_set *writefds AND
      fd_set *exceptfds AND struct timeval *timeout)
{
  int i;
  mach_port_t port;
  int got;
  int *types;
  struct hurd_userlink *ulink;
  mach_port_t *ports;
  struct hurd_fd **cells;
  error_t err;
  fd_set rfds, wfds, xfds;
  int firstfd, lastfd;
  mach_msg_timeout_t to = (timeout != NULL ?
			   (timeout->tv_sec * 1000 +
			    timeout->tv_usec / 1000) :
			   0);

  /* Use local copies so we can't crash from user bogosity.  */
  if (readfds == NULL)
    FD_ZERO (&rfds);
  else
    rfds = *readfds;
  if (writefds == NULL)
    FD_ZERO (&wfds);
  else
    wfds = *writefds;
  if (exceptfds == NULL)
    FD_ZERO (&xfds);
  else
    xfds = *exceptfds;

  HURD_CRITICAL_BEGIN;
  __mutex_lock (&_hurd_dtable_lock);

  if (nfds > _hurd_dtablesize)
    nfds = _hurd_dtablesize;

  /* Collect the ports for interesting FDs.  */
  cells = __alloca (nfds * sizeof (*cells));
  ports = __alloca (nfds * sizeof (*ports));
  types = __alloca (nfds * sizeof (*types));
  ulink = __alloca (nfds * sizeof (*ulink));
  firstfd = lastfd = -1;
  for (i = 0; i < nfds; ++i)
    {
      int type = 0;
      if (readfds != NULL && FD_ISSET (i, &rfds))
	type |= SELECT_READ;
      if (writefds != NULL && FD_ISSET (i, &wfds))
	type |= SELECT_WRITE;
      if (exceptfds != NULL && FD_ISSET (i, &xfds))
	type |= SELECT_URG;
      types[i] = type;
      if (type)
	{
	  cells[i] = _hurd_dtable[i];
	  ports[i] = _hurd_port_get (&cells[i]->port, &ulink[i]);
	  if (ports[i] == MACH_PORT_NULL)
	    {
	      /* If one descriptor is bogus, we fail completely.  */
	      while (i-- > 0)
		_hurd_port_free (&cells[i]->port, &ulink[i], ports[i]);
	      errno = EBADF;
	      break;
	    }
	  lastfd = i;
	  if (firstfd == -1)
	    firstfd = i;
	}
    }

  __mutex_unlock (&_hurd_dtable_lock);
  HURD_CRITICAL_END;

  if (i < nfds)
    return -1;

  /* Get a port to receive the io_select_reply messages on.  */
  port = __mach_reply_port ();

  /* Send them all io_select request messages.  */
  got = 0;
  err = 0;
  for (i = firstfd; i <= lastfd; ++i)
    if (types[i])
      {
	if (!err)
	  {
	    int tag = i;
	    int type = types[i];
	    err = __io_select (ports[i], port,
			       /* Poll for each but the last.  */
			       (i == lastfd && got == 0) ? to : 0,
			       &type, &tag);
	    switch (err)
	      {
	      case MACH_RCV_TIMED_OUT:
		/* No immediate response.  This is normal.  */
		err = 0;
		break;

	      case 0:
		/* We got an answer.  This is not necessarily the answer to
                   the query we sent just now.  It may correspond to any
                   prior query which timed out before its answer arrived.  */
		if (tag < 0 || tag > i || (type & SELECT_ALL) == 0)
		  /* This is not a proper answer to any query we have yet
                     made.  */
		  err = EGRATUITOUS;
		else
		  {
		    /* Some port is ready.  TAG tells us which.  */
		    types[tag] &= type;
		    types[tag] |= SELECT_RETURNED;
		    ++got;
		  }
		break;

	      default:
		/* Any other error kills us.
		   But we must continue to loop to free the ports.  */
		break;
	      }
	  }
	_hurd_port_free (&cells[i]->port, &ulink[i], ports[i]);
      }

  /* Now wait for reply messages.  */
  if (!err && got == 0 && port != MACH_PORT_NULL)
    {
      /* Now wait for io_select_reply messages on PORT,
	 timing out as appropriate.  */

      union
	{
	  mach_msg_header_t head;
	  struct
	    {
	      mach_msg_header_t head;
	      mach_msg_type_t err_type;
	      error_t err;
	    } error;
	  struct
	    {
	      mach_msg_header_t head;
	      mach_msg_type_t err_type;
	      error_t err;
	      mach_msg_type_t result_type;
	      int result;
	      mach_msg_type_t tag_type;
	      int tag;
	    } success;
	} msg;
      mach_msg_option_t options = (timeout == NULL ? 0 : MACH_RCV_TIMEOUT);
      error_t msgerr;
      while ((msgerr = __mach_msg (&msg.head,
				   MACH_RCV_MSG | options,
				   0, sizeof msg, port, to,
				   MACH_PORT_NULL)) == MACH_MSG_SUCCESS)
	{
	  /* We got a message.  Decode it.  */
#define IO_SELECT_REPLY_MSGID (21012 + 100) /* XXX */
	  const mach_msg_type_t inttype =
	    { MACH_MSG_TYPE_INTEGER_32, 32, 1, 1, 0, 0 };
	  if (msg.head.msgh_id == IO_SELECT_REPLY_MSGID &&
	      msg.head.msgh_size >= sizeof msg.error &&
	      !(msg.head.msgh_bits & MACH_MSGH_BITS_COMPLEX) &&
	      *(int *) &msg.error.err_type == *(int *) &inttype)
	    {
	      /* This is a properly formatted message so far.
		 See if it is a success or a failure.  */
	      if (msg.error.err)
		{
		  err = msg.error.err;
		  if (msg.head.msgh_size != sizeof msg.error)
		    __mach_msg_destroy (&msg);
		}
	      else if (msg.head.msgh_size != sizeof msg.success ||
		       *(int *) &msg.success.tag_type != *(int *) &inttype ||
		       *(int *) &msg.success.result_type != *(int *) &inttype)
		__mach_msg_destroy (&msg);
	      else if ((msg.success.result & SELECT_ALL) == 0 ||
		       msg.success.tag < firstfd || msg.success.tag > lastfd)
		err = EGRATUITOUS;
	      else
		{
		  /* This is a winning io_select_reply message!
		     Record the readiness it indicates and send a reply.  */
		  types[msg.success.tag] &= msg.success.result;
		  types[msg.success.tag] |= SELECT_RETURNED;
		  ++got;
		}
	    }

	  if (msg.head.msgh_remote_port != MACH_PORT_NULL)
	    __mach_port_deallocate (__mach_task_self (),
				    msg.head.msgh_remote_port);

	  if (got || err == EINTR)
	    {
	      /* Poll for another message.  */
	      to = 0;
	      options |= MACH_RCV_TIMEOUT;
	    }
	}

    if (err == MACH_RCV_TIMED_OUT)
      /* This is the normal value for ERR.  We might have timed out and
         read no messages.  Otherwise, after receiving the first message,
         we poll for more messages.  We receive with a timeout of 0 to
         effect a poll, so ERR is MACH_RCV_TIMED_OUT when the poll finds no
         message waiting.  */
      err = 0;

      if (got && err == EINTR)
	/* Some calls were interrupted, but at least one descriptor
	   is known to be ready now, so we will return success.  */
	err = 0;
    }

  if (port != MACH_PORT_NULL)
    /* We must destroy the port if we made some select requests
       that might send notification on that port after we no longer care.
       If the port were reused, that notification could confuse the next
       select call to use the port.  The notification might be valid,
       but the descriptor may have changed to a different server.  */
    __mach_port_destroy (__mach_task_self (), port);

  if (timeout && got == 0 && err == MACH_RCV_TIMED_OUT)
    /* No io_select call returned success immediately, and the last call
       blocked for our full timeout period and then timed out.  So the
       multiplex times out too.  */
    return 0;

  if (err)
    return __hurd_fail (err);

  /* Below we recalculate GOT to include an increment for each operation
     allowed on each fd.  */
  got = 0;

  /* Set the user bitarrays.  We only ever have to clear bits, as all desired
     ones are initially set.  */
  for (i = 0; i < nfds; ++i)
    {
      int type = types[i];

      if ((type & SELECT_RETURNED) == 0)
	type = 0;

      if (readfds != NULL && (type & SELECT_READ) == 0)
	FD_CLR (i, readfds);
      else
	got++;
      if (writefds != NULL && (type & SELECT_WRITE) == 0)
	FD_CLR (i, writefds);
      else
	got++;
      if (exceptfds != NULL && (type & SELECT_URG) == 0)
	FD_CLR (i, exceptfds);
      else
	got++;
    }

  return got;
}

weak_alias (__select, select)
