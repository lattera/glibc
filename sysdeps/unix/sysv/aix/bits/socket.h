/* System-specific socket constants and types.  AIX version.
   Copyright (C) 1991,92,1994-1999,2000,2001 Free Software Foundation, Inc.
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

#ifndef __BITS_SOCKET_H
#define __BITS_SOCKET_H

#if !defined _SYS_SOCKET_H && !defined _NETINET_IN_H
# error "Never include <bits/socket.h> directly; use <sys/socket.h> instead."
#endif

#define	__need_size_t
#define __need_NULL
#include <stddef.h>

#include <limits.h>
#include <sys/types.h>

/* Type for length arguments in socket calls.  */
#ifndef __socklen_t_defined
typedef __socklen_t socklen_t;
# define __socklen_t_defined
#endif

/* Types of sockets.  */
enum __socket_type
{
  SOCK_STREAM = 1,		/* Sequenced, reliable, connection-based
				   byte streams.  */
#define SOCK_STREAM SOCK_STREAM
  SOCK_DGRAM = 2,		/* Connectionless, unreliable datagrams
				   of fixed maximum length.  */
#define SOCK_DGRAM SOCK_DGRAM
  SOCK_RAW = 3,			/* Raw protocol interface.  */
#define SOCK_RAW SOCK_RAW
  SOCK_RDM = 4,			/* Reliably-delivered messages.  */
#define SOCK_RDM SOCK_RDM
  SOCK_SEQPACKET = 5,		/* Sequenced, reliable, connection-based,
				   datagrams of fixed maximum length.  */
#define SOCK_SEQPACKET SOCK_SEQPACKET
  SOCK_CONN_DGRAM = 6		/* Conneciton datagram.  */
#define SOCK_CONN_DGRAM	SOCK_CONN_DGRAM
};

/* Protocol families.  */
#define	PF_UNSPEC	0	/* Unspecified.  */
#define	PF_LOCAL	1	/* Local to host (pipes and file-domain).  */
#define	PF_UNIX		PF_LOCAL /* Old BSD name for PF_LOCAL.  */
#define	PF_FILE		PF_LOCAL /* Another non-standard name for PF_LOCAL.  */
#define	PF_INET		2	/* IP protocol family.  */
#define PF_IMPLINK	3	/* ARPAnet IMP addresses.  */
#define PF_PUP		4	/* PUP protocols (e.g., BSP).  */
#define PF_CHAOS	5	/* MIT CHAOS protocols.  */
#define PF_NS		6	/* XEROX NS protocols.  */
#define PF_ISO		7	/* ISO protocols.  */
#define PF_OSI		PF_ISO
#define PF_ECMA		8	/* European Computer Manufacturers.  */
#define PF_DATAKIT	9	/* Datakit protocols.  */
#define PF_CCITT	10	/* CCITT protocols, X.25 etc. */
#define PF_SNA		11	/* IBM SNA.  */
#define PF_DECnet	12	/* DECnet.  */
#define PF_DLI		13	/* DEC Direct data link interface.  */
#define PF_LAT		14	/* LAT. */
#define PF_HYLINK	15	/* NSC Hyperchannel.  */
#define PF_APPLETALK	16	/* Apple Talk.  */
#define PF_NETLINK	17	/* Internet Routing Protocol.  */
#define	PF_ROUTE	PF_NETLINK /* Alias to emulate 4.4BSD.  */
#define PF_LINK		18	/* Link layer interface.  */
#define PF_XTP		19	/* eXpress Transfer Protocol (no AF).  */
#define PF_INTF		20	/* Debugging use only.  */
#define PF_RIF		21	/* Raw interface.  */
#define PF_NETWARE	22
#define PF_NDD		23
#define PF_INET6	24	/* IPv6.  */
#define PF_MAX		30	/* For now..  */

/* Address families.  */
#define AF_UNSPEC       PF_UNSPEC
#define AF_LOCAL        PF_LOCAL
#define AF_UNIX         PF_UNIX
#define AF_FILE         PF_FILE
#define AF_INET         PF_INET
#define AF_IMPLINK      PF_IMPLINK
#define AF_PUP          PF_PUP
#define AF_CHAOS        PF_CHAOS
#define AF_NS           PF_NS
#define AF_ISO          PF_ISO
#define AF_OSI          PF_OSI
#define AF_ECMA         PF_ECMA
#define AF_DATAKIT      PF_DATAKIT
#define AF_CCITT        PF_CCITT
#define AF_SNA          PF_SNA
#define AF_DECnet       PF_DECnet
#define AF_DLI          PF_DLI
#define AF_LAT          PF_LAT
#define AF_HYLINK       PF_HYLINK
#define AF_APPLETALK    PF_APPLETALK
#define AF_NETLINK      PF_NETLINK
#define AF_ROUTE        PF_ROUTE
#define AF_LINK         PF_LINK
#define AF_INTF         PF_INTF
#define AF_RIF          PF_RIF
#define AF_NETWARE      PF_NETWARE
#define AF_NDD          PF_NDD
#define AF_INET6        PF_INET6
#define AF_MAX          PF_MAX

/* Socket level values.  Others are defined in the appropriate headers.

   XXX These definitions also should go into the appropriate headers as
   far as they are available.  */
#define SOL_SOCKET	0xffff

/* Maximum queue length specifiable by listen.  */
#define SOMAXCONN	1024

/* Get the definition of the macro to define the common sockaddr members.  */
#include <bits/sockaddr.h>

/* Structure describing a generic socket address.  */
struct sockaddr
  {
    __SOCKADDR_COMMON (sa_);	/* Common data: address family and length.  */
    char sa_data[14];		/* Address data.  */
  };


/* Structure large enough to hold any socket address (with the historical
   exception of AF_UNIX).  We reserve 128 bytes.  */
#if ULONG_MAX > 0xffffffff
# define __ss_aligntype	__uint64_t
#else
# define __ss_aligntype	__uint32_t
#endif
#define _SS_SIZE	128
#define _SS_PADSIZE	(_SS_SIZE - (2 * sizeof (__ss_aligntype)))

struct sockaddr_storage
  {
    __SOCKADDR_COMMON (ss_);	/* Address family, etc.  */
    __ss_aligntype __ss_align;	/* Force desired alignment.  */
    char __ss_padding[_SS_PADSIZE];
  };


/* Bits in the FLAGS argument to `send', `recv', et al.  */
enum
  {
    MSG_OOB		= 0x01,	/* Process out-of-band data.  */
#define MSG_OOB		MSG_OOB
    MSG_PEEK		= 0x02,	/* Peek at incoming messages.  */
#define MSG_PEEK	MSG_PEEK
    MSG_DONTROUTE	= 0x04,	/* Don't use local routing.  */
#define MSG_DONTROUTE	MSG_DONTROUTE
    MSG_EOR		= 0x08, /* End of record.  */
#define	MSG_EOR		MSG_EOR
    MSG_TRUNC		= 0x10,
#define	MSG_TRUNC	MSG_TRUNC
    MSG_CTRUNC		= 0x20,	/* Control data lost before delivery.  */
#define MSG_CTRUNC	MSG_CTRUNC
    MSG_WAITALL		= 0x40, /* Wait for a full request.  */
#define	MSG_WAITALL	MSG_WAITALL
    MSG_MPEG2		= 0x80,	/* Message contain MPEG2 data.  */
#define MSG_MPEG2	MSG_MPEG2
  };


/* Structure describing messages sent by
   `sendmsg' and received by `recvmsg'.  */
struct msghdr
  {
    void *msg_name;		/* Address to send to/receive from.  */
    socklen_t msg_namelen;	/* Length of address data.  */

    struct iovec *msg_iov;	/* Vector of data to send/receive into.  */
    int msg_iovlen;		/* Number of elements in the vector.  */

    void *msg_control;		/* Ancillary data (eg BSD filedesc passing). */
    socklen_t msg_controllen;	/* Ancillary data buffer length.  */

    int msg_flags;		/* Flags on received message.  */
  };

/* Structure used for storage of ancillary data object information.  */
struct cmsghdr
  {
    socklen_t cmsg_len;		/* Length of data in cmsg_data plus length
				   of cmsghdr structure.  */
    int cmsg_level;		/* Originating protocol.  */
    int cmsg_type;		/* Protocol specific type.  */
#if (!defined __STRICT_ANSI__ && __GNUC__ >= 2) || __STDC_VERSION__ >= 199901L
    __extension__ unsigned char __cmsg_data __flexarr; /* Ancillary data.  */
#endif
  };

/* Ancillary data object manipulation macros.  */
#if (!defined __STRICT_ANSI__ && __GNUC__ >= 2) || __STDC_VERSION__ >= 199901L
# define CMSG_DATA(cmsg) ((cmsg)->__cmsg_data)
#else
# define CMSG_DATA(cmsg) ((unsigned char *) ((struct cmsghdr *) (cmsg) + 1))
#endif
#define CMSG_NXTHDR(mhdr, cmsg) __cmsg_nxthdr (mhdr, cmsg)
#define CMSG_FIRSTHDR(mhdr) \
  ((size_t) (mhdr)->msg_controllen >= sizeof (struct cmsghdr)		      \
   ? (struct cmsghdr *) (mhdr)->msg_control : (struct cmsghdr *) NULL)
#define CMSG_ALIGN(len) (((len) + sizeof (size_t) - 1) \
			 & ~(sizeof (size_t) - 1))
#define CMSG_SPACE(len) (CMSG_ALIGN (len) \
			 + CMSG_ALIGN (sizeof (struct cmsghdr)))
#define CMSG_LEN(len)   (CMSG_ALIGN (sizeof (struct cmsghdr)) + (len))

extern struct cmsghdr *__cmsg_nxthdr (struct msghdr *__mhdr,
				      struct cmsghdr *__cmsg) __THROW;
#ifdef __USE_EXTERN_INLINES
# ifndef _EXTERN_INLINE
#  define _EXTERN_INLINE extern __inline
# endif
_EXTERN_INLINE struct cmsghdr *
__cmsg_nxthdr (struct msghdr *__mhdr, struct cmsghdr *__cmsg) __THROW
{
  if ((size_t) __cmsg->cmsg_len < sizeof (struct cmsghdr))
    /* The kernel header does this so there may be a reason.  */
    return 0;

  __cmsg = (struct cmsghdr *) ((unsigned char *) __cmsg
			       + CMSG_ALIGN (__cmsg->cmsg_len));
  if ((unsigned char *) (__cmsg + 1) >= ((unsigned char *) __mhdr->msg_control
					 + __mhdr->msg_controllen)
      || ((unsigned char *) __cmsg + CMSG_ALIGN (__cmsg->cmsg_len)
	  > ((unsigned char *) __mhdr->msg_control + __mhdr->msg_controllen)))
    /* No more entries.  */
    return 0;
  return __cmsg;
}
#endif	/* Use `extern inline'.  */

/* Socket level message types.  This must match the definitions in
   <linux/socket.h>.  */
enum
  {
    SCM_RIGHTS = 0x01		/* Transfer file descriptors.  */
#define SCM_RIGHTS SCM_RIGHTS
  };

/* Options flags per socket.  */
#define SO_DEBUG	0x0001	/* Turn on debugging info recording.  */
#define SO_ACCEPTCONN	0x0002	/* Socket has had listen().  */
#define SO_REUSEADDR	0x0004	/* Allow local address reuse.  */
#define SO_KEEPALIVE	0x0008	/* Keep connections alive.  */
#define SO_DONTROUTE	0x0010	/* Just use interface addresses.  */
#define SO_BROADCAST	0x0020	/* Permit sending of broadcast msgs.  */
#define SO_USELOOPBACK	0x0040	/* Bypass hardware when possible.  */
#define SO_LINGER	0x0080	/* Linger on close if data present.  */
#define SO_OOBINLINE	0x0100	/* Leave received OOB data in line.  */
#define SO_REUSEPORT	0x0200	/* Allow local address & port reuse.  */
#define SO_USE_IFBUFS	0x0400	/* Interface will supply buffers.  */
#define SO_CKSUMRECV	0x0800	/* Defer checksum until receive.  */
#define SO_NOREUSEADDR	0x1000	/* Prevent local address reuse.  */
#define SO_SNDBUF	0x1001	/* Send buffer size.  */
#define SO_RCVBUF	0x1002	/* Receive buffer size.  */
#define SO_SNDLOWAT	0x1003	/* Send low-water mark.  */
#define SO_RCVLOWAT	0x1004	/* Receive low-water mark.  */
#define SO_SNDTIMEO	0x1005	/* Send timeout.  */
#define SO_RCVTIMEO	0x1006	/* Receive timeout.  */
#define SO_ERROR	0x1007	/* Get error status and clear.  */
#define SO_TYPE		0x1008	/* Get socket type.  */
#define SO_KERNACCEPT	0x2000	/* Derive a in-kernel only socket.  */
#define SO_AUDIT	0x8000	/* Turn on socket auditing.  */


/* Structure used to manipulate the SO_LINGER option.  */
struct linger
  {
    int l_onoff;		/* Nonzero to linger on close.  */
    int l_linger;		/* Time to linger.  */
  };

#endif	/* bits/socket.h */
