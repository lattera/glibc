/* Copyright (C) 1997,98,99,2000,02 Free Software Foundation, Inc.
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

#include <net/if.h>
#include <errno.h>
#include <stddef.h>

unsigned int
if_nametoindex (const char *ifname)
{
  __set_errno (ENOSYS);
  return 0;
}
libc_hidden_def (if_nametoindex)
stub_warning (if_nametoindex)

char *
if_indextoname (unsigned int ifindex, char *ifname)
{
  __set_errno (ENOSYS);
  return NULL;
}
libc_hidden_def (if_indextoname)
stub_warning (if_indextoname)

void
if_freenameindex (struct if_nameindex *ifn)
{
}
stub_warning (if_freenameindex)

struct if_nameindex *
if_nameindex (void)
{
  __set_errno (ENOSYS);
  return NULL;
}
stub_warning (if_nameindex)
#include <stub-tag.h>

#if 0
void
internal_function
__protocol_available (int *have_inet, int *have_inet6)
{
  /* By default we assume that IPv4 is available, IPv6 not.  */
  *have_inet = 1;
  *have_inet6 = 0;
}
#endif
