/* Copyright (C) 1996,1997,1998,1999,2002,2004 Free Software Foundation, Inc.
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

#include <bits/libc-lock.h>
#include <errno.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include "netgroup.h"
#include "nsswitch.h"


/* Protect above variable against multiple uses at the same time.  */
__libc_lock_define_initialized (static, lock)

/* The whole information for the set/get/endnetgrent functions are
   kept in this structure.  */
static struct __netgrent dataset;

/* The lookup function for the first entry of this service.  */
extern int __nss_netgroup_lookup (service_user **nip, const char *name,
				  void **fctp) internal_function;


/* Set up NIP to run through the services.  If ALL is zero, use NIP's
   current location if it's not nil.  Return nonzero if there are no
   services (left).  */
static enum nss_status
setup (void **fctp, const char *func_name, int all, service_user **nipp)
{
  /* Remember the first service_entry, it's always the same.  */
  static service_user *startp;
  int no_more;

  if (startp == NULL)
    {
      /* Executing this more than once at the same time must yield the
	 same result every time.  So we need no locking.  */
      no_more = __nss_netgroup_lookup (nipp, func_name, fctp);
      startp = no_more ? (service_user *) -1 : *nipp;
    }
  else if (startp == (service_user *) -1)
    /* No services at all.  */
    return 1;
  else
    {
      if (all || *nipp == NULL)
	/* Reset to the beginning of the service list.  */
	*nipp = startp;
      /* Look up the first function.  */
      no_more = __nss_lookup (nipp, func_name, fctp);
    }
  return no_more;
}

/* Free used memory.  */
static void
free_memory (struct __netgrent *data)
{
  while (data->known_groups != NULL)
    {
      struct name_list *tmp = data->known_groups;
      data->known_groups = data->known_groups->next;
      free (tmp);
    }

  while (data->needed_groups != NULL)
    {
      struct name_list *tmp = data->needed_groups;
      data->needed_groups = data->needed_groups->next;
      free (tmp);
    }
}

static int
internal_function
__internal_setnetgrent_reuse (const char *group, struct __netgrent *datap,
			      int *errnop)
{
  union
  {
    enum nss_status (*f) (const char *, struct __netgrent *);
    void *ptr;
  } fct;
  enum nss_status status = NSS_STATUS_UNAVAIL;
  struct name_list *new_elem;

  /* Cycle through all the services and run their setnetgrent functions.  */
  int no_more = setup (&fct.ptr, "setnetgrent", 1, &datap->nip);
  while (! no_more)
    {
      /* Ignore status, we force check in `__nss_next'.  */
      status = (*fct.f) (group, datap);

      no_more = __nss_next (&datap->nip, "setnetgrent", &fct.ptr, status, 0);
    }

  /* Add the current group to the list of known groups.  */
  size_t group_len = strlen (group) + 1;
  new_elem = (struct name_list *) malloc (sizeof (struct name_list)
					  + group_len);
  if (new_elem == NULL)
    {
      *errnop = errno;
      status = NSS_STATUS_TRYAGAIN;
    }
  else
    {
      new_elem->next = datap->known_groups;
      memcpy (new_elem->name, group, group_len);
      datap->known_groups = new_elem;
    }

  return status == NSS_STATUS_SUCCESS;
}

int internal_setnetgrent (const char *group, struct __netgrent *datap);
libc_hidden_proto (internal_setnetgrent)

int
internal_setnetgrent (const char *group, struct __netgrent *datap)
{
  /* Free list of all netgroup names from last run.  */
  free_memory (datap);

  return __internal_setnetgrent_reuse (group, datap, &errno);
}
libc_hidden_def (internal_setnetgrent)
strong_alias (internal_setnetgrent, __internal_setnetgrent)

int
setnetgrent (const char *group)
{
  int result;

  __libc_lock_lock (lock);

  result = internal_setnetgrent (group, &dataset);

  __libc_lock_unlock (lock);

  return result;
}


void internal_endnetgrent (struct __netgrent *datap);
libc_hidden_proto (internal_endnetgrent)

void
internal_endnetgrent (struct __netgrent *datap)
{
  service_user *old_nip;
  union
  {
    enum nss_status (*f) (struct __netgrent *);
    void *ptr;
  } fct;

  /* Remember which was the last used service.  */
  old_nip = datap->nip;

  /* Cycle through all the services and run their endnetgrent functions.  */
  int no_more = setup (&fct.ptr, "endnetgrent", 1, &datap->nip);
  while (! no_more)
    {
      /* Ignore status, we force check in `__nss_next'.  */
      (void) (*fct.f) (datap);

      no_more = (datap->nip == old_nip
		 || __nss_next (&datap->nip, "endnetgrent", &fct.ptr, 0, 1));
    }

  /* Now free list of all netgroup names from last run.  */
  free_memory (datap);
}
libc_hidden_def (internal_endnetgrent)
strong_alias (internal_endnetgrent, __internal_endnetgrent)


void
endnetgrent (void)
{
  __libc_lock_lock (lock);

  internal_endnetgrent (&dataset);

  __libc_lock_unlock (lock);
}


int internal_getnetgrent_r (char **hostp, char **userp, char **domainp,
			    struct __netgrent *datap,
			    char *buffer, size_t buflen, int *errnop);
libc_hidden_proto (internal_getnetgrent_r)

int
internal_getnetgrent_r (char **hostp, char **userp, char **domainp,
			  struct __netgrent *datap,
			  char *buffer, size_t buflen, int *errnop)
{
  union
  {
    enum nss_status (*f) (struct __netgrent *, char *, size_t, int *);
    void *ptr;
  } fct;

  /* Initialize status to return if no more functions are found.  */
  enum nss_status status = NSS_STATUS_NOTFOUND;

  /* Run through available functions, starting with the same function last
     run.  We will repeat each function as long as it succeeds, and then go
     on to the next service action.  */
  int no_more = setup (&fct.ptr, "getnetgrent_r", 0, &datap->nip);
  while (! no_more)
    {
      status = (*fct.f) (datap, buffer, buflen, &errno);

      if (status == NSS_STATUS_RETURN)
	{
	  /* This was the last one for this group.  Look at next group
	     if available.  */
	  int found = 0;
	  while (datap->needed_groups != NULL && ! found)
	    {
	      struct name_list *tmp = datap->needed_groups;
	      datap->needed_groups = datap->needed_groups->next;
	      tmp->next = datap->known_groups;
	      datap->known_groups = tmp;

	      found = __internal_setnetgrent_reuse (datap->known_groups->name,
						    datap, errnop);
	    }

	  if (found)
	    continue;
	}
      else if (status == NSS_STATUS_SUCCESS && datap->type == group_val)
	{
	  /* The last entry was a name of another netgroup.  */
	  struct name_list *namep;

	  /* Ignore if we've seen the name before.  */
	  for (namep = datap->known_groups; namep != NULL;
	       namep = namep->next)
	    if (strcmp (datap->val.group, namep->name) == 0)
	      break;
	  if (namep != NULL)
	    /* Really ignore.  */
	    continue;

	  size_t group_len = strlen (datap->val.group) + 1;
	  namep = (struct name_list *) malloc (sizeof (struct name_list)
					       + group_len);
	  if (namep == NULL)
	    /* We are out of memory.  */
	    status = NSS_STATUS_RETURN;
	  else
	    {
	      namep->next = datap->needed_groups;
	      memcpy (namep->name, datap->val.group, group_len);
	      datap->needed_groups = namep;
	      /* And get the next entry.  */
	      continue;
	    }
	}

      no_more = __nss_next (&datap->nip, "getnetgrent_r", &fct.ptr, status, 0);
    }

  if (status == NSS_STATUS_SUCCESS)
    {
      *hostp = (char *) datap->val.triple.host;
      *userp = (char *) datap->val.triple.user;
      *domainp = (char *) datap->val.triple.domain;
    }

  return status == NSS_STATUS_SUCCESS ? 1 : 0;
}
libc_hidden_def (internal_getnetgrent_r)
strong_alias (internal_getnetgrent_r, __internal_getnetgrent_r)

/* The real entry point.  */
int
__getnetgrent_r (char **hostp, char **userp, char **domainp,
		 char *buffer, size_t buflen)
{
  enum nss_status status;

  __libc_lock_lock (lock);

  status = internal_getnetgrent_r (hostp, userp, domainp, &dataset,
				   buffer, buflen, &errno);

  __libc_lock_unlock (lock);

  return status;
}
weak_alias (__getnetgrent_r, getnetgrent_r)

/* Test whether given (host,user,domain) triple is in NETGROUP.  */
int
innetgr (const char *netgroup, const char *host, const char *user,
	 const char *domain)
{
  union
  {
    int (*f) (const char *, struct __netgrent *);
    void *ptr;
  } setfct;
  union
  {
    void (*f) (struct __netgrent *);
    void *ptr;
  } endfct;
  union
  {
    int (*f) (struct __netgrent *, char *, size_t, int *);
    void *ptr;
  } getfct;
  struct __netgrent entry;
  int result = 0;
  const char *current_group = netgroup;
  int real_entry = 0;

  memset (&entry, '\0', sizeof (entry));

  /* Walk through the services until we found an answer or we shall
     not work further.  We can do some optimization here.  Since all
     services must provide the `setnetgrent' function we can do all
     the work during one walk through the service list.  */
  while (1)
    {
      int no_more = setup (&setfct.ptr, "setnetgrent", 1, &entry.nip);
      while (! no_more)
	{
	  /* Open netgroup.  */
	  enum nss_status status = (*setfct.f) (current_group, &entry);

	  if (status == NSS_STATUS_SUCCESS
	      && __nss_lookup (&entry.nip, "getnetgrent_r", &getfct.ptr) == 0)
	    {
	      char buffer[1024];

	      while ((*getfct.f) (&entry, buffer, sizeof buffer, &errno)
		     == NSS_STATUS_SUCCESS)
		{
		  if (entry.type == group_val)
		    {
		      /* Make sure we haven't seen the name before.  */
		      struct name_list *namep;

		      for (namep = entry.known_groups; namep != NULL;
			   namep = namep->next)
			if (strcmp (entry.val.group, namep->name) == 0)
			  break;
		      if (namep == NULL
			  && strcmp (netgroup, entry.val.group) != 0)
			{
			  size_t group_len = strlen (entry.val.group) + 1;
			  namep =
			    (struct name_list *) malloc (sizeof (*namep)
							 + group_len);
			  if (namep == NULL)
			    {
			      /* Out of memory, simply return.  */
			      result = -1;
			      break;
			    }

			  namep->next = entry.needed_groups;
			  memcpy (namep->name, entry.val.group, group_len);
			  entry.needed_groups = namep;
			}
		    }
		  else
		    {
		      real_entry = 1;

		      if ((entry.val.triple.host == NULL || host == NULL
			   || __strcasecmp (entry.val.triple.host, host) == 0)
			  && (entry.val.triple.user == NULL || user == NULL
			      || strcmp (entry.val.triple.user, user) == 0)
			  && (entry.val.triple.domain == NULL || domain == NULL
			      || __strcasecmp (entry.val.triple.domain,
					       domain) == 0))
			{
			  result = 1;
			  break;
			}
		    }
		}

	      if (result != 0)
		break;

	      /* If we found one service which does know the given
		 netgroup we don't try further.  */
	      status = NSS_STATUS_RETURN;
	    }

	  /* Free all resources of the service.  */
	  if (__nss_lookup (&entry.nip, "endnetgrent", &endfct.ptr) == 0)
	    (*endfct.f) (&entry);

	  /* Look for the next service.  */
	  no_more = __nss_next (&entry.nip, "setnetgrent",
				&setfct.ptr, status, 0);
	}

      if (result == 0 && entry.needed_groups != NULL)
	{
	  struct name_list *tmp = entry.needed_groups;
	  entry.needed_groups = tmp->next;
	  tmp->next = entry.known_groups;
	  entry.known_groups = tmp;
	  current_group = entry.known_groups->name;
	  continue;
	}

      /* No way out.  */
      break;
    }

  /* Free the memory.  */
  free_memory (&entry);

  return result;
}
libc_hidden_def (innetgr)
