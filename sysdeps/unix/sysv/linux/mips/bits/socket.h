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

#ifndef _SYS_STAT_H
# error "Never include <bits/socket.h> directly; use <sys/socket.h> instead."
#endif

#define	__need_size_t
#define __need_NULL
#include <stddef.h>

/* Type for length arguments in socket calls.  */
typedef unsigned int socklen_t;

/* Supported address families. */
#define PF_UNSPEC	0
#define PF_UNIX		1		/* Unix domain sockets 		*/
#define PF_LOCAL	1		/* POSIX name for AF_UNIX	*/
#define PF_FILE		PF_LOCAL	/* POSIX name for PF_LOCAL.	*/
#define PF_INET		2		/* Internet IP Protocol 	*/
#define PF_AX25		3		/* Amateur Radio AX.25 		*/
#define PF_IPX		4		/* Novell IPX 			*/
#define PF_APPLETALK	5		/* Appletalk DDP 		*/
#define PF_NETROM	6		/* Amateur Radio NET/ROM 	*/
#define PF_BRIDGE	7		/* Multiprotocol bridge 	*/
#define PF_AAL5		8		/* Reserved for Werner's ATM 	*/
#define PF_X25		9		/* Reserved for X.25 project 	*/
#define PF_INET6	10		/* IP version 6			*/
#define PF_ROSE		11		/* Amateur Radio X.25 PLP	*/
#define PF_DECNET	12		/* Reserved for DECnet project	*/
#define PF_NETBEUI	13		/* Reserved for 802.2LLC project*/
#define PF_MAX		32		/* For now.. */

/* Protocol families, same as address families. */
#define AF_UNSPEC	PF_UNSPEC
#define AF_UNIX		PF_UNIX
#define AF_LOCAL	PF_LOCAL
#define AF_FILE		PF_FILE
#define AF_INET		PF_INET
#define AF_AX25		PF_AX25
#define AF_IPX		PF_IPX
#define AF_APPLETALK	PF_APPLETALK
#define	AF_NETROM	PF_NETROM
#define AF_BRIDGE	PF_BRIDGE
#define AF_AAL5		PF_AAL5
#define AF_X25		PF_X25
#define AF_INET6	PF_INET6
#define AF_ROSE		PF_ROSE
#define AF_DECNET	PF_DECNET
#define AF_NETBEUI	PF_NETBEUI

#define AF_MAX		PF_MAX

/* Raw IP packet level.  */
#define SOL_RAW		255

/* Maximum queue length specifiable by listen.  */
#define SOMAXCONN	128

/* Get the definition of the macro to define the common sockaddr members.  */
#include <bits/sockaddr.h>

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
    socklen_t msg_namelen;	/* Length of address data.  */

    struct iovec *msg_iov;	/* Vector of data to send/receive into.  */
    int msg_iovlen;		/* Number of elements in the vector.  */

    __ptr_t msg_control;	/* Ancillary data (eg BSD filedesc passing). */
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
#if !defined __STRICT_ANSI__ && defined __GNUC__ && __GNUC__ >= 2
    unsigned char __cmsg_data[0]; /* Ancillary data.  */
#endif
  };

/* Ancillary data object manipulation macros.  */
#if !defined __STRICT_ANSI__ && defined __GNUC__ && __GNUC__ >= 2
# define CMSG_DATA(cmsg) ((cmsg)->__cmsg_data)
#else
# define CMSG_DATA(cmsg) ((unsigned char *) ((struct cmsghdr *) (cmsg) + 1))
#endif
#define CMSG_NXTHDR(mhdr, cmsg) __cmsg_nxthdr (mhdr, cmsg)
#define CMSG_FIRSTHDR(mhdr) \
  ((size_t) (mhdr)->msg_controllen >= sizeof (struct cmsghdr)			      \
   ? (struct cmsghdr *) (mhdr)->msg_control : (struct cmsghdr *) NULL)


#ifndef _EXTERN_INLINE
# define _EXTERN_INLINE extern __inline
#endif
extern struct cmsghdr *__cmsg_nxthdr __P ((struct msghdr *__mhdr,
					   struct cmsghdr *__cmsg));
_EXTERN_INLINE struct cmsghdr *
__cmsg_nxthdr (struct msghdr *__mhdr, struct cmsghdr *__cmsg)
{
  unsigned char *__p;

  if ((size_t) __cmsg->cmsg_len < sizeof (struct cmsghdr))
    /* The kernel header does this so there may be a reason.  */
    return NULL;

  __p = (((unsigned char *) __cmsg)
	 + ((__cmsg->cmsg_len + sizeof (long int) - 1) & ~sizeof (long int)));
  if (__p >= (unsigned char *) __mhdr->msg_control + __mhdr->msg_controllen)
    /* No more entries.  */
    return NULL;
  return (struct cmsghdr *) __p;
}


/* Get socket manipulation related informations from kernel headers.  */
#include <asm/socket.h>


/* Structure used to manipulate the SO_LINGER option.  */
struct linger
  {
    int l_onoff;		/* Nonzero to linger on close.  */
    int l_linger;		/* Time to linger.  */
  };
