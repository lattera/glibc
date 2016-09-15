/* Network-related functions for internal library use.
   Copyright (C) 2016 Free Software Foundation, Inc.
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

#ifndef _NET_INTERNAL_H
#define _NET_INTERNAL_H 1

#include <arpa/inet.h>
#include <stdint.h>

int __inet6_scopeid_pton (const struct in6_addr *address,
                          const char *scope, uint32_t *result)
  internal_function attribute_hidden;
libc_hidden_proto (__inet6_scopeid_pton)

#endif /* _NET_INTERNAL_H */
