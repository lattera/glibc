/* Determine protocol families for which interfaces exist.  Linux version.
   Copyright (C) 2003 Free Software Foundation, Inc.
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
#include <ifaddrs.h>
#include <netdb.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>

#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#include "kernel-features.h"


static int
make_request (int fd, pid_t pid, bool *seen_ipv4, bool *seen_ipv6)
{
  struct
  {
    struct nlmsghdr nlh;
    struct rtgenmsg g;
  } req;
  struct sockaddr_nl nladdr;

  req.nlh.nlmsg_len = sizeof (req);
  req.nlh.nlmsg_type = RTM_GETADDR;
  req.nlh.nlmsg_flags = NLM_F_ROOT | NLM_F_MATCH | NLM_F_REQUEST;
  req.nlh.nlmsg_pid = 0;
  req.nlh.nlmsg_seq = time (NULL);
  req.g.rtgen_family = AF_UNSPEC;

  memset (&nladdr, '\0', sizeof (nladdr));
  nladdr.nl_family = AF_NETLINK;

  if (TEMP_FAILURE_RETRY (__sendto (fd, (void *) &req, sizeof (req), 0,
				    (struct sockaddr *) &nladdr,
				    sizeof (nladdr))) < 0)
    return -1;

  *seen_ipv4 = false;
  *seen_ipv6 = false;

  bool done = false;
  char buf[4096];
  struct iovec iov = { buf, sizeof (buf) };

  do
    {
      struct msghdr msg =
	{
	  (void *) &nladdr, sizeof (nladdr),
	  &iov, 1,
	  NULL, 0,
	  0
	};

      ssize_t read_len = TEMP_FAILURE_RETRY (__recvmsg (fd, &msg, 0));
      if (read_len < 0)
	return -1;

      if (msg.msg_flags & MSG_TRUNC)
	return -1;

      struct nlmsghdr *nlmh;
      for (nlmh = (struct nlmsghdr *) buf;
	   NLMSG_OK (nlmh, (size_t) read_len);
	   nlmh = (struct nlmsghdr *) NLMSG_NEXT (nlmh, read_len))
	{
	  if (nladdr.nl_pid != 0 || (pid_t) nlmh->nlmsg_pid != pid
	      || nlmh->nlmsg_seq != req.nlh.nlmsg_seq)
	    continue;

	  if (nlmh->nlmsg_type == RTM_NEWADDR)
	    {
	      struct ifaddrmsg *ifam = (struct ifaddrmsg *) NLMSG_DATA (nlmh);

	      switch (ifam->ifa_family)
		{
		case AF_INET:
		  *seen_ipv4 = true;
		  break;
		case AF_INET6:
		  *seen_ipv6 = true;
		  break;
		default:
		  /* Ignore.  */
		  break;
		}
	    }
	  else if (nlmh->nlmsg_type == NLMSG_DONE)
	    /* We found the end, leave the loop.  */
	    done = true;
	  else ;
	}
    }
  while (! done);

  __close (fd);

  return 0;
}


/* We don't know if we have NETLINK support compiled in in our
   Kernel.  */
#if __ASSUME_NETLINK_SUPPORT == 0
/* Define in ifaddrs.h.  */
extern int __no_netlink_support attribute_hidden;
#else
# define __no_netlink_support 0
#endif


void
attribute_hidden
__check_pf (bool *seen_ipv4, bool *seen_ipv6)
{
  if (! __no_netlink_support)
    {
      int fd = __socket (PF_NETLINK, SOCK_RAW, NETLINK_ROUTE);

      struct sockaddr_nl nladdr;
      memset (&nladdr, '\0', sizeof (nladdr));
      nladdr.nl_family = AF_NETLINK;

      socklen_t addr_len = sizeof (nladdr);

      if (fd >= 0
	  && __bind (fd, (struct sockaddr *) &nladdr, sizeof (nladdr)) == 0
	  && __getsockname (fd, (struct sockaddr *) &nladdr, &addr_len) == 0
	  && make_request (fd, nladdr.nl_pid, seen_ipv4, seen_ipv6) == 0)
	/* It worked.  */
	return;

      if (fd >= 0)
	__close (fd);

#if __ASSUME_NETLINK_SUPPORT == 0
      /* Remember that there is no netlink support.  */
      __no_netlink_support = 1;
#else
      /* We cannot determine what interfaces are available.  Be
	 pessimistic.  */
      *seen_ipv4 = true;
      *seen_ipv6 = true;
#endif
    }

#if __ASSUME_NETLINK_SUPPORT == 0
  /* No netlink.  Get the interface list via getifaddrs.  */
  struct ifaddrs *ifa = NULL;
  if (getifaddrs (&ifa) != 0)
    {
      /* We cannot determine what interfaces are available.  Be
	 pessimistic.  */
      *seen_ipv4 = true;
      *seen_ipv6 = true;
      return;
    }

  *seen_ipv4 = false;
  *seen_ipv6 = false;

  struct ifaddrs *runp;
  for (runp = ifa; runp != NULL; runp = runp->ifa_next)
    if (runp->ifa_addr->sa_family == PF_INET)
      *seen_ipv4 = true;
    else if (runp->ifa_addr->sa_family == PF_INET6)
      *seen_ipv6 = true;

  (void) freeifaddrs (ifa);
#endif
}
