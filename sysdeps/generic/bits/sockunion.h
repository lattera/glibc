/* Definition of `sockaddr_union'.  Generic/4.2 BSD version.
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

/*
 * Never include this file directly; use <sys/socket.h> instead.
 */

#ifndef _BITS_SOCKUNION_H
#define _BITS_SOCKUNION_H	1

#include <netinet/in.h>
#include <sys/un.h>

/* Union of all sockaddr types (required by IPv6 Basic API).  */
union sockaddr_union
  {
    struct sockaddr sa;
    struct sockaddr_in sin;
    struct sockaddr_in6 sin6;
    struct sockaddr_un sun;
    char __maxsize[128];
  };

#endif	/* bits/sockunion.h */
