/* Copyright (C) 1996 Free Software Foundation, Inc.
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
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#include <libc-lock.h>
#include <netdb.h>
#include "netgroup.h"
#include "nsswitch.h"


/* Protect above variable against multiple uses at the same time.  */
__libc_lock_define_initialized (static, lock)

/* This handle for the NSS data base is shared between all
   set/get/endXXXent functions.  */
static service_user *nip;
/* Remember the first service_entry, it's always the same.  */
static service_user *startp;


/* The lookup function for the first entry of this service.  */
extern int __nss_netgroup_lookup (service_user **nip, const char *name,
				  void **fctp);

/* Set up NIP to run through the services.  If ALL is zero, use NIP's
   current location if it's not nil.  Return nonzero if there are no
   services (left).  */
static enum nss_status
setup (void **fctp, const char *func_name, int all)
{
  int no_more;
  if (startp == NULL)
    {
      no_more = __nss_netgroup_lookup (&nip, func_name, fctp);
      startp = no_more ? (service_user *) -1 : nip;
    }
  else if (startp == (service_user *) -1)
    /* No services at all.  */
    return 1;
  else
    {
      if (all || !nip)
	/* Reset to the beginning of the service list.  */
	nip = startp;
      /* Look up the first function.  */
      no_more = __nss_lookup (&nip, func_name, fctp);
    }
  return no_more;
}

int
setnetgrent (const char *group)
{
  enum nss_status (*fct) (const char *);
  enum nss_status status = NSS_STATUS_UNAVAIL;
  int no_more;

  __libc_lock_lock (lock);

  /* Cycle through all the services and run their setnetgrent functions.  */
  no_more = setup ((void **) &fct, "setnetgrent", 1);
  while (! no_more)
    {
      /* Ignore status, we force check in __NSS_NEXT.  */
      status = (*fct) (group);

      no_more = __nss_next (&nip, "setnetgrent", (void **) &fct, status, 0);
    }

  __libc_lock_unlock (lock);

  return status == NSS_STATUS_SUCCESS;
}


void
endnetgrent (void)
{
  service_user *old_nip;
  enum nss_status (*fct) (void);
  int no_more;

  __libc_lock_lock (lock);

  /* Remember which was the last used service.  */
  old_nip = nip;

  /* Cycle through all the services and run their setnetgrent functions.  */
  no_more = setup ((void **) &fct, "endnetgrent", 1);
  while (! no_more)
    {
      /* Ignore status, we force check in __NSS_NEXT.  */
      (void) (*fct) ();

      no_more = (nip == old_nip
		 || __nss_next (&nip, "endnetgrent", (void **) &fct, 0, 1));
    }

  __libc_lock_unlock (lock);
}


int
__getnetgrent_r (char **hostp, char **userp, char **domainp,
		 char *buffer, int buflen)
{
  enum nss_status (*fct) (struct __netgrent *, char *, int);
  struct __netgrent result;
  int no_more;

  /* Initialize status to return if no more functions are found.  */
  enum nss_status status = NSS_STATUS_NOTFOUND;

  __libc_lock_lock (lock);

  /* Run through available functions, starting with the same function last
     run.  We will repeat each function as long as it succeeds, and then go
     on to the next service action.  */
  no_more = setup ((void **) &fct, "getnetgrent_r", 0);
  while (! no_more)
    {
      status = (*fct) (&result, buffer, buflen);

      no_more = __nss_next (&nip, "getnetgrent_r", (void **) &fct, status, 0);
    }

  if (status == NSS_STATUS_SUCCESS)
    {
      *hostp = result.host;
      *userp = result.user;
      *domainp = result.domain;
    }

  __libc_lock_unlock (lock);

  return status == NSS_STATUS_SUCCESS ? 1 : 0;
}
weak_alias (__getnetgrent_r, getnetgrent_r)

/* Test whether given (host,user,domain) triple is in NETGROUP.  */
int
innetgr (const char *netgroup, const char *host, const char *user,
	 const char *domain)
{
  int (*setfct) (const char *);
  void (*endfct) (void);
  int (*getfct) (struct __netgrent *, char *, int);
  int result = 0;
  int no_more;

  __libc_lock_lock (lock);

  /* Walk through the services until we found an answer or we shall
     not work further.  We can do some optimization here.  Since all
     services must provide the `setnetgrent' function we can do all
     the work during one walk through the service list.  */
  no_more = setup ((void **) &setfct, "setnetgrent", 1);
  while (! no_more)
    {
      enum nss_status status;

      /* Open netgroup.  */
      status = (*setfct) (netgroup);
      if (status == NSS_STATUS_SUCCESS
	  && __nss_lookup (&nip, "getnetgrent_r", (void **) &getfct) == 0)
	{
	  char buffer[1024];
	  struct __netgrent entry;

	  while ((*getfct) (&entry, buffer, sizeof buffer)
		 == NSS_STATUS_SUCCESS)
	    {
	      if ((entry.host == NULL || host == NULL
		   || strcmp (entry.host, host) == 0)
		  && (entry.user == NULL || user == NULL
		      || strcmp (entry.user, user) == 0)
		  && (entry.domain == NULL || domain == NULL
		      || strcmp (entry.domain, domain) == 0))
		{
		  result = 1;
		  break;
		}
	    }

	  if (result != 0)
	    break;

	  /* If we found one service which does know the given
	     netgroup we don't try further.  */
	  status = NSS_STATUS_RETURN;
	}

      /* Free all resources of the service.  */
      if (__nss_lookup (&nip, "endnetgrent", (void **) &endfct) == 0)
	(*endfct) ();

      /* Look for the next service.  */
      no_more = __nss_next (&nip, "setnetgrent", (void **) &setfct, status, 0);
    }

  __libc_lock_unlock (lock);

  return result;
}
