/* Copyright (C) 1997, 1998 Free Software Foundation, Inc.
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

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <bits/libc-lock.h>

/* Try to get a socket to talk to the kernel.  */
#if defined SIOGIFINDEX || defined SIOGIFNAME
static int
internal_function
opensock (void)
{
  /* Cache the last AF that worked, to avoid many redundant calls to
     socket().  */
  static int sock_af = -1;
  int fd = -1;
  __libc_lock_define_initialized (static, lock);

  if (sock_af != -1)
    {
      fd = __socket (sock_af, SOCK_DGRAM, 0);
      if (fd != -1)
        return fd;
    }

  __libc_lock_lock (lock);

  if (sock_af != -1)
    fd = __socket (sock_af, SOCK_DGRAM, 0);

  if (fd == -1)
    {
      fd = __socket (sock_af = AF_INET6, SOCK_DGRAM, 0);
      if (fd < 0)
	fd = __socket (sock_af = AF_INET, SOCK_DGRAM, 0);
      if (fd < 0)
	fd = __socket (sock_af = AF_IPX, SOCK_DGRAM, 0);
      if (fd < 0)
	fd = __socket (sock_af = AF_AX25, SOCK_DGRAM, 0);
      if (fd < 0)
	fd = __socket (sock_af = AF_APPLETALK, SOCK_DGRAM, 0);
    }

  __libc_lock_unlock (lock);
  return fd;
}
#endif

unsigned int
if_nametoindex (const char *ifname)
{
#ifndef SIOGIFINDEX
  __set_errno (ENOSYS);
  return 0;
#else
  struct ifreq ifr;
  int fd = opensock ();

  if (fd < 0)
    return 0;

  strncpy (ifr.ifr_name, ifname, sizeof (ifr.ifr_name));
  if (__ioctl (fd, SIOGIFINDEX, &ifr) < 0)
    {
      int saved_errno = errno;
      __close (fd);
      if (saved_errno == EINVAL)
	__set_errno (ENOSYS);
      return 0;
    }
  __close (fd);
  return ifr.ifr_ifindex;
#endif
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
#ifndef SIOGIFINDEX
  __set_errno (ENOSYS);
  return NULL;
#else
  int fd = opensock ();
  struct ifconf ifc;
  unsigned int nifs, i;
  int rq_len;
  struct if_nameindex *idx = NULL;
  static int new_siocgifconf = 1;
#define RQ_IFS	4

  if (fd < 0)
    return NULL;

  ifc.ifc_buf = NULL;

  /* We may be able to get the needed buffer size directly, rather than
     guessing.  */
  if (new_siocgifconf)
    {
      ifc.ifc_buf = NULL;
      ifc.ifc_len = 0;
      if (__ioctl (fd, SIOCGIFCONF, &ifc) < 0 || ifc.ifc_len == 0)
	{
	  new_siocgifconf = 0;
	  rq_len = RQ_IFS * sizeof (struct ifreq);
	}
      else
	rq_len = ifc.ifc_len;
    }
  else
    rq_len = RQ_IFS * sizeof (struct ifreq);

  /* Read all the interfaces out of the kernel.  */
  do
    {
      ifc.ifc_buf = alloca (ifc.ifc_len = rq_len);
      if (ifc.ifc_buf == NULL || __ioctl (fd, SIOCGIFCONF, &ifc) < 0)
	{
	  __close (fd);
	  return NULL;
	}
      rq_len *= 2;
    }
  while (ifc.ifc_len == rq_len && new_siocgifconf == 0);

  nifs = ifc.ifc_len / sizeof (struct ifreq);

  idx = malloc ((nifs + 1) * sizeof (struct if_nameindex));
  if (idx == NULL)
    {
      __close (fd);
      return NULL;
    }

  for (i = 0; i < nifs; ++i)
    {
      struct ifreq *ifr = &ifc.ifc_req[i];
      idx[i].if_name = __strdup (ifr->ifr_name);
      if (idx[i].if_name == NULL
	  || __ioctl (fd, SIOGIFINDEX, ifr) < 0)
	{
	  int saved_errno = errno;
	  unsigned int j;

	  for (j =  0; j < i; ++j)
	    free (idx[j].if_name);
	  free (idx);
	  __close (fd);
	  if (saved_errno == EINVAL)
	    __set_errno (ENOSYS);
	  return NULL;
	}
      idx[i].if_index = ifr->ifr_ifindex;
    }

  idx[i].if_index = 0;
  idx[i].if_name = NULL;

  __close (fd);
  return idx;
#endif
}

char *
if_indextoname (unsigned int ifindex, char *ifname)
{
#ifndef SIOGIFINDEX
  __set_errno (ENOSYS);
  return NULL;
#else
  struct if_nameindex *idx;
  struct if_nameindex *p;
  char *result = NULL;

#ifdef SIOGIFNAME
  /* We may be able to do the conversion directly, rather than searching a
     list.  This ioctl is not present in kernels before version 2.1.50.  */
  struct ifreq ifr;
  int fd;
  static int siogifname_works = 1;

  if (siogifname_works)
    {
      int serrno = errno;

      fd = opensock ();

      if (fd < 0)
	return NULL;

      ifr.ifr_ifindex = ifindex;
      if (__ioctl (fd, SIOGIFNAME, &ifr) < 0)
	{
	  if (errno == EINVAL)
	    siogifname_works = 0;   /* Don't make the same mistake twice. */
	}
      else
	{
	  __close (fd);
	  return strncpy (ifname, ifr.ifr_name, IFNAMSIZ);
	}

      __close (fd);

      __set_errno (serrno);
    }
#endif

  idx = if_nameindex ();

  if (idx != NULL)
    {
      for (p = idx; p->if_index || p->if_name; ++p)
	if (p->if_index == ifindex)
	  {
	    result = strncpy (ifname, p->if_name, IFNAMSIZ);
	    break;
	  }

      if_freenameindex (idx);
    }
  return result;
#endif
}
