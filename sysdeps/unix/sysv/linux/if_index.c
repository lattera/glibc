/* Copyright (C) 1997 Free Software Foundation, Inc.
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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <bits/libc-lock.h>

/* Try to get a socket to talk to the kernel.  */
static int
opensock (void)
{
  /* Cache the last AF that worked, to avoid many redundant calls to
     socket().  */
  static int sock_af = -1;
  int fd = -1;
  __libc_lock_define_initialized (static, lock);

  if (sock_af != -1)
    {
      fd = socket (sock_af, SOCK_DGRAM, 0);
      if (fd != -1)
        return fd;
    }

  __libc_lock_lock (lock);

  if (sock_af != -1)
    fd = socket (sock_af, SOCK_DGRAM, 0);

  if (fd == -1)
    {
      fd = socket (sock_af = AF_INET6, SOCK_DGRAM, 0);
      if (fd < 0)
	fd = socket (sock_af = AF_INET, SOCK_DGRAM, 0);
      if (fd < 0)
	fd = socket (sock_af = AF_IPX, SOCK_DGRAM, 0);
      if (fd < 0)
	fd = socket (sock_af = AF_AX25, SOCK_DGRAM, 0);
      if (fd < 0)
	fd = socket (sock_af = AF_APPLETALK, SOCK_DGRAM, 0);
    }

  __libc_lock_unlock (lock);
  return fd;
}

unsigned int
if_nametoindex (const char *ifname)
{
  struct ifreq ifr;
  int fd = opensock ();

  if (fd < 0)
    return 0;

  strncpy (ifr.ifr_name, ifname, sizeof (ifr.ifr_name));
  if (ioctl (fd, SIOGIFINDEX, &ifr) < 0)
    {
      close (fd);
      return 0;
    }
  close (fd);
  return ifr.ifr_ifindex;
}

void
if_freenameindex (struct if_nameindex *ifn)
{
  struct if_nameindex *ptr = ifn;
  while (ptr->if_name || ptr->if_index)
    {
      if (ptr->if_name)
	free (ptr->if_name);
      ++ptr;
    }
  free (ifn);
}

struct if_nameindex *
if_nameindex (void)
{
  int fd = opensock ();
  struct ifconf ifc;
  unsigned int rq_ifs = 4, nifs, i;
  struct if_nameindex *idx = NULL;

  if (fd < 0)
    return NULL;

  ifc.ifc_buf = NULL;

  /* Read all the interfaces out of the kernel.  */
  do
    {
      rq_ifs *= 2;
      ifc.ifc_len = rq_ifs * sizeof (struct ifreq);
      ifc.ifc_buf = realloc (ifc.ifc_buf, ifc.ifc_len);
      if (ifc.ifc_buf == NULL)
	{
	  close(fd);
	  return NULL;
	}
      if (ioctl (fd, SIOCGIFCONF, &ifc) < 0)
	goto jump;
    }
  while ((unsigned int) ifc.ifc_len == (rq_ifs * sizeof (struct ifreq)));

  nifs = ifc.ifc_len / sizeof (struct ifreq);
  ifc.ifc_buf = realloc (ifc.ifc_buf, ifc.ifc_len);

  idx = malloc ((nifs+1) * sizeof (struct if_nameindex));
  if (idx == NULL)
    goto jump;

  for (i = 0; i < nifs; ++i)
    {
      struct ifreq *ifr = &ifc.ifc_req[i];
      if ((idx[i].if_name = malloc (strlen (ifr->ifr_name)+1)) == NULL)
	{
	  free (idx);
	  idx = NULL;
	  goto jump;
	}
      strcpy (idx[i].if_name, ifr->ifr_name);
      if (ioctl (fd, SIOGIFINDEX, ifr) < 0)
	{
	  free (idx);
	  idx = NULL;
	  goto jump;
	}
      idx[i].if_index = ifr->ifr_ifindex;
    }
  idx[i].if_index = 0;
  idx[i].if_name = NULL;

jump:
  free (ifc.ifc_buf);
  close (fd);
  return idx;
}

char *
if_indextoname (unsigned int ifindex, char *ifname)
{
  struct if_nameindex *idx = if_nameindex ();
  struct if_nameindex *p;

  for (p = idx; p->if_index || p->if_name; ++p)
    if (p->if_index == ifindex)
      {
	strncpy (ifname, p->if_name, IFNAMSIZ);
	if_freenameindex (idx);
	return ifname;
      }

  if_freenameindex (idx);
  return NULL;
}
