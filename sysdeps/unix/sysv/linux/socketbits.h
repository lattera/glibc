/* System-specific socket constants and types.  Linux version.
   Copyright (C) 1991, 92, 94, 95, 96, 97 Free Software Foundation, Inc.
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

#ifndef	_SOCKETBITS_H

#define	_SOCKETBITS_H	1
#include <features.h>

#define	__need_size_t
#include <stddef.h>


__BEGIN_DECLS

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
  SOCK_PACKET = 10		/* Linux specific way of getting packets
				   at the dev level.  For writing rarp and
				   other similar things on the user level. */
#define SOCK_PACKET SOCK_PACKET
};

/* Protocol families.  */
#define	PF_UNSPEC	0	/* Unspecified.  */
#define	PF_LOCAL	1	/* Local to host (pipes and file-domain).  */
#define	PF_UNIX		PF_LOCAL /* Old BSD name for PF_LOCAL.  */
#define	PF_INET		2	/* IP protocol family.  */
#define	PF_AX25		3	/* Amateur Radio AX.25.  */
#define	PF_IPX		4	/* Novell Internet Protocol.  */
#define	PF_APPLETALK	5	/* Don't use this.  */
#define	PF_NETROM	6	/* Amateur radio NetROM.  */
#define	PF_BRIDGE	7	/* Multiprotocol bridge.  */
#define	PF_AAL5		8	/* Reserved for Werner's ATM.  */
#define	PF_X25		9	/* Reserved for X.25 project.  */
#define	PF_INET6	10	/* IP version 6.  */
#define	PF_MAX		12	/* For now.. */

/* Address families.  */
#define	AF_UNSPEC	PF_UNSPEC
#define	AF_LOCAL	PF_LOCAL
#define	AF_UNIX		PF_UNIX
#define	AF_INET		PF_INET
#define	AF_AX25		PF_AX25
#define	AF_IPX		PF_IPX
#define	AF_APPLETALK	PF_APPLETALK
#define	AF_NETROM	PF_NETROM
#define	AF_BRIDGE	PF_BRIDGE
#define	AF_AAL5		PF_AAL5
#define	AF_X25		PF_X25
#define	AF_INET6	PF_INET6
#define	AF_MAX		PF_MAX

/* Raw IP packet level.  */
#define SOL_RAW		255

/* Maximum queue length specifiable by listen.  */
#define SOMAXCONN	128

/* Get the definition of the macro to define the common sockaddr members.  */
#include <sockaddrcom.h>

/* Structure describing a generic socket address.  */
struct sockaddr
  {
    __SOCKADDR_COMMON (sa_);	/* Common data: address family and length.  */
    char sa_data[14];		/* Address data.  */
  };


/* Bits in the FLAGS argument to `send', `recv', et al.  */
enum
  {
    MSG_OOB		= 0x01,	/* Process out-of-band data.  */
    MSG_PEEK		= 0x02,	/* Peek at incoming messages.  */
    MSG_DONTROUTE	= 0x04,	/* Don't use local routing.  */
    MSG_CTRUNC		= 0x08,	/* Control data lost before delivery.  */
    MSG_PROXY		= 0x10	/* Supply or ask second address.  */
  };


/* Structure describing messages sent by
   `sendmsg' and received by `recvmsg'.  */
struct msghdr
  {
    __ptr_t msg_name;		/* Address to send to/receive from.  */
    int msg_namelen;		/* Length of address data.  */
    /* XXX Should be type `size_t' according to POSIX.1g.  */

    struct iovec *msg_iov;	/* Vector of data to send/receive into.  */
    int msg_iovlen;		/* Number of elements in the vector.  */
    /* XXX Should be type `size_t' according to POSIX.1g.  */

    __ptr_t msg_control;	/* Ancillary data (eg BSD filedesc passing). */
    int msg_controllen;		/* Ancillary data buffer length.  */
    /* XXX Should be type `size_t' according to POSIX.1g.  */
    int msg_flags;		/* Flags on received message.  */
  };


/* Get socket manipulation related informations from kernel headers.  */
#include <asm/socket.h>


/* Structure used to manipulate the SO_LINGER option.  */
struct linger
  {
    int l_onoff;		/* Nonzero to linger on close.  */
    int l_linger;		/* Time to linger.  */
  };

__END_DECLS

#endif /* socketbits.h */
