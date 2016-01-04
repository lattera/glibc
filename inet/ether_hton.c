/* Copyright (C) 1996-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1996.

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

#include <errno.h>
#include <netinet/ether.h>
#include <netinet/if_ether.h>
#include <string.h>

#include "../nss/nsswitch.h"

/* Type of the lookup function we need here.  */
typedef int (*lookup_function) (const char *, struct etherent *, char *, int,
				int *);

/* The lookup function for the first entry of this service.  */
extern int __nss_ethers_lookup (service_user **nip, const char *name,
				void **fctp) internal_function;


int
ether_hostton (const char *hostname, struct ether_addr *addr)
{
  static service_user *startp;
  static lookup_function start_fct;
  service_user *nip;
  union
  {
    lookup_function f;
    void *ptr;
  } fct;
  int no_more;
  enum nss_status status = NSS_STATUS_UNAVAIL;
  struct etherent etherent;

  if (startp == NULL)
    {
      no_more = __nss_ethers_lookup (&nip, "gethostton_r", &fct.ptr);
      if (no_more)
	startp = (service_user *) -1;
      else
	{
	  startp = nip;
	  start_fct = fct.f;
	}
    }
  else
    {
      fct.f = start_fct;
      no_more = (nip = startp) == (service_user *) -1;
    }

  while (no_more == 0)
    {
      char buffer[1024];

      status = (*fct.f) (hostname, &etherent, buffer, sizeof buffer, &errno);

      no_more = __nss_next2 (&nip, "gethostton_r", NULL, &fct.ptr, status, 0);
    }

  if (status == NSS_STATUS_SUCCESS)
    memcpy (addr, etherent.e_addr.ether_addr_octet,
	    sizeof (struct ether_addr));

  return status == NSS_STATUS_SUCCESS ? 0 : -1;
}
