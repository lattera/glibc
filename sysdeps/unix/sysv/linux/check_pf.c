/* Determine protocol families for which interfaces exist.  Linux version.
   Copyright (C) 2003, 2006-2008, 2010, 2011 Free Software Foundation, Inc.
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

#include <assert.h>
#include <errno.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <stddef.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>

#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#include <not-cancel.h>
#include <kernel-features.h>
#include <bits/libc-lock.h>
#include <atomic.h>
#include <nscd/nscd-client.h>


#ifndef IFA_F_HOMEADDRESS
# define IFA_F_HOMEADDRESS 0
#endif
#ifndef IFA_F_OPTIMISTIC
# define IFA_F_OPTIMISTIC 0
#endif


struct cached_data
{
  uint32_t timestamp;
  uint32_t usecnt;
  bool seen_ipv4;
  bool seen_ipv6;
  size_t in6ailen;
  struct in6addrinfo in6ai[0];
};

static struct cached_data noai6ai_cached =
  {
    .usecnt = 1,	/* Make sure we never try to delete this entry.  */
    .in6ailen = 0
  };

static struct cached_data *cache;
__libc_lock_define_initialized (static, lock);


#ifdef IS_IN_nscd
static uint32_t nl_timestamp;

uint32_t
__bump_nl_timestamp (void)
{
  if (atomic_increment_val (&nl_timestamp) == 0)
    atomic_increment (&nl_timestamp);

  return nl_timestamp;
}
#endif


static struct cached_data *
make_request (int fd, pid_t pid)
{
  struct req
  {
    struct nlmsghdr nlh;
    struct rtgenmsg g;
    /* struct rtgenmsg consists of a single byte.  This means there
       are three bytes of padding included in the REQ definition.
       We make them explicit here.  */
    char pad[3];
  } req;
  struct sockaddr_nl nladdr;

  req.nlh.nlmsg_len = sizeof (req);
  req.nlh.nlmsg_type = RTM_GETADDR;
  req.nlh.nlmsg_flags = NLM_F_ROOT | NLM_F_MATCH | NLM_F_REQUEST;
  req.nlh.nlmsg_pid = 0;
  req.nlh.nlmsg_seq = time (NULL);
  req.g.rtgen_family = AF_UNSPEC;

  assert (sizeof (req) - offsetof (struct req, pad) == 3);
  memset (req.pad, '\0', sizeof (req.pad));

  memset (&nladdr, '\0', sizeof (nladdr));
  nladdr.nl_family = AF_NETLINK;

#ifdef PAGE_SIZE
  /* Help the compiler optimize out the malloc call if PAGE_SIZE
     is constant and smaller or equal to PTHREAD_STACK_MIN/4.  */
  const size_t buf_size = PAGE_SIZE;
#else
  const size_t buf_size = __getpagesize ();
#endif
  bool use_malloc = false;
  char *buf;

  if (__libc_use_alloca (buf_size))
    buf = alloca (buf_size);
  else
    {
      buf = malloc (buf_size);
      if (buf != NULL)
	use_malloc = true;
      else
	goto out_fail;
    }

  struct iovec iov = { buf, buf_size };

  if (TEMP_FAILURE_RETRY (__sendto (fd, (void *) &req, sizeof (req), 0,
				    (struct sockaddr *) &nladdr,
				    sizeof (nladdr))) < 0)
    goto out_fail;

  bool done = false;
  struct in6ailist
  {
    struct in6addrinfo info;
    struct in6ailist *next;
  } *in6ailist = NULL;
  size_t in6ailistlen = 0;
  bool seen_ipv4 = false;
  bool seen_ipv6 = false;

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
	goto out_fail;

      if (msg.msg_flags & MSG_TRUNC)
	goto out_fail;

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
	      struct rtattr *rta = IFA_RTA (ifam);
	      size_t len = nlmh->nlmsg_len - NLMSG_LENGTH (sizeof (*ifam));

	      if (ifam->ifa_family != AF_INET
		  && ifam->ifa_family != AF_INET6)
		continue;

	      const void *local = NULL;
	      const void *address = NULL;
	      while (RTA_OK (rta, len))
		{
		  switch (rta->rta_type)
		    {
		    case IFA_LOCAL:
		      local = RTA_DATA (rta);
		      break;

		    case IFA_ADDRESS:
		      address = RTA_DATA (rta);
		      goto out;
		    }

		  rta = RTA_NEXT (rta, len);
		}

	      if (local != NULL)
		{
		  address = local;
		out:
		  if (ifam->ifa_family == AF_INET)
		    {
		      if (*(const in_addr_t *) address
			  != htonl (INADDR_LOOPBACK))
			seen_ipv4 = true;
		    }
		  else
		    {
		      if (!IN6_IS_ADDR_LOOPBACK (address))
			seen_ipv6 = true;
		    }
		}

	      struct in6ailist *newp = alloca (sizeof (*newp));
	      newp->info.flags = (((ifam->ifa_flags
				    & (IFA_F_DEPRECATED
				       | IFA_F_OPTIMISTIC))
				   ? in6ai_deprecated : 0)
				  | ((ifam->ifa_flags
				      & IFA_F_HOMEADDRESS)
				     ? in6ai_homeaddress : 0));
	      newp->info.prefixlen = ifam->ifa_prefixlen;
	      newp->info.index = ifam->ifa_index;
	      if (ifam->ifa_family == AF_INET)
		{
		  newp->info.addr[0] = 0;
		  newp->info.addr[1] = 0;
		  newp->info.addr[2] = htonl (0xffff);
		  newp->info.addr[3] = *(const in_addr_t *) address;
		}
	      else
		memcpy (newp->info.addr, address, sizeof (newp->info.addr));
	      newp->next = in6ailist;
	      in6ailist = newp;
	      ++in6ailistlen;
	    }
	  else if (nlmh->nlmsg_type == NLMSG_DONE)
	    /* We found the end, leave the loop.  */
	    done = true;
	}
    }
  while (! done);

  struct cached_data *result;
  if (seen_ipv6 && in6ailist != NULL)
    {
      result = malloc (sizeof (*result)
		       + in6ailistlen * sizeof (struct in6addrinfo));
      if (result == NULL)
	goto out_fail;

#ifdef IS_IN_nscd
      result->timestamp = nl_timestamp;
#else
      result->timestamp = __nscd_get_nl_timestamp ();
#endif
      result->usecnt = 2;
      result->seen_ipv4 = seen_ipv4;
      result->seen_ipv6 = true;
      result->in6ailen = in6ailistlen;

      do
	{
	  result->in6ai[--in6ailistlen] = in6ailist->info;
	  in6ailist = in6ailist->next;
	}
      while (in6ailist != NULL);
    }
  else
    {
      atomic_add (&noai6ai_cached.usecnt, 2);
      noai6ai_cached.seen_ipv4 = seen_ipv4;
      noai6ai_cached.seen_ipv6 = seen_ipv6;
      result = &noai6ai_cached;
    }

  if (use_malloc)
    free (buf);
  return result;

out_fail:
  if (use_malloc)
    free (buf);
  return NULL;
}


/* We don't know if we have NETLINK support compiled into our
   Kernel.  */
#if __ASSUME_NETLINK_SUPPORT == 0
/* Define in ifaddrs.h.  */
extern int __no_netlink_support attribute_hidden;
#else
# define __no_netlink_support 0
#endif


void
attribute_hidden
__check_pf (bool *seen_ipv4, bool *seen_ipv6,
	    struct in6addrinfo **in6ai, size_t *in6ailen)
{
  *in6ai = NULL;
  *in6ailen = 0;

  if (! __no_netlink_support)
    {
      struct cached_data *olddata = NULL;
      struct cached_data *data = NULL;

      __libc_lock_lock (lock);

#ifdef IS_IN_nscd
# define cache_valid() nl_timestamp != 0 && cache->timestamp == nl_timestamp
#else
# define cache_valid() \
      ({ uint32_t val = __nscd_get_nl_timestamp ();			      \
	 val != 0 && cache->timestamp == val; })
#endif
      if (cache != NULL && cache_valid ())
	{
	  data = cache;
	  atomic_increment (&cache->usecnt);
	}
      else
	{
	  int fd = __socket (PF_NETLINK, SOCK_RAW, NETLINK_ROUTE);

	  if (__builtin_expect (fd >= 0, 1))
	    {
	      struct sockaddr_nl nladdr;
	      memset (&nladdr, '\0', sizeof (nladdr));
	      nladdr.nl_family = AF_NETLINK;

	      socklen_t addr_len = sizeof (nladdr);

	      if(__bind (fd, (struct sockaddr *) &nladdr, sizeof (nladdr)) == 0
		 && __getsockname (fd, (struct sockaddr *) &nladdr,
				   &addr_len) == 0)
		data = make_request (fd, nladdr.nl_pid);

	      close_not_cancel_no_status (fd);
	    }

	  if (data != NULL)
	    {
	      olddata = cache;
	      cache = data;
	    }
	}

      __libc_lock_unlock (lock);

      if (data != NULL)
	{
	  /* It worked.  */
	  *seen_ipv4 = data->seen_ipv4;
	  *seen_ipv6 = data->seen_ipv6;
	  *in6ailen = data->in6ailen;
	  *in6ai = data->in6ai;

	  if (olddata != NULL && olddata->usecnt > 0
	      && atomic_add_zero (&olddata->usecnt, -1))
	    free (olddata);

	  return;
	}

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

  struct ifaddrs *runp;
  for (runp = ifa; runp != NULL; runp = runp->ifa_next)
    if (runp->ifa_addr != NULL)
      {
	if (runp->ifa_addr->sa_family == PF_INET)
	  *seen_ipv4 = true;
	else if (runp->ifa_addr->sa_family == PF_INET6)
	  *seen_ipv6 = true;
      }

  (void) freeifaddrs (ifa);
#endif
}


void
__free_in6ai (struct in6addrinfo *ai)
{
  if (ai != NULL)
    {
      struct cached_data *data =
	(struct cached_data *) ((char *) ai
				- offsetof (struct cached_data, in6ai));

      if (atomic_add_zero (&data->usecnt, -1))
	{
	  __libc_lock_lock (lock);

	  if (data->usecnt == 0)
	    /* Still unused.  */
	    free (data);

	  __libc_lock_unlock (lock);
	}
    }
}
