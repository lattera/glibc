/* Copyright (C) 1991, 92, 93, 94, 95, 96 Free Software Foundation, Inc.
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
#include <assert.h>

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
  mach_port_t portset;
  int got;
  error_t err;
  fd_set rfds, wfds, xfds;
  int firstfd, lastfd;
  mach_msg_timeout_t to = (timeout != NULL ?
			   (timeout->tv_sec * 1000 +
			    timeout->tv_usec / 1000) :
			   0);
  struct
    {
      struct hurd_userlink ulink;
      struct hurd_fd *cell;
      mach_port_t io_port;
      int type;
      mach_port_t reply_port;
    } d[nfds];

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
      d[i].type = type;
      if (type)
	{
	  d[i].cell = _hurd_dtable[i];
	  d[i].io_port = _hurd_port_get (&d[i].cell->port, &d[i].ulink);
	  if (d[i].io_port == MACH_PORT_NULL)
	    {
	      /* If one descriptor is bogus, we fail completely.  */
	      while (i-- > 0)
		_hurd_port_free (&d[i].cell->port, &d[i].ulink, d[i].io_port);
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

  /* Send them all io_select request messages.  */
  err = 0;
  got = 0;
  portset = MACH_PORT_NULL;
  for (i = firstfd; i <= lastfd; ++i)
    if (d[i].type)
      {
	int type = d[i].type;
	d[i].reply_port = __mach_reply_port ();
	err = __io_select (d[i].io_port, d[i].reply_port,
			   /* Poll only when there's a single descriptor.  */
			   (firstfd == lastfd) ? to : 0,
			   &type);
	switch (err)
	  {
	  case MACH_RCV_TIMED_OUT:
	    /* No immediate response.  This is normal.  */
	    err = 0;
	    if (firstfd == lastfd)
	      /* When there's a single descriptor, we don't need a portset,
		 so just pretend we have one, but really use the single reply
		 port.  */
	      portset = d[i].reply_port;
	    else if (got == 0)
	      /* We've got multiple reply ports, so we need a port set to
		 multiplex them.  */
	      {
		/* We will wait again for a reply later.  */
		if (portset == MACH_PORT_NULL)
		  /* Create the portset to receive all the replies on.  */
		  err = __mach_port_allocate (__mach_task_self (),
					      MACH_PORT_RIGHT_PORT_SET,
					      &portset);
		if (! err)
		  /* Put this reply port in the port set.  */
		  __mach_port_move_member (__mach_task_self (),
					   d[i].reply_port, portset);
	      }
	    break;

	  default:
	    /* No other error should happen.  Callers of select don't
	       expect to see errors, so we simulate readiness of the erring
	       object and the next call hopefully will get the error again.  */
	    type = SELECT_ALL;
	    /* FALLTHROUGH */

	  case 0:
	    /* We got an answer.  */
	    if ((type & SELECT_ALL) == 0)
	      /* Bogus answer; treat like an error, as a fake positive.  */
	      type = SELECT_ALL;

	    /* This port is already ready already.  */
	    d[i].type &= type;
	    d[i].type |= SELECT_RETURNED;
	    ++got;
	    break;
	  }
	_hurd_port_free (&d[i].cell->port, &d[i].ulink, d[i].io_port);
      }

  /* Now wait for reply messages.  */
  if (!err && got == 0)
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
	    } success;
	} msg;
      mach_msg_option_t options = (timeout == NULL ? 0 : MACH_RCV_TIMEOUT);
      error_t msgerr;
      while ((msgerr = __mach_msg (&msg.head,
				   MACH_RCV_MSG | options,
				   0, sizeof msg, portset, to,
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
	      if (msg.error.err == EINTR &&
		  msg.head.msgh_size == sizeof msg.error)
		{
		  /* EINTR response; poll for further responses
		     and then return quickly.  */
		  err = EINTR;
		  goto poll;
		}
	      if (msg.error.err ||
		  msg.head.msgh_size != sizeof msg.success ||
		  *(int *) &msg.success.result_type != *(int *) &inttype ||
		  (msg.success.result & SELECT_ALL) == 0)
		{
		  /* Error or bogus reply.  Simulate readiness.  */
		  __mach_msg_destroy (&msg);
		  msg.success.result = SELECT_ALL;
		}

	      /* Look up the respondant's reply port and record its
                 readiness.  */
	      {
		int had = got;
		for (i = firstfd; i <= lastfd; ++i)
		  if (d[i].type && d[i].reply_port == msg.head.msgh_local_port)
		    {
		      d[i].type &= msg.success.result;
		      d[i].type |= SELECT_RETURNED;
		      ++got;
		    }
		assert (got > had);
	      }
	    }

	  if (msg.head.msgh_remote_port != MACH_PORT_NULL)
	    __mach_port_deallocate (__mach_task_self (),
				    msg.head.msgh_remote_port);

	  if (got)
	  poll:
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

      if (got)
	/* At least one descriptor is known to be ready now, so we will
	   return success.  */
	err = 0;
    }

  for (i = firstfd; i <= lastfd; ++i)
    if (d[i].type)
      __mach_port_destroy (__mach_task_self (), d[i].reply_port);
  if (firstfd != lastfd && portset != MACH_PORT_NULL)
    /* Destroy PORTSET, but only if it's not actually the reply port for a
       single descriptor (in which case it's destroyed in the previous loop;
       not doing it here is just a bit more efficient).  */
    __mach_port_destroy (__mach_task_self (), portset);

  if (err)
    return __hurd_fail (err);

  /* Below we recalculate GOT to include an increment for each operation
     allowed on each fd.  */
  got = 0;

  /* Set the user bitarrays.  We only ever have to clear bits, as all desired
     ones are initially set.  */
  for (i = firstfd; i <= lastfd; ++i)
    {
      int type = d[i].type;

      if ((type & SELECT_RETURNED) == 0)
	type = 0;

      if (type & SELECT_READ)
	got++;
      else if (readfds)
	FD_CLR (i, readfds);
      if (type & SELECT_WRITE)
	got++;
      else if (writefds)
	FD_CLR (i, writefds);
      if (type & SELECT_URG)
	got++;
      else if (exceptfds)
	FD_CLR (i, exceptfds);
    }

  return got;
}

weak_alias (__select, select)
