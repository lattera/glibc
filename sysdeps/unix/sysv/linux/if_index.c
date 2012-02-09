/* Copyright (C) 1997, 1998, 1999, 2000, 2002, 2003, 2004, 2005, 2007
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

#include <alloca.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <bits/libc-lock.h>
#include <not-cancel.h>
#include <kernel-features.h>

#include "netlinkaccess.h"


/* Variable to signal whether SIOCGIFCONF is not available.  */
# if __ASSUME_SIOCGIFNAME == 0
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
      close_not_cancel_no_status (fd);
      if (saved_errno == EINVAL)
	__set_errno (ENOSYS);
      return 0;
    }
  close_not_cancel_no_status (fd);
  return ifr.ifr_ifindex;
#endif
}
libc_hidden_def (if_nametoindex)


void
if_freenameindex (struct if_nameindex *ifn)
{
  struct if_nameindex *ptr = ifn;
  while (ptr->if_name || ptr->if_index)
    {
      free (ptr->if_name);
      ++ptr;
    }
  free (ifn);
}
libc_hidden_def (if_freenameindex)


#if __ASSUME_NETLINK_SUPPORT == 0
static struct if_nameindex *
if_nameindex_ioctl (void)
{
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
  ifc.ifc_buf = alloca (rq_len);
  ifc.ifc_len = rq_len;
  while (1)
    {
        if (__ioctl (fd, SIOCGIFCONF, &ifc) < 0)
	{
	  close_not_cancel_no_status (fd);
	  return NULL;
	}
      if (ifc.ifc_len < rq_len || ! old_siocgifconf)
	break;

      ifc.ifc_buf = extend_alloca (ifc.ifc_buf, rq_len, 2 * rq_len);
      ifc.ifc_len = rq_len;
    }

  nifs = ifc.ifc_len / sizeof (struct ifreq);

  idx = malloc ((nifs + 1) * sizeof (struct if_nameindex));
  if (idx == NULL)
    {
      close_not_cancel_no_status (fd);
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
	  close_not_cancel_no_status (fd);
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

  close_not_cancel_no_status (fd);
  return idx;
}
#endif


static struct if_nameindex *
if_nameindex_netlink (void)
{
  struct netlink_handle nh = { 0, 0, 0, NULL, NULL };
  struct if_nameindex *idx = NULL;

  if (__no_netlink_support || __netlink_open (&nh) < 0)
    return NULL;


  /* Tell the kernel that we wish to get a list of all
     active interfaces.  Collect all data for every interface.  */
  if (__netlink_request (&nh, RTM_GETLINK) < 0)
    goto exit_free;

  /* Count the interfaces.  */
  unsigned int nifs = 0;
  for (struct netlink_res *nlp = nh.nlm_list; nlp; nlp = nlp->next)
    {
      struct nlmsghdr *nlh;
      size_t size = nlp->size;

      if (nlp->nlh == NULL)
	continue;

      /* Walk through all entries we got from the kernel and look, which
         message type they contain.  */
      for (nlh = nlp->nlh; NLMSG_OK (nlh, size); nlh = NLMSG_NEXT (nlh, size))
	{
	  /* Check if the message is what we want.  */
	  if ((pid_t) nlh->nlmsg_pid != nh.pid || nlh->nlmsg_seq != nlp->seq)
	    continue;

	  if (nlh->nlmsg_type == NLMSG_DONE)
	    break;		/* ok */

	  if (nlh->nlmsg_type == RTM_NEWLINK)
	    ++nifs;
	}
    }

  idx = malloc ((nifs + 1) * sizeof (struct if_nameindex));
  if (idx == NULL)
    {
    nomem:
      __set_errno (ENOBUFS);
      goto exit_free;
    }

  /* Add the interfaces.  */
  nifs = 0;
  for (struct netlink_res *nlp = nh.nlm_list; nlp; nlp = nlp->next)
    {
      struct nlmsghdr *nlh;
      size_t size = nlp->size;

      if (nlp->nlh == NULL)
	continue;

      /* Walk through all entries we got from the kernel and look, which
         message type they contain.  */
      for (nlh = nlp->nlh; NLMSG_OK (nlh, size); nlh = NLMSG_NEXT (nlh, size))
	{
	  /* Check if the message is what we want.  */
	  if ((pid_t) nlh->nlmsg_pid != nh.pid || nlh->nlmsg_seq != nlp->seq)
	    continue;

	  if (nlh->nlmsg_type == NLMSG_DONE)
	    break;		/* ok */

	  if (nlh->nlmsg_type == RTM_NEWLINK)
	    {
	      struct ifinfomsg *ifim = (struct ifinfomsg *) NLMSG_DATA (nlh);
	      struct rtattr *rta = IFLA_RTA (ifim);
	      size_t rtasize = IFLA_PAYLOAD (nlh);

	      idx[nifs].if_index = ifim->ifi_index;

	      while (RTA_OK (rta, rtasize))
		{
		  char *rta_data = RTA_DATA (rta);
		  size_t rta_payload = RTA_PAYLOAD (rta);

		  if (rta->rta_type == IFLA_IFNAME)
		    {
		      idx[nifs].if_name = __strndup (rta_data, rta_payload);
		      if (idx[nifs].if_name == NULL)
			{
			  idx[nifs].if_index = 0;
			  if_freenameindex (idx);
			  idx = NULL;
			  goto nomem;
			}
		      break;
		    }

		  rta = RTA_NEXT (rta, rtasize);
		}

	      ++nifs;
	    }
	}
    }

  idx[nifs].if_index = 0;
  idx[nifs].if_name = NULL;

 exit_free:
  __netlink_free_handle (&nh);
  __netlink_close (&nh);

  return idx;
}


struct if_nameindex *
if_nameindex (void)
{
#ifndef SIOCGIFINDEX
  __set_errno (ENOSYS);
  return NULL;
#else
  struct if_nameindex *result = if_nameindex_netlink ();
# if __ASSUME_NETLINK_SUPPORT == 0
  if (__no_netlink_support)
    result = if_nameindex_ioctl ();
# endif
  return result;
#endif
}
libc_hidden_def (if_nameindex)


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

      close_not_cancel_no_status (fd);

      if (status  < 0)
	{
#  if __ASSUME_SIOCGIFNAME == 0
	  if (errno == EINVAL)
	    siocgifname_works_not = 1; /* Don't make the same mistake twice. */
	  else
#  endif
	    {
	      if (errno == ENODEV)
		/* POSIX requires ENXIO.  */
		__set_errno (ENXIO);

	      return NULL;
	    }
	}
      else
	return strncpy (ifname, ifr.ifr_name, IFNAMSIZ);

#  if __ASSUME_SIOCGIFNAME == 0
      __set_errno (serrno);
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

      if (result == NULL)
	__set_errno (ENXIO);
    }
  return result;
# endif
#endif
}
libc_hidden_def (if_indextoname)


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
      if (__ioctl (fd, SIOCGIFCONF, &ifc) < 0)
	{
	  close_not_cancel_no_status (fd);
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

  close_not_cancel_no_status (fd);
}
#endif
