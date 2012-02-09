/* Guts of both `select' and `poll' for Hurd.
   Copyright (C) 1991,92,93,94,95,96,97,98,99,2001
   	Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <sys/types.h>
#include <sys/poll.h>
#include <hurd.h>
#include <hurd/fd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

/* All user select types.  */
#define SELECT_ALL (SELECT_READ | SELECT_WRITE | SELECT_URG)

/* Used to record that a particular select rpc returned.  Must be distinct
   from SELECT_ALL (which better not have the high bit set).  */
#define SELECT_RETURNED ((SELECT_ALL << 1) & ~SELECT_ALL)

/* Check the first NFDS descriptors either in POLLFDS (if nonnnull) or in
   each of READFDS, WRITEFDS, EXCEPTFDS that is nonnull.  If TIMEOUT is not
   NULL, time out after waiting the interval specified therein.  Returns
   the number of ready descriptors, or -1 for errors.  */
int
_hurd_select (int nfds,
	      struct pollfd *pollfds,
	      fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
	      const struct timespec *timeout, const sigset_t *sigmask)
{
  int i;
  mach_port_t portset;
  int got;
  error_t err;
  fd_set rfds, wfds, xfds;
  int firstfd, lastfd;
  mach_msg_timeout_t to = (timeout != NULL ?
			   (timeout->tv_sec * 1000 +
			    (timeout->tv_nsec + 999999) / 1000000) :
			   0);
  struct
    {
      struct hurd_userlink ulink;
      struct hurd_fd *cell;
      mach_port_t io_port;
      int type;
      mach_port_t reply_port;
    } d[nfds];
  sigset_t oset;

  union typeword		/* Use this to avoid unkosher casts.  */
    {
      mach_msg_type_t type;
      uint32_t word;
    };
  assert (sizeof (union typeword) == sizeof (mach_msg_type_t));
  assert (sizeof (uint32_t) == sizeof (mach_msg_type_t));

  if (sigmask && __sigprocmask (SIG_SETMASK, sigmask, &oset))
    return -1;

  if (pollfds)
    {
      /* Collect interesting descriptors from the user's `pollfd' array.
	 We do a first pass that reads the user's array before taking
	 any locks.  The second pass then only touches our own stack,
	 and gets the port references.  */

      for (i = 0; i < nfds; ++i)
	if (pollfds[i].fd >= 0)
	  {
	    int type = 0;
	    if (pollfds[i].events & POLLIN)
	      type |= SELECT_READ;
	    if (pollfds[i].events & POLLOUT)
	      type |= SELECT_WRITE;
	    if (pollfds[i].events & POLLPRI)
	      type |= SELECT_URG;

	    d[i].io_port = pollfds[i].fd;
	    d[i].type = type;
	  }
	else
	  d[i].type = 0;

      HURD_CRITICAL_BEGIN;
      __mutex_lock (&_hurd_dtable_lock);

      for (i = 0; i < nfds; ++i)
	if (d[i].type != 0)
	  {
	    const int fd = (int) d[i].io_port;

	    if (fd < _hurd_dtablesize)
	      {
		d[i].cell = _hurd_dtable[fd];
		d[i].io_port = _hurd_port_get (&d[i].cell->port, &d[i].ulink);
		if (d[i].io_port != MACH_PORT_NULL)
		  continue;
	      }

	    /* If one descriptor is bogus, we fail completely.  */
	    while (i-- > 0)
	      if (d[i].type != 0)
		_hurd_port_free (&d[i].cell->port,
				 &d[i].ulink, d[i].io_port);
	    break;
	  }

      __mutex_unlock (&_hurd_dtable_lock);
      HURD_CRITICAL_END;

      if (i < nfds)
	{
	  if (sigmask)
	    __sigprocmask (SIG_SETMASK, &oset, NULL);
	  errno = EBADF;
	  return -1;
	}

      lastfd = i - 1;
      firstfd = i == 0 ? lastfd : 0;
    }
  else
    {
      /* Collect interested descriptors from the user's fd_set arguments.
	 Use local copies so we can't crash from user bogosity.  */

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
		    if (d[i].type != 0)
		      _hurd_port_free (&d[i].cell->port, &d[i].ulink,
				       d[i].io_port);
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
	{
	  if (sigmask)
	    __sigprocmask (SIG_SETMASK, &oset, NULL);
	  errno = EBADF;
	  return -1;
	}
    }


  err = 0;
  got = 0;

  /* Send them all io_select request messages.  */

  if (firstfd == -1)
    /* But not if there were no ports to deal with at all.
       We are just a pure timeout.  */
    portset = __mach_reply_port ();
  else
    {
      portset = MACH_PORT_NULL;

      for (i = firstfd; i <= lastfd; ++i)
	if (d[i].type)
	  {
	    int type = d[i].type;
	    d[i].reply_port = __mach_reply_port ();
	    err = __io_select (d[i].io_port, d[i].reply_port,
			       /* Poll only if there's a single descriptor.  */
			       (firstfd == lastfd) ? to : 0,
			       &type);
	    switch (err)
	      {
	      case MACH_RCV_TIMED_OUT:
		/* No immediate response.  This is normal.  */
		err = 0;
		if (firstfd == lastfd)
		  /* When there's a single descriptor, we don't need a
		     portset, so just pretend we have one, but really
		     use the single reply port.  */
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
		/* No other error should happen.  Callers of select
		   don't expect to see errors, so we simulate
		   readiness of the erring object and the next call
		   hopefully will get the error again.  */
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
    }

  /* Now wait for reply messages.  */
  if (!err && got == 0)
    {
      /* Now wait for io_select_reply messages on PORT,
	 timing out as appropriate.  */

      union
	{
	  mach_msg_header_t head;
#ifdef MACH_MSG_TRAILER_MINIMUM_SIZE
	  struct
	    {
	      mach_msg_header_t head;
	      NDR_record_t ndr;
	      error_t err;
	    } error;
	  struct
	    {
	      mach_msg_header_t head;
	      NDR_record_t ndr;
	      error_t err;
	      int result;
	      mach_msg_trailer_t trailer;
	    } success;
#else
	  struct
	    {
	      mach_msg_header_t head;
	      union typeword err_type;
	      error_t err;
	    } error;
	  struct
	    {
	      mach_msg_header_t head;
	      union typeword err_type;
	      error_t err;
	      union typeword result_type;
	      int result;
	    } success;
#endif
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
#ifdef MACH_MSG_TYPE_BIT
	  const union typeword inttype =
	  { type:
	    { MACH_MSG_TYPE_INTEGER_T, sizeof (integer_t) * 8, 1, 1, 0, 0 }
	  };
#endif
	  if (msg.head.msgh_id == IO_SELECT_REPLY_MSGID &&
	      msg.head.msgh_size >= sizeof msg.error &&
	      !(msg.head.msgh_bits & MACH_MSGH_BITS_COMPLEX) &&
#ifdef MACH_MSG_TYPE_BIT
	      msg.error.err_type.word == inttype.word
#endif
	      )
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
#ifdef MACH_MSG_TYPE_BIT
		  msg.success.result_type.word != inttype.word ||
#endif
		  (msg.success.result & SELECT_ALL) == 0)
		{
		  /* Error or bogus reply.  Simulate readiness.  */
		  __mach_msg_destroy (&msg.head);
		  msg.success.result = SELECT_ALL;
		}

	      /* Look up the respondent's reply port and record its
                 readiness.  */
	      {
		int had = got;
		if (firstfd != -1)
		  for (i = firstfd; i <= lastfd; ++i)
		    if (d[i].type
			&& d[i].reply_port == msg.head.msgh_local_port)
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

  if (firstfd != -1)
    for (i = firstfd; i <= lastfd; ++i)
      if (d[i].type)
	__mach_port_destroy (__mach_task_self (), d[i].reply_port);
  if (firstfd == -1 || (firstfd != lastfd && portset != MACH_PORT_NULL))
    /* Destroy PORTSET, but only if it's not actually the reply port for a
       single descriptor (in which case it's destroyed in the previous loop;
       not doing it here is just a bit more efficient).  */
    __mach_port_destroy (__mach_task_self (), portset);

  if (err)
    {
      if (sigmask)
	__sigprocmask (SIG_SETMASK, &oset, NULL);
      return __hurd_fail (err);
    }

  if (pollfds)
    /* Fill in the `revents' members of the user's array.  */
    for (i = 0; i < nfds; ++i)
      {
	int type = d[i].type;
	int_fast16_t revents = 0;

	if (type & SELECT_RETURNED)
	  {
	    if (type & SELECT_READ)
	      revents |= POLLIN;
	    if (type & SELECT_WRITE)
	      revents |= POLLOUT;
	    if (type & SELECT_URG)
	      revents |= POLLPRI;
	  }

	pollfds[i].revents = revents;
      }
  else
    {
      /* Below we recalculate GOT to include an increment for each operation
	 allowed on each fd.  */
      got = 0;

      /* Set the user bitarrays.  We only ever have to clear bits, as all
	 desired ones are initially set.  */
      if (firstfd != -1)
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
    }

  if (sigmask && __sigprocmask (SIG_SETMASK, &oset, NULL))
    return -1;

  return got;
}
