/* Copyright (C) 1991, 92, 93, 94, 95, 96, 97 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#ifndef _NETINET_IP6_H
#define _NETINET_IP6_H 1

#include <netinet/in.h>
#include <endian.h>

struct ipv6hdr
{
#if __BYTE_ORDER == __LITTLE_ENDIAN
  u_int8_t ipv6_version:4;
  u_int8_t ipv6_priority:4; /* going away? */
  u_int32_t ipv6_flowid:24;
#elif __BYTE_ORDER == __BIG_ENDIAN
  u_int32_t ipv6_flowid:24;
  u_int8_t ipv6_priority:4; /* going away? */
  u_int8_t ipv6_version:4;
#else
# error  Unknown endianness
#endif
  u_int16_t ipv6_len;
  u_int8_t ipv6_nextheader;
  u_int8_t ipv6_hoplimit;
  struct in6_addr ipv6_src;
  struct in6_addr ipv6_dst;
};

#endif /* netinet/ip6.h */
