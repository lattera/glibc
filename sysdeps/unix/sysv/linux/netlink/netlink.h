/* Definitions for use with Linux AF_NETLINK sockets.
   Copyright (C) 1998 Free Software Foundation, Inc.
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

#ifndef __NETLINK_NETLINK_H
#define __NETLINK_NETLINK_H	1

#include <features.h>

#include <sys/types.h>
#include <bits/sockaddr.h>

__BEGIN_DECLS

struct sockaddr_nl
  {
    __SOCKADDR_COMMON (nl_);
    unsigned short nl_pad;		/* zero.  */
    uint32_t nl_pid;			/* process pid.  */
    uint32_t nl_groups;			/* multicast groups mask.  */
  };

#define NETLINK_ROUTE		0	/* Routing/device hook.  */
#define NETLINK_SKIP		1	/* Reserved for ENskip.  */
#define NETLINK_USERSOCK	2	/* Reserved for user mode
					   socket protocolss.  */
#define NETLINK_FIREWALL	3	/* Firewalling hook.  */
#define NETLINK_ARPD		8
#define NETLINK_ROUTE6		11	/* AF_INET6 route comm channel */
#define NETLINK_IP6_FW		13
#define NETLINK_TAPBASE		16	/* 16 to 31 are ethertap */

#define MAX_LINKS 32

struct nlmsghdr
  {
    uint32_t nlmsg_len;		/* Length of message including header */
    uint16_t nlmsg_type;	/* Message content */
    uint16_t nlmsg_flags;	/* Additional flags */
    uint32_t nlmsg_seq;		/* Sequence number */
    uint32_t nlmsg_pid;		/* Sending process PID */
  };

/* Flag bits */
#define NLM_F_REQUEST	1	/* Message is a request.  */
#define NLM_F_MULTI	2	/* Multipart message, terminated by
				   NLMSG_DONE.  */
#define NLM_F_ACK	4	/* If operation succeeds, reply with ack.  */
#define NLM_F_ECHO	8	/* Echo this request.  */

/* Modifiers to GET request */
#define NLM_F_ROOT	0x100	/* specify tree	root.  */
#define NLM_F_MATCH	0x200	/* return all matching.  */
#define NLM_F_ATOMIC	0x400	/* atomic GET.  */
#define NLM_F_DUMP	(NLM_F_ROOT|NLM_F_MATCH)

/* Modifiers to NEW request */
#define NLM_F_REPLACE	0x100	/* Override existing.  */
#define NLM_F_EXCL	0x200	/* Do not touch, if it exists.  */
#define NLM_F_CREATE	0x400	/* Create, if it does not exist.  */
#define NLM_F_APPEND	0x800	/* Add to end of list.  */

/*
   4.4BSD ADD		NLM_F_CREATE|NLM_F_EXCL
   4.4BSD CHANGE	NLM_F_REPLACE

   True CHANGE		NLM_F_CREATE|NLM_F_REPLACE
   Append		NLM_F_CREATE
   Check		NLM_F_EXCL
 */

#define NLMSG_ALIGNTO	4

#define NLMSG_ALIGN(len) \
	(((len) + NLMSG_ALIGNTO - 1) & ~(NLMSG_ALIGNTO - 1))

#define NLMSG_LENGTH(len) \
	((len) + NLMSG_ALIGN (sizeof (struct nlmsghdr)))

#define NLMSG_SPACE(len) \
	NLMSG_ALIGN (NLMSG_LENGTH (len))

#define NLMSG_DATA(nlh) \
	((void *) (((char *) nlh) + NLMSG_LENGTH (0)))

#define NLMSG_NEXT(nlh, len) \
	 ((len) -= NLMSG_ALIGN ((nlh)->nlmsg_len),			      \
	  (struct nlmsghdr *) (((char *) (nlh))				      \
			       + NLMSG_ALIGN ((nlh)->nlmsg_len)))

#define NLMSG_OK(nlh, len) \
	((len) > 0 && (nlh)->nlmsg_len >= sizeof (struct nlmsghdr)
	 && (nlh)->nlmsg_len <= (len))

#define NLMSG_PAYLOAD(nlh,len) \
	((nlh)->nlmsg_len - NLMSG_SPACE (len))

#define NLMSG_NOOP		0x1	/* Nothing.  */
#define NLMSG_ERROR		0x2	/* Error.  */
#define NLMSG_DONE		0x3	/* End of a dump.  */
#define NLMSG_OVERRUN		0x4	/* Data lost.  */

struct nlmsgerr
  {
    int	error;
    struct nlmsghdr msg;
  };

#define NET_MAJOR 36		/* Major 36 is reserved for networking 						*/

#endif
