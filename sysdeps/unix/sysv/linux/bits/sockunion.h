/* Definition of `sockaddr_union'.  Linux version.
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

#ifndef _SYS_SOCKET_H
# error "Never include <bits/sockunion.h> directly; use <sys/socket.h> instead."
#endif

#include <netash/ash.h>
#include <netatalk/at.h>
#include <netax25/ax25.h>
#include <neteconet/ec.h>
#include <netinet/in.h>
#include <netipx/ipx.h>
#include <netrose/rose.h>
#include <sys/un.h>

/* Union of all sockaddr types (required by IPv6 Basic API).  This is
   somewhat evil.  */
union sockaddr_union
  {
    struct sockaddr sa;
    struct sockaddr_ash sash;
    struct sockaddr_at sat;
    struct sockaddr_ax25 sax25;
    struct sockaddr_ec sec;
    struct sockaddr_in sin;
    struct sockaddr_in6 sin6;
    struct sockaddr_ipx sipx;
    struct sockaddr_rose rose;
    struct sockaddr_un sun;
    char __maxsize[128];
  };
