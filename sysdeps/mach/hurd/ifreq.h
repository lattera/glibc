/* Fetch the host's network interface list.  Hurd version.
   Copyright (C) 2002 Free Software Foundation, Inc.
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

#include <net/if.h>
#include <hurd.h>
#include <hurd/pfinet.h>
#include <sys/socket.h>
#include <sys/mman.h>


static inline void
__ifreq (struct ifreq **ifreqs, int *num_ifs, int sockfd)
{
  file_t server;

  server = _hurd_socket_server (PF_INET, 0);
  if (server == MACH_PORT_NULL)
    {
    out:
      *num_ifs = 0;
      *ifreqs = NULL;
    }
  else
    {
      char *data = NULL;
      size_t len = 0;
      error_t err = __pfinet_siocgifconf (server, -1, &data, &len);
      if (err == MACH_SEND_INVALID_DEST || err == MIG_SERVER_DIED)
	{
	  /* On the first use of the socket server during the operation,
	     allow for the old server port dying.  */
	  server = _hurd_socket_server (PF_INET, 1);
	  if (server == MACH_PORT_NULL)
	    goto out;
	  err = __pfinet_siocgifconf (server, -1, (data_t *) ifreqs, &len);
	}
      if (err)
	goto out;

      if (len % sizeof (struct ifreq) != 0)
	{
	  munmap (data, len);
	  errno = EGRATUITOUS;
	  goto out;
	}
      *num_ifs = len / sizeof (struct ifreq);
      *ifreqs = (struct ifreq *) data;
    }

}


static inline struct ifreq *
__if_nextreq (struct ifreq *ifr)
{
  return ifr + 1;
}


static inline void
__if_freereq (struct ifreq *ifreqs, int num_ifs)
{
  __munmap (ifreqs, num_ifs * sizeof (struct ifreq));
}
