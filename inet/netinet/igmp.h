/* Copyright (C) 1997, 1999, 2003 Free Software Foundation, Inc.
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

#ifndef _NETINET_IGMP_H
#define	_NETINET_IGMP_H 1

#include <sys/cdefs.h>
#include <sys/types.h>

#define IGMP_HOST_MEMBERSHIP_QUERY      0x11    /* From RFC1112 */
#define IGMP_HOST_MEMBERSHIP_REPORT     0x12    /* Ditto */
#define IGMP_DVMRP                      0x13    /* DVMRP routing */
#define IGMP_PIM                        0x14    /* PIM routing */
#define IGMP_TRACE                      0x15
#define IGMP_HOST_NEW_MEMBERSHIP_REPORT 0x16    /* New version of 0x11 */
#define IGMP_HOST_LEAVE_MESSAGE         0x17

#define IGMP_MTRACE_RESP                0x1e
#define IGMP_MTRACE                     0x1f

/*
 *      Use the BSD names for these for compatibility
 */

#define IGMP_DELAYING_MEMBER            0x01
#define IGMP_IDLE_MEMBER                0x02
#define IGMP_LAZY_MEMBER                0x03
#define IGMP_SLEEPING_MEMBER            0x04
#define IGMP_AWAKENING_MEMBER           0x05

#define IGMP_MINLEN                     8

#define IGMP_MAX_HOST_REPORT_DELAY      10      /* max delay for response to */
                                                /* query (in seconds)   */

#define IGMP_TIMER_SCALE                10      /* denotes that the igmphdr->timer field */
                                                /* specifies time in 10th of seconds     */

#define IGMP_AGE_THRESHOLD              400     /* If this host don't hear any IGMP V1  */
                                                /* message in this period of time,      */
                                                /* revert to IGMP v2 router.            */

#define IGMP_ALL_HOSTS          htonl(0xE0000001L)
#define IGMP_ALL_ROUTER         htonl(0xE0000002L)
#define IGMP_LOCAL_GROUP        htonl(0xE0000000L)
#define IGMP_LOCAL_GROUP_MASK   htonl(0xFFFFFF00L)

#ifdef __USE_BSD

#include <netinet/in.h>

__BEGIN_DECLS

/*
 * Copyright (c) 1988 Stephen Deering.
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Stephen Deering of Stanford University.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)igmp.h	8.1 (Berkeley) 6/10/93
 *	$FreeBSD$
 */

struct igmp {
  u_int8_t igmp_type;             /* IGMP type */
  u_int8_t igmp_code;             /* routing code */
  u_int16_t igmp_cksum;           /* checksum */
  struct in_addr igmp_group;      /* group address */
};

/*
 * Message types, including version number.
 */
#define IGMP_MEMBERSHIP_QUERY   	0x11	/* membership query         */
#define IGMP_V1_MEMBERSHIP_REPORT	0x12	/* Ver. 1 membership report */
#define IGMP_V2_MEMBERSHIP_REPORT	0x16	/* Ver. 2 membership report */
#define IGMP_V2_LEAVE_GROUP		0x17	/* Leave-group message	    */

__END_DECLS

#endif

#endif	/* netinet/igmp.h */
