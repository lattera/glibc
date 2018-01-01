/* Copyright (C) 2001-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, see <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <string.h>
#include <sys/socket.h>

#include <hurd.h>
#include <hurd/fd.h>
#include <hurd/socket.h>

/* Receive a message as described by MESSAGE from socket FD.
   Returns the number of bytes read or -1 for errors.  */
ssize_t
__libc_recvmsg (int fd, struct msghdr *message, int flags)
{
  error_t err;
  addr_port_t aport;
  char *data = NULL;
  mach_msg_type_number_t len = 0;
  mach_port_t *ports;
  mach_msg_type_number_t nports = 0;
  char *cdata = NULL;
  mach_msg_type_number_t clen = 0;
  size_t amount;
  char *buf;
  int i;

  /* Find the total number of bytes to be read.  */
  amount = 0;
  for (i = 0; i < message->msg_iovlen; i++)
    {
      amount += message->msg_iov[i].iov_len;

      /* As an optimization, we set the initial values of DATA and LEN
         from the first non-empty iovec.  This kicks-in in the case
         where the whole packet fits into that iovec buffer.  */
      if (data == NULL && message->msg_iov[i].iov_len > 0)
	{
	  data = message->msg_iov[i].iov_base;
	  len = message->msg_iov[i].iov_len;
	}
    }

  buf = data;
  if (err = HURD_DPORT_USE (fd, __socket_recv (port, &aport,
					       flags, &data, &len,
					       &ports, &nports,
					       &cdata, &clen,
					       &message->msg_flags, amount)))
    return __hurd_sockfail (fd, flags, err);

  if (message->msg_name != NULL && aport != MACH_PORT_NULL)
    {
      char *buf = message->msg_name;
      mach_msg_type_number_t buflen = message->msg_namelen;
      int type;

      err = __socket_whatis_address (aport, &type, &buf, &buflen);
      if (err == EOPNOTSUPP)
	/* If the protocol server can't tell us the address, just return a
	   zero-length one.  */
	{
	  buf = message->msg_name;
	  buflen = 0;
	  err = 0;
	}

      if (err)
	{
	  __mach_port_deallocate (__mach_task_self (), aport);
	  return __hurd_sockfail (fd, flags, err);
	}

      if (message->msg_namelen > buflen)
	message->msg_namelen = buflen;

      if (buf != message->msg_name)
	{
	  memcpy (message->msg_name, buf, message->msg_namelen);
	  __vm_deallocate (__mach_task_self (), (vm_address_t) buf, buflen);
	}

      if (buflen > 0)
	((struct sockaddr *) message->msg_name)->sa_family = type;
    }
  else if (message->msg_name != NULL)
    message->msg_namelen = 0;

  __mach_port_deallocate (__mach_task_self (), aport);

  if (buf == data)
    buf += len;
  else
    {
      /* Copy the data into MSG.  */
      if (len > amount)
	message->msg_flags |= MSG_TRUNC;
      else
	amount = len;

      buf = data;
      for (i = 0; i < message->msg_iovlen; i++)
	{
#define min(a, b)	((a) > (b) ? (b) : (a))
	  size_t copy = min (message->msg_iov[i].iov_len, amount);

	  memcpy (message->msg_iov[i].iov_base, buf, copy);

	  buf += copy;
	  amount -= copy;
	  if (len == 0)
	    break;
	}

      __vm_deallocate (__mach_task_self (), (vm_address_t) data, len);
    }

  /* Copy the control message into MSG.  */
  if (clen > message->msg_controllen)
    message->msg_flags |= MSG_CTRUNC;
  else
    message->msg_controllen = clen;
  memcpy (message->msg_control, cdata, message->msg_controllen);

  __vm_deallocate (__mach_task_self (), (vm_address_t) cdata, clen);

  return (buf - data);
}

weak_alias (__libc_recvmsg, recvmsg)
weak_alias (__libc_recvmsg, __recvmsg)
