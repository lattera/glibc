/* getifaddrs -- get names and addresses of all network interfaces
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

#include <assert.h>
#include <errno.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netpacket/packet.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sysdep.h>
#include <time.h>
#include <unistd.h>

#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#include "kernel-features.h"

/* We don't know if we have NETLINK support compiled in in our
   Kernel, so include the old implementation as fallback.  */
#if __ASSUME_NETLINK_SUPPORT == 0
static int no_netlink_support;

# define getifaddrs fallback_getifaddrs
# include "sysdeps/gnu/ifaddrs.c"
# undef getifaddrs

#else

# define no_netlink_support 0

#endif


struct netlink_res
{
  struct netlink_res *next;
  struct nlmsghdr *nlh;
  size_t size;			/* Size of response.  */
  uint32_t seq;			/* sequential number we used.  */
};


struct netlink_handle
{
  int fd;			/* Netlink file descriptor.  */
  pid_t pid;			/* Process ID.  */
  uint32_t seq;			/* The sequence number we use currently.  */
  struct netlink_res *nlm_list;	/* Pointer to list of responses.  */
  struct netlink_res *end_ptr;	/* For faster append of new entries.  */
};


/* struct to hold the data for one ifaddrs entry, so we can allocate
   everything at once.  */
struct ifaddrs_storage
{
  struct ifaddrs ifa;
  union
  {
    /* Save space for the biggest of the four used sockaddr types and
       avoid a lot of casts.  */
    struct sockaddr sa;
    struct sockaddr_ll sl;
    struct sockaddr_in s4;
    struct sockaddr_in6 s6;
  } addr, netmask, broadaddr;
  char name[IF_NAMESIZE + 1];
};


static void
free_netlink_handle (struct netlink_handle *h)
{
  struct netlink_res *ptr;
  int saved_errno = errno;

  ptr = h->nlm_list;
  while (ptr != NULL)
    {
      struct netlink_res *tmpptr;

      tmpptr = ptr->next;
      free (ptr);
      ptr = tmpptr;
    }

  errno = saved_errno;
}


static int
netlink_sendreq (struct netlink_handle *h, int type)
{
  struct
  {
    struct nlmsghdr nlh;
    struct rtgenmsg g;
  } req;
  struct sockaddr_nl nladdr;

  if (h->seq == 0)
    h->seq = time (NULL);

  req.nlh.nlmsg_len = sizeof (req);
  req.nlh.nlmsg_type = type;
  req.nlh.nlmsg_flags = NLM_F_ROOT | NLM_F_MATCH | NLM_F_REQUEST;
  req.nlh.nlmsg_pid = 0;
  req.nlh.nlmsg_seq = h->seq;
  req.g.rtgen_family = AF_UNSPEC;

  memset (&nladdr, '\0', sizeof (nladdr));
  nladdr.nl_family = AF_NETLINK;

  return TEMP_FAILURE_RETRY (sendto (h->fd, (void *) &req, sizeof (req), 0,
				     (struct sockaddr *) &nladdr,
				     sizeof (nladdr)));
}


static int
netlink_receive (struct netlink_handle *h)
{
  struct netlink_res *nlm_next;
  char buf[4096];
  struct iovec iov = { buf, sizeof (buf) };
  struct sockaddr_nl nladdr;
  struct nlmsghdr *nlmh;
  int read_len;
  bool done = false;

  while (! done)
    {
      struct msghdr msg =
	{
	  (void *) &nladdr, sizeof (nladdr),
	  &iov, 1,
	  NULL, 0,
	  0
	};

      read_len = TEMP_FAILURE_RETRY (recvmsg (h->fd, &msg, 0));
      if (read_len < 0)
	return -1;

      if (msg.msg_flags & MSG_TRUNC)
	return -1;

      nlm_next = (struct netlink_res *) malloc (sizeof (struct netlink_res)
						+ read_len);
      if (nlm_next == NULL)
	return -1;
      nlm_next->next = NULL;
      nlm_next->nlh = memcpy (nlm_next + 1, buf, read_len);
      nlm_next->size = read_len;
      nlm_next->seq = h->seq;
      if (h->nlm_list == NULL)
	{
	  h->nlm_list = nlm_next;
	  h->end_ptr = nlm_next;
	}
      else
	{
	  h->end_ptr->next = nlm_next;
	  h->end_ptr = nlm_next;
	}

      for (nlmh = (struct nlmsghdr *) buf;
	   NLMSG_OK (nlmh, (size_t) read_len);
	   nlmh = (struct nlmsghdr *) NLMSG_NEXT (nlmh, read_len))
	{
	  if ((pid_t) nlmh->nlmsg_pid != h->pid || nlmh->nlmsg_seq != h->seq)
	    continue;

	  if (nlmh->nlmsg_type == NLMSG_DONE)
	    {
	      /* We found the end, leave the loop.  */
	      done = true;
	      break;
	    }
	  if (nlmh->nlmsg_type == NLMSG_ERROR)
	    {
	      struct nlmsgerr *nlerr = (struct nlmsgerr *) NLMSG_DATA (nlmh);
	      if (nlmh->nlmsg_len < NLMSG_LENGTH (sizeof (struct nlmsgerr)))
		errno = EIO;
	      else
		errno = -nlerr->error;
	      return -1;
	    }
	}
    }
  return 0;
}


static void
netlink_close (struct netlink_handle *h)
{
  /* Don't modify errno.  */
  INTERNAL_SYSCALL_DECL (err);
  (void) INTERNAL_SYSCALL (close, err, 1, h->fd);
}


/* Open a NETLINK socket.  */
static int
netlink_open (struct netlink_handle *h)
{
  struct sockaddr_nl nladdr;

  h->fd = socket (PF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
  if (h->fd < 0)
    return -1;

  memset (&nladdr, '\0', sizeof (nladdr));
  nladdr.nl_family = AF_NETLINK;
  if (bind (h->fd, (struct sockaddr *) &nladdr, sizeof (nladdr)) < 0)
    {
      netlink_close (h);
      return -1;
    }
  return 0;
}


/* We know the number of RTM_NEWLINK entries, so we reserve the first
   # of entries for this type. All RTM_NEWADDR entries have an index
   pointer to the RTM_NEWLINK entry.  To find the entry, create
   a table to map kernel index entries to our index numbers.
   Since we get at first all RTM_NEWLINK entries, it can never happen
   that a RTM_NEWADDR index is not known to this map.  */
static int
map_newlink (int index, int *map, int max)
{
  int i;

  for (i = 0; i < max; i++)
    {
      if (map[i] == -1)
	{
	  map[i] = index;
	  return i;
	}
      else if (map[i] == index)
	return i;
    }
  /* This should never be reached. If this will be reached, we have
     a very big problem.  */
  abort ();
}


/* Create a linked list of `struct ifaddrs' structures, one for each
   network interface on the host machine.  If successful, store the
   list in *IFAP and return 0.  On errors, return -1 and set `errno'.  */
int
getifaddrs (struct ifaddrs **ifap)
{
  struct netlink_handle nh = { 0, 0, 0, NULL, NULL };
  struct netlink_res *nlp;
  struct ifaddrs_storage *ifas;
  unsigned int i, newlink, newaddr, newaddr_idx;
  int *map_newlink_data;
  size_t ifa_data_size = 0;  /* Size to allocate for all ifa_data.  */
  char *ifa_data_ptr;        /* Pointer to the unused part of memory for
				ifa_data.  */
  int result = 0;

  if (ifap)
    *ifap = NULL;

  if (! no_netlink_support && netlink_open (&nh) < 0)
    {
#if __ASSUME_NETLINK_SUPPORT == 0
       no_netlink_support = 1;
#else
       return -1;
#endif
    }

#if __ASSUME_NETLINK_SUPPORT == 0
  if (no_netlink_support)
    return fallback_getifaddrs (ifap);
#endif

  nh.pid = getpid ();

  /* Tell the kernel that we wish to get a list of all
     active interfaces.  */
  if (netlink_sendreq (&nh, RTM_GETLINK) < 0)
    {
      result = -1;
      goto exit_close;
    }
  /* Collect all data for every interface.  */
  if (netlink_receive (&nh) < 0)
    {
      result = -1;
      goto exit_free;
    }


  /* Now ask the kernel for all addresses which are assigned
     to an interface.  Since we store the addresses after the
     interfaces in the list, we will later always find the
     interface before the corresponding addresses.  */
  ++nh.seq;
  if (netlink_sendreq (&nh, RTM_GETADDR) < 0
      /* Collect all data for every interface.  */
      || netlink_receive (&nh) < 0)
    {
      result = -1;
      goto exit_free;
    }

  /* Count all RTM_NEWLINK and RTM_NEWADDR entries to allocate
     enough memory.  */
  newlink = newaddr = 0;
  for (nlp = nh.nlm_list; nlp; nlp = nlp->next)
    {
      struct nlmsghdr *nlh;
      size_t size = nlp->size;

      if (nlp->nlh == NULL)
	continue;

      /* Walk through all entries we got from the kernel and look, which
         message type they contain.  */
      for (nlh = nlp->nlh; NLMSG_OK (nlh, size); nlh = NLMSG_NEXT (nlh, size))
	{
	  /* check if the message is what we want */
	  if ((pid_t) nlh->nlmsg_pid != nh.pid || nlh->nlmsg_seq != nlp->seq)
	    continue;

	  if (nlh->nlmsg_type == NLMSG_DONE)
	    break;		/* ok */

	  if (nlh->nlmsg_type == RTM_NEWLINK)
	    {
	      /* A RTM_NEWLINK message can have IFLA_STATS data. We need to
		 know the size before creating the list to allocate enough
		 memory.  */
	      struct ifinfomsg *ifim = (struct ifinfomsg *) NLMSG_DATA (nlh);
	      struct rtattr *rta = IFLA_RTA (ifim);
	      size_t rtasize = IFLA_PAYLOAD (nlh);

	      while (RTA_OK (rta, rtasize))
		{
		  size_t rta_payload = RTA_PAYLOAD (rta);

		  if (rta->rta_type == IFLA_STATS)
		    {
		      ifa_data_size += rta_payload;
		      break;
		    }
		  else
		    rta = RTA_NEXT (rta, rtasize);
		}
	      ++newlink;
	    }
	  else if (nlh->nlmsg_type == RTM_NEWADDR)
	    ++newaddr;
	}
    }

  /* Return if no interface is up.  */
  if ((newlink + newaddr) == 0)
    goto exit_free;

  /* Table for mapping kernel index to entry in our list.  */
  map_newlink_data = alloca (newlink * sizeof (int));

  /* Allocate memory for all entries we have and initialize next
     pointer.  */
  ifas = (struct ifaddrs_storage *) calloc (1,
					    (newlink + newaddr)
					    * sizeof (struct ifaddrs_storage)
					    + ifa_data_size);
  if (ifas == NULL)
    {
      result = -1;
      goto exit_free;
    }

  for (i = 0; i < newlink + newaddr - 1; i++)
    {
      ifas[i].ifa.ifa_next = &ifas[i + 1].ifa;
      map_newlink_data[i] = -1;
    }
  ifa_data_ptr = (char *) &ifas[newlink + newaddr];
  newaddr_idx = 0;		/* Counter for newaddr index.  */

  /* Walk through the list of data we got from the kernel.  */
  for (nlp = nh.nlm_list; nlp; nlp = nlp->next)
    {
      struct nlmsghdr *nlh;
      size_t size = nlp->size;

      if (nlp->nlh == NULL)
	continue;

      /* Walk through one message and look at the type: If it is our
	 message, we need RTM_NEWLINK/RTM_NEWADDR and stop if we reach
	 the end or we find the end marker (in this case we ignore the
	 following data.  */
      for (nlh = nlp->nlh; NLMSG_OK (nlh, size); nlh = NLMSG_NEXT (nlh, size))
	{
	  int ifa_index = 0;

	  /* Check if the message is the one we want */
	  if ((pid_t) nlh->nlmsg_pid != nh.pid || nlh->nlmsg_seq != nlp->seq)
	    continue;

	  if (nlh->nlmsg_type == NLMSG_DONE)
	    break;		/* ok */

	  if (nlh->nlmsg_type == RTM_NEWLINK)
	    {
	      /* We found a new interface. Now extract everything from the
		 interface data we got and need.  */
	      struct ifinfomsg *ifim = (struct ifinfomsg *) NLMSG_DATA (nlh);
	      struct rtattr *rta = IFLA_RTA (ifim);
	      size_t rtasize = IFLA_PAYLOAD (nlh);

	      /* Interfaces are stored in the first "newlink" entries
		 of our list, starting in the order as we got from the
		 kernel.  */
              ifa_index = map_newlink (ifim->ifi_index - 1,
				       map_newlink_data, newlink);
	      ifas[ifa_index].ifa.ifa_flags = ifim->ifi_flags;

	      while (RTA_OK (rta, rtasize))
		{
		  char *rta_data = RTA_DATA (rta);
		  size_t rta_payload = RTA_PAYLOAD (rta);

		  switch (rta->rta_type)
		    {
		    case IFLA_ADDRESS:
		      ifas[ifa_index].addr.sl.sll_family = AF_PACKET;
		      memcpy (ifas[ifa_index].addr.sl.sll_addr,
			      (char *) rta_data, rta_payload);
		      ifas[ifa_index].addr.sl.sll_halen = rta_payload;
		      ifas[ifa_index].addr.sl.sll_ifindex = ifim->ifi_index;
		      ifas[ifa_index].addr.sl.sll_hatype = ifim->ifi_type;

		      ifas[ifa_index].ifa.ifa_addr = &ifas[ifa_index].addr.sa;
		      break;

		    case IFLA_BROADCAST:
		      ifas[ifa_index].broadaddr.sl.sll_family = AF_PACKET;
		      memcpy (ifas[ifa_index].broadaddr.sl.sll_addr,
			      (char *) rta_data, rta_payload);
		      ifas[ifa_index].broadaddr.sl.sll_halen = rta_payload;
		      ifas[ifa_index].broadaddr.sl.sll_ifindex
			= ifim->ifi_index;
		      ifas[ifa_index].broadaddr.sl.sll_hatype = ifim->ifi_type;

		      ifas[ifa_index].ifa.ifa_broadaddr
			= &ifas[ifa_index].broadaddr.sa;
		      break;

		    case IFLA_IFNAME:	/* Name of Interface */
		      if ((rta_payload + 1) <= sizeof (ifas[ifa_index].name))
			{
			  ifas[ifa_index].ifa.ifa_name = ifas[ifa_index].name;
			  strncpy (ifas[ifa_index].name, rta_data,
				   rta_payload);
			  ifas[ifa_index].name[rta_payload] = '\0';
			}
		      break;

		    case IFLA_STATS:	/* Statistics of Interface */
		      ifas[ifa_index].ifa.ifa_data = ifa_data_ptr;
		      ifa_data_ptr += rta_payload;
		      memcpy (ifas[ifa_index].ifa.ifa_data, rta_data,
			      rta_payload);
		      break;

		    case IFLA_UNSPEC:
		      break;
		    case IFLA_MTU:
		      break;
		    case IFLA_LINK:
		      break;
		    case IFLA_QDISC:
		      break;
		    default:
		      break;
		    }

		  rta = RTA_NEXT (rta, rtasize);
		}
	    }
	  else if (nlh->nlmsg_type == RTM_NEWADDR)
	    {
	      struct ifaddrmsg *ifam = (struct ifaddrmsg *) NLMSG_DATA (nlh);
	      struct rtattr *rta = IFA_RTA (ifam);
	      size_t rtasize = IFA_PAYLOAD (nlh);

	      /* New Addresses are stored in the order we got them from
		 the kernel after interfaces. Theoretically it is possible
		 that we have holes in the interface part of the list,
		 but we always have already the interface for this address.  */
	      ifa_index = newlink + newaddr_idx;
	      ifas[ifa_index].ifa.ifa_flags
		= ifas[map_newlink (ifam->ifa_index - 1,
				    map_newlink_data, newlink)].ifa.ifa_flags;
	      ++newaddr_idx;

	      while (RTA_OK (rta, rtasize))
		{
		  char *rta_data = RTA_DATA (rta);
		  size_t rta_payload = RTA_PAYLOAD (rta);

		  switch (rta->rta_type)
		    {
		    case IFA_ADDRESS:
		      {
			struct sockaddr *sa;

			if (ifas[ifa_index].ifa.ifa_addr != NULL)
			  {
			    /* In a point-to-poing network IFA_ADDRESS
			       contains the destination address, local
			       address is supplied in IFA_LOCAL attribute.
			       destination address and broadcast address
			       are stored in an union, so it doesn't matter
			       which name we use.  */
			    ifas[ifa_index].ifa.ifa_broadaddr
			      = &ifas[ifa_index].broadaddr.sa;
			    sa = &ifas[ifa_index].broadaddr.sa;
			  }
			else
			  {
			    ifas[ifa_index].ifa.ifa_addr
			      = &ifas[ifa_index].addr.sa;
			    sa = &ifas[ifa_index].addr.sa;
			  }

			sa->sa_family = ifam->ifa_family;

			switch (ifam->ifa_family)
			  {
			  case AF_INET:
			    memcpy (&((struct sockaddr_in *) sa)->sin_addr,
				    rta_data, rta_payload);
			    break;

			  case AF_INET6:
			    memcpy (&((struct sockaddr_in6 *) sa)->sin6_addr,
				    rta_data, rta_payload);
			    if (IN6_IS_ADDR_LINKLOCAL (rta_data) ||
				IN6_IS_ADDR_MC_LINKLOCAL (rta_data))
			      ((struct sockaddr_in6 *) sa)->sin6_scope_id =
				ifam->ifa_scope;
			    break;

			  default:
			    memcpy (sa->sa_data, rta_data, rta_payload);
			    break;
			  }
		      }
		      break;

		    case IFA_LOCAL:
		      if (ifas[ifa_index].ifa.ifa_addr != NULL)
			{
			  /* If ifa_addr is set and we get IFA_LOCAL,
			     assume we have a point-to-point network.
			     Move address to correct field.  */
			  ifas[ifa_index].broadaddr = ifas[ifa_index].addr;
			  ifas[ifa_index].ifa.ifa_broadaddr
			    = &ifas[ifa_index].broadaddr.sa;
			  memset (&ifas[ifa_index].addr, '\0',
				  sizeof (ifas[ifa_index].addr));
			}

		      ifas[ifa_index].ifa.ifa_addr = &ifas[ifa_index].addr.sa;
		      ifas[ifa_index].ifa.ifa_addr->sa_family
			= ifam->ifa_family;

		      switch (ifam->ifa_family)
			{
			case AF_INET:
			  memcpy (&ifas[ifa_index].addr.s4.sin_addr,
				  rta_data, rta_payload);
			  break;

			case AF_INET6:
			  memcpy (&ifas[ifa_index].addr.s6.sin6_addr,
				  rta_data, rta_payload);
			  if (IN6_IS_ADDR_LINKLOCAL (rta_data) ||
			      IN6_IS_ADDR_MC_LINKLOCAL (rta_data))
			    ifas[ifa_index].addr.s6.sin6_scope_id =
			      ifam->ifa_scope;
			  break;

			default:
			  memcpy (ifas[ifa_index].addr.sa.sa_data,
				  rta_data, rta_payload);
			  break;
			}
		      break;

		    case IFA_BROADCAST:
		      /* We get IFA_BROADCAST, so IFA_LOCAL was too much.  */
		      if (ifas[ifa_index].ifa.ifa_broadaddr != NULL)
			memset (&ifas[ifa_index].broadaddr, '\0',
				sizeof (ifas[ifa_index].broadaddr));

		      ifas[ifa_index].ifa.ifa_broadaddr
			= &ifas[ifa_index].broadaddr.sa;
		      ifas[ifa_index].ifa.ifa_broadaddr->sa_family
			= ifam->ifa_family;

		      switch (ifam->ifa_family)
			{
			case AF_INET:
			  memcpy (&ifas[ifa_index].broadaddr.s4.sin_addr,
				  rta_data, rta_payload);
			  break;

			case AF_INET6:
			  memcpy (&ifas[ifa_index].broadaddr.s6.sin6_addr,
				  rta_data, rta_payload);
			  if (IN6_IS_ADDR_LINKLOCAL (rta_data)
			      || IN6_IS_ADDR_MC_LINKLOCAL (rta_data))
			    ifas[ifa_index].broadaddr.s6.sin6_scope_id
			      = ifam->ifa_scope;
			  break;

			default:
			  memcpy (&ifas[ifa_index].broadaddr.sa.sa_data,
				  rta_data, rta_payload);
			  break;
			}
		      break;

		    case IFA_LABEL:
		      if (rta_payload + 1 <= sizeof (ifas[ifa_index].name))
			{
			  ifas[ifa_index].ifa.ifa_name = ifas[ifa_index].name;
			  strncpy (ifas[ifa_index].name, rta_data,
				   rta_payload);
			  ifas[ifa_index].name[rta_payload] = '\0';
			}
		      else
			abort ();
		      break;

		    case IFA_UNSPEC:
		      break;
		    case IFA_CACHEINFO:
		      break;
		    default:
		      break;
		    }

		  rta = RTA_NEXT (rta, rtasize);
		}

	      /* If we didn't get the interface name with the
		 address, use the name from the interface entry.  */
	      if (ifas[ifa_index].ifa.ifa_name == NULL)
		ifas[ifa_index].ifa.ifa_name
		  = ifas[map_newlink (ifam->ifa_index - 1,
				      map_newlink_data, newlink)].ifa.ifa_name;

	      /* Calculate the netmask.  */
	      if (ifas[ifa_index].ifa.ifa_addr
		  && ifas[ifa_index].ifa.ifa_addr->sa_family != AF_UNSPEC
		  && ifas[ifa_index].ifa.ifa_addr->sa_family != AF_PACKET)
		{
		  uint32_t max_prefixlen = 0;
		  char *cp = NULL;

		  ifas[ifa_index].ifa.ifa_netmask
		    = &ifas[ifa_index].netmask.sa;

		  switch (ifas[ifa_index].ifa.ifa_addr->sa_family)
		    {
		    case AF_INET:
		      cp = (char *) &ifas[ifa_index].netmask.s4.sin_addr;
		      max_prefixlen = 32;
		      break;

		    case AF_INET6:
		      cp = (char *) &ifas[ifa_index].netmask.s6.sin6_addr;
		      max_prefixlen = 128;
		      break;
		    }

		  ifas[ifa_index].ifa.ifa_netmask->sa_family
		    = ifas[ifa_index].ifa.ifa_addr->sa_family;

		  if (cp != NULL)
		    {
		      char c;
		      unsigned int preflen;

		      if ((max_prefixlen > 0) &&
			  (ifam->ifa_prefixlen > max_prefixlen))
			preflen = max_prefixlen;
		      else
			preflen = ifam->ifa_prefixlen;

		      for (i = 0; i < (preflen / 8); i++)
			*cp++ = 0xff;
		      c = 0xff;
		      c <<= (8 - (preflen % 8));
		      *cp = c;
		    }
		}
	    }
	}
    }

  if (ifap != NULL)
    *ifap = &ifas[0].ifa;

 exit_free:
  free_netlink_handle (&nh);

 exit_close:
  netlink_close (&nh);

  return result;
}


#if __ASSUME_NETLINK_SUPPORT != 0
void
freeifaddrs (struct ifaddrs *ifa)
{
  free (ifa);
}
#endif
