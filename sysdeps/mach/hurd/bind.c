/* Copyright (C) 1992, 1994 Free Software Foundation, Inc.
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
DEFUN(bind, (fd, addr, len),
      int fd AND struct sockaddr *addr AND size_t len)
{
  addr_port_t aport;
  error_t err;

  if (addr->sa_family == AF_LOCAL)
    {
      /* For the local domain, we must create a node in the filesystem
	 using the ifsock translator and then fetch the address from it.  */
      struct sockaddr_un *unaddr = (struct sockaddr_un *) addr;
      file_t dir, node;
      char name[len - offsetof (struct sockaddr_un, sun_path)], *n;
      strncpy (name, unaddr->sun_path, sizeof name);
      dir = __file_name_split (name, &n);
      if (dir == MACH_PORT_NULL)
	return -1;
      
      /* Create a new, unlinked node in the target directory.  */
      err = __dir_mkfile (dir, O_CREAT, 0666 & ~_hurd_umask, &node);

      if (! err)
	{
	  file_t ifsock;
	  /* Set the node's translator to make it a local-domain socket.  */
	  err = __file_set_translator (node, 
				       FS_TRANS_EXCL | FS_TRANS_SET,
				       FS_TRANS_EXCL | FS_TRANS_SET, 0,
				       _HURD_IFSOCK, sizeof _HURD_IFSOCK,
				       MACH_PORT_NULL,
				       MACH_MSG_TYPE_COPY_SEND);
	  if (! err)
	    /* Link the node, now a socket, into the target directory.  */
	    err = __dir_link (node, dir, n);
	  if (! err)
	    /* Get a port to the ifsock translator.  */
	    {
	      retry_type retry;
	      string_t retry_name;

	      err = __dir_lookup (dir, n, 0, 0, &retry, retry_name, &ifsock);

	      if (! err && (retry != FS_RETRY_NORMAL || retry_name[0]))
		/* Either someone has fucked with our new node, or the ifsock
		   translator is acting very oddly.  */
		{
		  struct hurd_userlink crdir_ulink;
		  file_t crdir =
		    _hurd_port_get (&_hurd_ports[INIT_PORT_CRDIR],
				    &crdir_ulink);

		  err = __hurd_file_name_lookup_retry (crdir,
						       retry, retry_name, 0, 0,
						       &ifsock);

		  _hurd_port_free (&_hurd_ports[INIT_PORT_CRDIR],
				   &crdir_ulink, crdir);
		}

	      if (err)
		/* If we failed, get rid of the node we created.  */
		__dir_unlink  (dir, n);
	    }
	  if (! err)
	    /* Get the address port.  */
	    err = __ifsock_getsockaddr (ifsock, &aport);
	  __mach_port_deallocate (__mach_task_self (), ifsock);
	  __mach_port_deallocate (__mach_task_self (), node);
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
							   addr->sa_family,
							   (char *) addr, len,
							   &aport, 1);
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
