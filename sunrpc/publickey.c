/* Get public or secret key from key server.
   Copyright (C) 1996, 1997, 1998, 1999 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1996.

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

#include <errno.h>
#include <rpc/netdb.h>
#include <rpc/auth_des.h>

#include "nsswitch.h"


/* Type of the lookup function for the public key.  */
typedef int (*public_function) (const char *, char *, int *);

/* Type of the lookup function for the secret key.  */
typedef int (*secret_function) (const char *, char *, const char *, int *);

/* The lookup function for the first entry of this service.  */
extern int __nss_publickey_lookup (service_user **nip, const char *name,
				   void **fctp);


int
getpublickey (const char *name, char *key)
{
  static service_user *startp;
  static public_function start_fct;
  service_user *nip;
  public_function fct;
  enum nss_status status = NSS_STATUS_UNAVAIL;
  int no_more;

  if (startp == NULL)
    {
      no_more = __nss_publickey_lookup (&nip, "getpublickey", (void **) &fct);
      if (no_more)
	startp = (service_user *) -1;
      else
	{
	  startp = nip;
	  start_fct = fct;
	}
    }
  else
    {
      fct = start_fct;
      no_more = (nip = startp) == (service_user *) -1;
    }

  while (! no_more)
    {
      status = (*fct) (name, key, &errno);

      no_more = __nss_next (&nip, "getpublickey", (void **) &fct, status, 0);
    }

  return status == NSS_STATUS_SUCCESS;
}


int
getsecretkey (const char *name, char *key, const char *passwd)
{
  static service_user *startp;
  static secret_function start_fct;
  service_user *nip;
  secret_function fct;
  enum nss_status status = NSS_STATUS_UNAVAIL;
  int no_more;

  if (startp == NULL)
    {
      no_more = __nss_publickey_lookup (&nip, "getsecretkey", (void **) &fct);
      if (no_more)
	startp = (service_user *) -1;
      else
	{
	  startp = nip;
	  start_fct = fct;
	}
    }
  else
    {
      fct = start_fct;
      no_more = (nip = startp) == (service_user *) -1;
    }

  while (! no_more)
    {
      status = (*fct) (name, key, passwd, &errno);

      no_more = __nss_next (&nip, "getsecretkey", (void **) &fct, status, 0);
    }

  return status == NSS_STATUS_SUCCESS;
}
