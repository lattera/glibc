/* Copyright (C) 1997 Free Software Foundation, Inc.
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

#ifndef _NETINET_UDP_H
#define _NETINET_UDP_H	1

#include <sys/types.h>

/* The Internet RFC 768 specifies this format for the UDP protocol.  */
struct udphdr
  {
    u_short uh_sport;		/* Source port.  */
    u_short uh_dport;		/* Destination port.  */
    u_short uh_ulen;		/* UDP length.  */
    u_short uh_sum;		/* UDP checksum.  */
  };

#define SOL_UDP		17	/* UDP level.  */

#endif /* netinet/udp.h */
