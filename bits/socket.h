/* System-specific socket constants and types.  Generic/4.3 BSD version.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef __BITS_SOCKET_H
#define __BITS_SOCKET_H	1

#if !defined _SYS_SOCKET_H && !defined _NETINET_IN_H
# error "Never include <bits/socket.h> directly; use <sys/socket.h> instead."
#endif

#include <limits.h>
#include <bits/types.h>

#define	__need_size_t
#include <stddef.h>

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
  SOCK_SEQPACKET = 5		/* Sequenced, reliable, connection-based,
				   datagrams of fixed maximum length.  */
#define SOCK_SEQPACKET SOCK_SEQPACKET
};

/* Protocol families.  */
#define	PF_UNSPEC	0	/* Unspecified.  */
#define	PF_LOCAL	1	/* Local to host (pipes and file-domain).  */
#define	PF_UNIX		PF_LOCAL /* Old BSD name for PF_LOCAL.  */
#define	PF_FILE		PF_LOCAL /* POSIX name for PF_LOCAL.  */
#define	PF_INET		2	/* IP protocol family.  */
#define	PF_IMPLINK	3	/* ARPAnet IMP protocol.  */
#define	PF_PUP		4	/* PUP protocols.  */
#define	PF_CHAOS	5	/* MIT Chaos protocols.  */
#define	PF_NS		6	/* Xerox NS protocols.  */
#define	PF_ISO		7	/* ISO protocols.  */
#define	PF_OSI		PF_ISO
#define	PF_ECMA		8	/* ECMA protocols.  */
#define	PF_DATAKIT	9	/* AT&T Datakit protocols.  */
#define	PF_CCITT	10	/* CCITT protocols (X.25 et al).  */
#define	PF_SNA		11	/* IBM SNA protocol.  */
#define	PF_DECnet	12	/* DECnet protocols.  */
#define	PF_DLI		13	/* Direct data link interface.  */
#define	PF_LAT		14	/* DEC Local Area Transport protocol.  */
#define	PF_HYLINK	15	/* NSC Hyperchannel protocol.  */
#define	PF_APPLETALK	16	/* Don't use this.  */
#define	PF_ROUTE	17	/* Internal Routing Protocol.  */
#define	PF_LINK		18	/* Link layer interface.  */
#define	PF_XTP		19	/* eXpress Transfer Protocol (no AF).  */
#define	PF_COIP		20	/* Connection-oriented IP, aka ST II.  */
#define	PF_CNT		21	/* Computer Network Technology.  */
#define PF_RTIP		22	/* Help Identify RTIP packets.  **/
#define	PF_IPX		23	/* Novell Internet Protocol.  */
#define	PF_SIP		24	/* Simple Internet Protocol.  */
#define PF_PIP		25	/* Help Identify PIP packets.  */
#define PF_INET6	26	/* IP version 6.  */
#define	PF_MAX		27

/* Address families.  */
#define	AF_UNSPEC	PF_UNSPEC
#define	AF_LOCAL	PF_LOCAL
#define	AF_UNIX		PF_UNIX
#define	AF_FILE		PF_FILE
#define	AF_INET		PF_INET
#define	AF_IMPLINK	PF_IMPLINK
#define	AF_PUP		PF_PUP
#define	AF_CHAOS	PF_CHAOS
#define	AF_NS		PF_NS
#define	AF_ISO		PF_ISO
#define	AF_OSI		PF_OSI
#define	AF_ECMA		PF_ECMA
#define	AF_DATAKIT	PF_DATAKIT
#define	AF_CCITT	PF_CCITT
#define	AF_SNA		PF_SNA
#define	AF_DECnet	PF_DECnet
#define	AF_DLI		PF_DLI
#define	AF_LAT		PF_LAT
#define	AF_HYLINK	PF_HYLINK
#define	AF_APPLETALK	PF_APPLETALK
#define	AF_ROUTE	PF_ROUTE
#define	AF_LINK		PF_LINK
#define	pseudo_AF_XTP	PF_XTP
#define	AF_COIP		PF_COIP
#define	AF_CNT		PF_CNT
#define pseudo_AF_RTIP	PF_RTIP
#define	AF_IPX		PF_IPX
#define	AF_SIP		PF_SIP
#define pseudo_AF_PIP	PF_PIP
#define AF_INET6	PF_INET6
#define	AF_MAX		PF_MAX


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
#define MSG_OOB MSG_OOB
    MSG_PEEK		= 0x02,	/* Peek at incoming messages.  */
#define MSG_PEEK MSG_PEEK
    MSG_DONTROUTE	= 0x04,	/* Don't use local routing.  */
#define MSG_DONTROUTE MSG_DONTROUTE
    MSG_EOR		= 0x08,	/* Data completes record.  */
#define MSG_EOR MSG_EOR
    MSG_TRUNC		= 0x10,	/* Data discarded before delivery.  */
#define MSG_TRUNC MSG_TRUNC
    MSG_CTRUNC		= 0x20,	/* Control data lost before delivery.  */
#define MSG_CTRUNC MSG_CTRUNC
    MSG_WAITALL		= 0x40,	/* Wait for full request or error.  */
#define MSG_WAITALL MSG_WAITALL
    MSG_DONTWAIT	= 0x80	/* This message should be nonblocking.  */
#define MSG_DONTWAIT MSG_DONTWAIT
  };


/* Structure describing messages sent by
   `sendmsg' and received by `recvmsg'.  */
struct msghdr
  {
    __ptr_t msg_name;		/* Address to send to/receive from.  */
    socklen_t msg_namelen;	/* Length of address data.  */

    struct iovec *msg_iov;	/* Vector of data to send/receive into.  */
    int msg_iovlen;		/* Number of elements in the vector.  */

    __ptr_t msg_accrights;	/* Access rights information.  */
    socklen_t msg_accrightslen;	/* Length of access rights information.  */

    int msg_flags;		/* Flags in received message.  */
  };


/* Protocol number used to manipulate socket-level options
   with `getsockopt' and `setsockopt'.  */
#define	SOL_SOCKET	0xffff

/* Socket-level options for `getsockopt' and `setsockopt'.  */
enum
  {
    SO_DEBUG = 0x0001,		/* Record debugging information.  */
#define SO_DEBUG SO_DEBUG
    SO_ACCEPTCONN = 0x0002,	/* Accept connections on socket.  */
#define SO_ACCEPTCONN SO_ACCEPTCONN
    SO_REUSEADDR = 0x0004,	/* Allow reuse of local addresses.  */
#define SO_REUSEADDR SO_REUSEADDR
    SO_KEEPALIVE = 0x0008,	/* Keep connections alive and send
				   SIGPIPE when they die.  */
#define SO_KEEPALIVE SO_KEEPALIVE
    SO_DONTROUTE = 0x0010,	/* Don't do local routing.  */
#define SO_DONTROUTE SO_DONTROUTE
    SO_BROADCAST = 0x0020,	/* Allow transmission of
				   broadcast messages.  */
#define SO_BROADCAST SO_BROADCAST
    SO_USELOOPBACK = 0x0040,	/* Use the software loopback to avoid
				   hardware use when possible.  */
#define SO_USELOOPBACK SO_USELOOPBACK
    SO_LINGER = 0x0080,		/* Block on close of a reliable
				   socket to transmit pending data.  */
#define SO_LINGER SO_LINGER
    SO_OOBINLINE = 0x0100,	/* Receive out-of-band data in-band.  */
#define SO_OOBINLINE SO_OOBINLINE
    SO_REUSEPORT = 0x0200,	/* Allow local address and port reuse.  */
#define SO_REUSEPORT SO_REUSEPORT
    SO_SNDBUF = 0x1001,		/* Send buffer size.  */
#define SO_SNDBUF SO_SNDBUF
    SO_RCVBUF = 0x1002,		/* Receive buffer.  */
#define SO_RCVBUF SO_RCVBUF
    SO_SNDLOWAT = 0x1003,	/* Send low-water mark.  */
#define SO_SNDLOWAT SO_SNDLOWAT
    SO_RCVLOWAT = 0x1004,	/* Receive low-water mark.  */
#define SO_RCVLOWAT SO_RCVLOWAT
    SO_SNDTIMEO = 0x1005,	/* Send timeout.  */
#define SO_SNDTIMEO SO_SNDTIMEO
    SO_RCVTIMEO = 0x1006,	/* Receive timeout.  */
#define SO_RCVTIMEO SO_RCVTIMEO
    SO_ERROR = 0x1007,		/* Get and clear error status.  */
#define SO_ERROR SO_ERROR
    SO_STYLE = 0x1008,		/* Get socket connection style.  */
#define SO_STYLE SO_STYLE
    SO_TYPE = SO_STYLE		/* Compatible name for SO_STYLE.  */
#define SO_TYPE SO_TYPE
  };

/* Structure used to manipulate the SO_LINGER option.  */
struct linger
  {
    int l_onoff;		/* Nonzero to linger on close.  */
    int l_linger;		/* Time to linger.  */
  };

#endif	/* bits/socket.h */
