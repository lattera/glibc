/* Copyright (C) 1999 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1999.

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

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#include "nsswitch.h"


/* We avoid using a too large buffer in case the user is accumulating the
   results and there is lots of unused space in the allocated buffer.  */
#define BUFLEN	512


/* Internal function to test whether IPv4 and/or IPv6 are available.  */
extern void __protocol_available (int *have_inet, int *have_inet6)
     internal_function;


/* Type of the lookup function we need here.  */
typedef enum nss_status (*lookup_function) (const char *, int, int,
					    struct hostent *, char *,
					    size_t, int *, int *);

/* The lookup function for the first entry of this service.  */
extern int __nss_hosts_lookup (service_user **nip, const char *name,
			       void **fctp);


struct hostent *
getipnodebyname (const char *name, int type, int flags, int *h_errnop)
{
  static service_user *startp;
  static lookup_function start_fct;
  service_user *nip;
  lookup_function fct;
  int no_more;
  size_t buffer_size;
  char *buffer;
  struct hostent *result;
  int save;
  int have_inet = 1;
  int have_inet6 = 1;

  /* First determine whether we have the appropriate interfaces.
     XXX I don't think we can cache the result since the system
     can be reconfigured.  */
  if ((flags & AI_ADDRCONFIG) != 0)
    __protocol_available (&have_inet, &have_inet6);

  /* Handle impossible requests first.  */
  if ((type == AF_INET && have_inet == 0)
      || (type == AF_INET6 && have_inet6 == 0 && ((flags & AI_V4MAPPED) == 0
						  || have_inet == 0))
      || (type != AF_INET && type != AF_INET6))
    {
      *h_errnop = NO_DATA;
      return NULL;
    }

  buffer_size = BUFLEN;
  buffer = (char *) malloc (buffer_size);

  result = NULL;
  if (buffer != NULL)
    {
#define HAVE_TYPE
#define resbuf (*((struct hostent *) buffer))
#include "../nss/digits_dots.c"
    }

  if (startp == NULL)
    {
      no_more = __nss_hosts_lookup (&nip, "getipnodebyname_r", (void **) &fct);
      if (no_more)
	startp = (service_user *) -1l;
      else
	{
	  startp = nip;
	  start_fct = fct;
	}
    }
  else
    {
      fct = start_fct;
      no_more = (nip = startp) == (service_user *) -1l;
    }

  /* First do a lookup with the original protocol type.  */
  while (no_more == 0 && buffer != NULL)
    {
      enum nss_status status;

      status = DL_CALL_FCT (fct, (name, type,
				  ((flags & AI_ALL)
				   ? flags : (flags & ~AI_V4MAPPED)),
				  (struct hostent *) buffer,
				  (char *) ((struct hostent *) buffer + 1),
				  buffer_size - sizeof (struct hostent),
				  &errno, h_errnop));

      if (status == NSS_STATUS_SUCCESS)
	{
	  result = (struct hostent *) buffer;
	  break;
	}

      if (status == NSS_STATUS_TRYAGAIN && *h_errnop == NETDB_INTERNAL
	  && errno == ERANGE)
	{
	  /* Resize the buffer, it's too small.  */
	  buffer_size += BUFLEN;
	  buffer = (char *) realloc (buffer, buffer_size);
	  continue;
	}

      no_more = __nss_next (&nip, "getipnodebyname_r",
			    (void **) &fct, status, 0);
    }

  /* If we are looking for an IPv6 address but haven't found any and
     do not have the AI_ALL but the AI_V4MAPPED flag set, now try
     looking up an IPv4 address and map it.  */
  if (buffer != NULL && result == NULL
      && type == AF_INET6 && (flags & AI_V4MAPPED) != 0
      && (no_more = (nip = startp) == (service_user *) -1l) == 0)
    {
      /* We have to use a new buffer if there is already a result.  */
      fct = start_fct;

      do
	{
	  enum nss_status status;

	  status = DL_CALL_FCT (fct, (name, type, flags,
				      (struct hostent *) buffer,
				      (char *) ((struct hostent *) buffer + 1),
				      buffer_size - sizeof (struct hostent),
				      &errno, h_errnop));

	  if (status == NSS_STATUS_SUCCESS)
	    {
	      result = (struct hostent *) buffer;
	      break;
	    }

	  if (status == NSS_STATUS_TRYAGAIN && *h_errnop == NETDB_INTERNAL
	      && errno == ERANGE)
	    {
	      /* Resize the buffer, it's too small.  */
	      buffer_size += BUFLEN;
	      buffer = (char *) realloc (buffer, buffer_size);
	      continue;
	    }

	  no_more = __nss_next (&nip, "getipnodebyname_r",
				(void **) &fct, status, 0);
	}
      while (no_more == 0 && buffer != NULL);
    }

 done:
  if (buffer == NULL)
    {
      /* We are out of memory.  */
      *h_errnop = TRY_AGAIN;
      assert (result == NULL);
    }
  else if (result == NULL && buffer != NULL)
    free (buffer);

  return result;
}
