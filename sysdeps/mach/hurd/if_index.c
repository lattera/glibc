/* Find network interface names and index numbers.  Hurd version.
   Copyright (C) 2000 Free Software Foundation, Inc.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <net/if.h>
#include <hurd.h>
#include <hurd/fsys.h>
#include <string.h>
#include <sys/mman.h>

static int
map_interfaces (int domain,
		unsigned int *idxp,
		int (*counted_initializer) (unsigned int count,
					    size_t nameslen),
		int (*iterator) (const char *))
{
  static const char ifopt[] = "--interface=";
  file_t server;
  char optsbuf[512], *opts = optsbuf, *p;
  size_t optslen = sizeof optsbuf;
  error_t err;

  /* Find the socket server for DOMAIN.  */
  server = _hurd_socket_server (domain, 0);
  if (server == MACH_PORT_NULL)
    return 0;

  err = __file_get_fs_options (server, &opts, &optslen);
  if (err == MACH_SEND_INVALID_DEST || err == MIG_SERVER_DIED)
    {
      /* On the first use of the socket server during the operation,
	 allow for the old server port dying.  */
      server = _hurd_socket_server (domain, 1);
      if (server == MACH_PORT_NULL)
	return -1;
      err = __file_get_fs_options (server, &opts, &optslen);
    }
  if (err)
    return __hurd_fail (err), 0;

  if (counted_initializer)
    {
      unsigned int count = 0;
      size_t nameslen = 0;
      p = memchr (opts, '\0', optslen);
      while (p != 0)
	{
	  char *end = memchr (p + 1, '\0', optslen - (p - opts));
	  if (end == 0)
	    break;
	  if (optslen - (p - opts) >= sizeof ifopt
	      && !memcmp (p + 1, ifopt, sizeof ifopt - 1))
	    {
	      size_t len = end + 1 - (p + sizeof ifopt);
	      nameslen += len > IFNAMSIZ+1 ? IFNAMSIZ+1 : len;
	      ++count;
	    }
	  p = end;
	}

      if ((*counted_initializer) (count, nameslen))
	return 0;
    }

  *idxp = 0;
  for (p = memchr (opts, '\0', optslen); p != 0;
       p = memchr (p + 1, '\0', optslen - (p - opts)))
    {
      ++*idxp;
      if (optslen - (p - opts) >= sizeof ifopt
	  && !memcmp (p + 1, ifopt, sizeof ifopt - 1)
	  && (*iterator) (p + sizeof ifopt))
	break;
    }

  if (opts != optsbuf)
    __munmap (opts, optslen);

  return 1;
}

unsigned int
if_nametoindex (const char *ifname)
{
  unsigned int idx;
  int find_name (const char *name)
    {
      return !strcmp (name, ifname);
    }
  return map_interfaces (PF_INET, &idx, 0, &find_name) ? idx : 0;
}

char *
if_indextoname (unsigned int ifindex, char *ifname)
{
  unsigned int idx;
  int find_idx (const char *name)
    {
      if (idx == ifindex)
	{
	  strncpy (ifname, name, IFNAMSIZ);
	  return 1;
	}
      return 0;
    }
  return map_interfaces (PF_INET, &idx, 0, &find_idx) ? ifname : NULL;
}


struct if_nameindex *
if_nameindex (void)
{
  unsigned int idx;
  struct if_nameindex *buf;
  char *namep;
  int alloc (unsigned int count, size_t nameslen)
    {
      buf = malloc ((sizeof buf[0] * (count + 1)) + nameslen);
      if (buf == 0)
	return 1;
      buf[count].if_index = 0;
      buf[count].if_name = NULL;
      namep = (char *) &buf[count + 1];
      return 0;
    }
  int fill (const char *name)
    {
      buf[idx - 1].if_index = idx;
      buf[idx - 1].if_name = namep;
      namep = __memccpy (namep, name, '\0', IFNAMSIZ+1) ?: &namep[IFNAMSIZ+1];
      return 0;
    }

  return map_interfaces (PF_INET, &idx, &alloc, &fill) ? buf : NULL;
}

void
if_freenameindex (struct if_nameindex *ifn)
{
  free (ifn);
}

#if 0
void
internal_function
__protocol_available (int *have_inet, int *have_inet6)
{
  *have_inet = _hurd_socket_server (PF_INET, 0) != MACH_PORT_NULL;
  *have_inet6 = _hurd_socket_server (PF_INET6, 0) != MACH_PORT_NULL;
}
#endif
