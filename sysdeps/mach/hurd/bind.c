/* Copyright (C) 1992,94,95,96,97,98,2001,02 Free Software Foundation, Inc.
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

#include <errno.h>
#include <sys/socket.h>
#include <hurd.h>
#include <hurd/fd.h>
#include <hurd/socket.h>
#include <hurd/paths.h>
#include <fcntl.h>
#include <stddef.h>
#include <hurd/ifsock.h>
#include <sys/un.h>
#include <string.h>

/* Give the socket FD the local address ADDR (which is LEN bytes long).  */
int
__bind  (int fd, __CONST_SOCKADDR_ARG addrarg, socklen_t len)
{
  addr_port_t aport;
  error_t err;
  const struct sockaddr_un *addr = addrarg.__sockaddr_un__;

  if (addr->sun_family == AF_LOCAL)
    {
      /* For the local domain, we must create a node in the filesystem
	 using the ifsock translator and then fetch the address from it.  */
      file_t dir, node;
      char name[len - offsetof (struct sockaddr_un, sun_path) + 1], *n;

      strncpy (name, addr->sun_path, sizeof name - 1);
      name[sizeof name - 1] = '\0'; /* Make sure */

      dir = __file_name_split (name, &n);
      if (dir == MACH_PORT_NULL)
	return -1;

      /* Create a new, unlinked node in the target directory.  */
      err = __dir_mkfile (dir, O_CREAT, 0666 & ~_hurd_umask, &node);

      if (! err)
	{
	  /* Set the node's translator to make it a local-domain socket.  */
	  err = __file_set_translator (node,
				       FS_TRANS_EXCL | FS_TRANS_SET,
				       FS_TRANS_EXCL | FS_TRANS_SET, 0,
				       _HURD_IFSOCK, sizeof _HURD_IFSOCK,
				       MACH_PORT_NULL,
				       MACH_MSG_TYPE_COPY_SEND);
	  if (! err)
	    {
	      /* Link the node, now a socket, into the target directory.  */
	      err = __dir_link (dir, node, n, 1);
	      if (err == EEXIST)
		err = EADDRINUSE;
	    }
	  __mach_port_deallocate (__mach_task_self (), node);
	  if (! err)
	    {
	      /* Get a port to the ifsock translator.  */
	      file_t ifsock = __file_name_lookup_under (dir, n, 0, 0);
	      if (ifsock == MACH_PORT_NULL)
		{
		  err = errno;
		  /* If we failed, get rid of the node we created.  */
		  __dir_unlink (dir, n);
		}
	      else
		{
		  /* Get the address port.  */
		  err = __ifsock_getsockaddr (ifsock, &aport);
		  if (err == MIG_BAD_ID || err == EOPNOTSUPP)
		    /* We are not talking to /hurd/ifsock.  Probably
		       someone came in after we linked our node, unlinked
		       it, and replaced it with a different node, before we
		       did our lookup.  Treat it as if our link had failed
		       with EEXIST.  */
		    err = EADDRINUSE;
		}
	      __mach_port_deallocate (__mach_task_self (), ifsock);
	    }
	}
      __mach_port_deallocate (__mach_task_self (), dir);

      if (err)
	return __hurd_fail (err);
    }
  else
    err = EIEIO;

  err = HURD_DPORT_USE (fd,
			({
			  if (err)
			    err = __socket_create_address (port,
							   addr->sun_family,
							   (char *) addr, len,
							   &aport);
			  if (! err)
			    {
			      err = __socket_bind (port, aport);
			      __mach_port_deallocate (__mach_task_self (),
						      aport);
			    }
			  err;
			}));

  return err ? __hurd_dfail (fd, err) : 0;
}

weak_alias (__bind, bind)
