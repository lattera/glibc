/* Copyright (C) 1997, 1998, 1999, 2000 Free Software Foundation, Inc.
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
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <bits/libc-lock.h>

#include "kernel-features.h"

/* Variable to signal whether SIOCGIFCONF is not available.  */
#if __ASSUME_SIOCGIFNAME == 0
static int old_siocgifconf;
#else
# define old_siocgifconf 0
#endif


unsigned int
if_nametoindex (const char *ifname)
{
#ifndef SIOCGIFINDEX
  __set_errno (ENOSYS);
  return 0;
#else
  struct ifreq ifr;
  int fd = __opensock ();

  if (fd < 0)
    return 0;

  strncpy (ifr.ifr_name, ifname, sizeof (ifr.ifr_name));
  if (__ioctl (fd, SIOCGIFINDEX, &ifr) < 0)
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
#ifndef SIOCGIFINDEX
  __set_errno (ENOSYS);
  return NULL;
#else
  int fd = __opensock ();
  struct ifconf ifc;
  unsigned int nifs, i;
  int rq_len;
  struct if_nameindex *idx = NULL;
# define RQ_IFS	4

  if (fd < 0)
    return NULL;

  ifc.ifc_buf = NULL;

  /* We may be able to get the needed buffer size directly, rather than
     guessing.  */
  if (! old_siocgifconf)
    {
      ifc.ifc_buf = NULL;
      ifc.ifc_len = 0;
      if (__ioctl (fd, SIOCGIFCONF, &ifc) < 0 || ifc.ifc_len == 0)
	{
# if __ASSUME_SIOCGIFNAME == 0
	  old_siocgifconf = 1;
# endif
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
  while (ifc.ifc_len == rq_len && old_siocgifconf);

  nifs = ifc.ifc_len / sizeof (struct ifreq);

  idx = malloc ((nifs + 1) * sizeof (struct if_nameindex));
  if (idx == NULL)
    {
      __close (fd);
      __set_errno (ENOBUFS);
      return NULL;
    }

  for (i = 0; i < nifs; ++i)
    {
      struct ifreq *ifr = &ifc.ifc_req[i];
      idx[i].if_name = __strdup (ifr->ifr_name);
      if (idx[i].if_name == NULL
	  || __ioctl (fd, SIOCGIFINDEX, ifr) < 0)
	{
	  int saved_errno = errno;
	  unsigned int j;

	  for (j =  0; j < i; ++j)
	    free (idx[j].if_name);
	  free (idx);
	  __close (fd);
	  if (saved_errno == EINVAL)
	    saved_errno = ENOSYS;
	  else if (saved_errno == ENOMEM)
	    saved_errno = ENOBUFS;
	  __set_errno (saved_errno);
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
#if !defined SIOCGIFINDEX && __ASSUME_SIOCGIFNAME == 0
  __set_errno (ENOSYS);
  return NULL;
#else
# if __ASSUME_SIOCGIFNAME == 0
  struct if_nameindex *idx;
  struct if_nameindex *p;
  char *result = NULL;
# endif

# if defined SIOCGIFNAME || __ASSUME_SIOCGIFNAME > 0
  /* We may be able to do the conversion directly, rather than searching a
     list.  This ioctl is not present in kernels before version 2.1.50.  */
  struct ifreq ifr;
  int fd;
#  if __ASSUME_SIOCGIFNAME == 0
  static int siocgifname_works_not;

  if (!siocgifname_works_not)
#  endif
    {
#  if __ASSUME_SIOCGIFNAME == 0
      int serrno = errno;
#  endif
      int status;

      fd = __opensock ();

      if (fd < 0)
	return NULL;

      ifr.ifr_ifindex = ifindex;
      status = __ioctl (fd, SIOCGIFNAME, &ifr);

      __close (fd);

#  if __ASSUME_SIOCGIFNAME == 0
      if (status  < 0)
	{
	  if (errno == EINVAL)
	    siocgifname_works_not = 1; /* Don't make the same mistake twice. */
	}
      else
	return strncpy (ifname, ifr.ifr_name, IFNAMSIZ);

      __set_errno (serrno);
#  else
      return status < 0 ? NULL : strncpy (ifname, ifr.ifr_name, IFNAMSIZ);
#  endif
    }
# endif

# if __ASSUME_SIOCGIFNAME == 0
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
# endif
#endif
}

#if 0
void
internal_function
__protocol_available (int *have_inet, int *have_inet6)
{
  int fd = __opensock ();
  unsigned int nifs;
  int rq_len;
  struct ifconf ifc;
# define RQ_IFS	4

  /* Wirst case assumption.  */
  *have_inet = 0;
  *have_inet6 = 0;

  if (fd < 0)
    /* We cannot open the socket.  No networking at all?  */
    return;

  /* We may be able to get the needed buffer size directly, rather than
     guessing.  */
  if (! old_siocgifconf)
    {
      ifc.ifc_buf = NULL;
      ifc.ifc_len = 0;
      if (__ioctl (fd, SIOCGIFCONF, &ifc) < 0 || ifc.ifc_len == 0)
	{
# if __ASSUME_SIOCGIFNAME == 0
	  old_siocgifconf = 1;
# endif
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
	  return;
	}
      rq_len *= 2;
    }
  while (ifc.ifc_len == rq_len && old_siocgifconf);

  nifs = ifc.ifc_len / sizeof (struct ifreq);

  /* Go through all the interfaces and get the address.  */
  while (nifs-- > 0)
    if (__ioctl (fd, SIOCGIFADDR, &ifc.ifc_req[nifs]) >= 0)
      {
	/* We successfully got information about this interface.  Now
	   test whether it is an IPv4 or IPv6 address.  */
	if (ifc.ifc_req[nifs].ifr_addr.sa_family == AF_INET)
	  *have_inet = 1;
	else if (ifc.ifc_req[nifs].ifr_addr.sa_family == AF_INET6)
	  *have_inet6 = 1;

	/* Note, this is & not &&.  It works since the values are always
	   0 or 1.  */
	if (*have_inet & *have_inet6)
	  /* We can stop early.  */
	  break;
      }

  __close (fd);
}
#endif
