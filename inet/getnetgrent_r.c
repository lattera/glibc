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

/* A netgroup can consist of names of other netgroups.  We have to
   track which netgroups were read and which still have to be read.  */
struct name_list
{
  const char *name;
  struct name_list *next;
};
struct name_list *known_groups;
struct name_list *needed_groups;


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

/* Free used memory.  */
static void
free_memory (void)
{
  while (known_groups != NULL)
    {
      struct name_list *tmp = known_groups;
      known_groups = known_groups->next;
      free (tmp->name);
      free (tmp);
    }

  while (needed_groups != NULL)
    {
      struct name_list *tmp = needed_groups;
      needed_groups = needed_groups->next;
      free (tmp->name);
      free (tmp);
    }
}

static int
internal_setnetgrent (const char *group)
{
  enum nss_status (*fct) (const char *);
  enum nss_status status = NSS_STATUS_UNAVAIL;
  struct name_list *new_elem;
  int no_more;

  /* Cycle through all the services and run their setnetgrent functions.  */
  no_more = setup ((void **) &fct, "setnetgrent", 1);
  while (! no_more)
    {
      /* Ignore status, we force check in `__nss_next'.  */
      status = (*fct) (group);

      no_more = __nss_next (&nip, "setnetgrent", (void **) &fct, status, 0);
    }

  /* Add the current group to the list of known groups.  */
  new_elem = (struct name_list *) malloc (sizeof (struct name_list));
  if (new_elem == NULL || (new_elem->name = __strdup (group)) == NULL)
    {
      if (new_elem != NULL)
	free (new_elem);
      status == NSS_STATUS_UNAVAIL;
    }
  else
    {
      new_elem->next = known_groups;
      known_groups = new_elem;
    }

  return status == NSS_STATUS_SUCCESS;
}

int
setnetgrent (const char *group)
{
  int result;

  __libc_lock_lock (lock);

  /* Free list of all netgroup names from last run.  */
  free_memory ();

  result = internal_setnetgrent (group);

  __libc_lock_unlock (lock);

  return result;
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
      /* Ignore status, we force check in `__nss_next'.  */
      (void) (*fct) ();

      no_more = (nip == old_nip
		 || __nss_next (&nip, "endnetgrent", (void **) &fct, 0, 1));
    }

  /* Now free list of all netgroup names from last run.  */
  free_memory ();

  __libc_lock_unlock (lock);
}


int
__getnetgrent_r (char **hostp, char **userp, char **domainp,
		 char *buffer, size_t buflen)
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

      if (status == NSS_STATUS_RETURN)
	{
	  /* This was the last one for this group.  Look at next group
	     if available.  */
	  int found = 0;
	  while (needed_groups != NULL && ! found)
	    {
	      struct name_list *tmp = needed_groups;
	      needed_groups = needed_groups->next;
	      tmp->next = known_groups;
	      known_groups = tmp;

	      found = internal_setnetgrent (known_groups->name);
	    }

	  if (found)
	    continue;
	}
      else if (status == NSS_STATUS_SUCCESS && result.type == group_val)
	{
	  /* The last entry was a name of another netgroup.  */
	  struct name_list *namep;

	  /* Ignore if we've seen the name before.  */
	  for (namep = known_groups; namep != NULL; namep = namep->next)
	    if (strcmp (result.val.group, namep->name) == 0)
	      break;
	  if (namep != NULL)
	    /* Really ignore.  */
	    continue;

	  namep = (struct name_list *) malloc (sizeof (struct name_list));
	  if (namep == NULL
	      || (namep->name = __strdup (result.val.group)) == NULL)
	    {
	      /* We are out of memory.  */
	      if (namep != NULL)
		free (namep);
	      status = NSS_STATUS_RETURN;
	    }
	  else
	    {
	      namep->next = needed_groups;
	      needed_groups = namep;
	      /* And get the next entry.  */
	      continue;
	    }
	}

      no_more = __nss_next (&nip, "getnetgrent_r", (void **) &fct, status, 0);
    }

  if (status == NSS_STATUS_SUCCESS)
    {
      *hostp = result.val.triple.host;
      *userp = result.val.triple.user;
      *domainp = result.val.triple.domain;
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
  struct name_list *known = NULL;
  struct name_list *needed = NULL;
  const char *current_group = netgroup;
  int real_entry = 0;

  __libc_lock_lock (lock);

  /* Walk through the services until we found an answer or we shall
     not work further.  We can do some optimization here.  Since all
     services must provide the `setnetgrent' function we can do all
     the work during one walk through the service list.  */
  while (1)
    {
      no_more = setup ((void **) &setfct, "setnetgrent", 1);
      while (! no_more)
	{
	  enum nss_status status;

	  /* Open netgroup.  */
	  status = (*setfct) (current_group);
	  if (status == NSS_STATUS_SUCCESS
	      && __nss_lookup (&nip, "getnetgrent_r", (void **) &getfct) == 0)
	    {
	      char buffer[1024];
	      struct __netgrent entry;

	      while ((*getfct) (&entry, buffer, sizeof buffer)
		     == NSS_STATUS_SUCCESS)
		{
		  if (entry.type == group_val)
		    {
		      /* Make sure we haven't seen the name before.  */
		      struct name_list *namep;

		      for (namep = known; namep != NULL; namep = namep->next)
			if (strcmp (entry.val.group, namep->name) == 0)
			  break;
		      if (namep == NULL
			  && strcmp (netgroup, entry.val.group) != 0)
			{
			  namep =
			    (struct name_list *) malloc (sizeof (*namep));
			  if (namep == NULL
			      || ((namep->name = __strdup (entry.val.group))
				  == NULL))
			    {
			      /* Out of memory, simply return.  */
			      if (namep != NULL)
				free (namep);
			      result = -1;
			      break;
			    }

			  namep->next = needed;
			  needed = namep;
			}
		    }
		  else
		    {
		      real_entry = 1;

		      if ((entry.val.triple.host == NULL || host == NULL
			   || strcmp (entry.val.triple.host, host) == 0)
			  && (entry.val.triple.user == NULL || user == NULL
			      || strcmp (entry.val.triple.user, user) == 0)
			  && (entry.val.triple.domain == NULL || domain == NULL
			      || strcmp (entry.val.triple.domain, domain) == 0))
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
	  if (__nss_lookup (&nip, "endnetgrent", (void **) &endfct) == 0)
	    (*endfct) ();

	  /* Look for the next service.  */
	  no_more = __nss_next (&nip, "setnetgrent",
				(void **) &setfct, status, 0);
	}

      if (result == 0 && needed != NULL)
	{
	  struct name_list *tmp = needed;
	  needed = tmp->next;
	  tmp->next = known;
	  known = tmp;
	  current_group = known->name;
	  continue;
	}

      /* No way out.  */
      break;
    }

  __libc_lock_unlock (lock);

  /* Free the memory.  */
  while (known != NULL)
    {
      struct name_list *tmp = known;
      known = known->next;
      free (tmp->name);
      free (tmp);
    }
  while (needed != NULL)
    {
      struct name_list *tmp = needed;
      needed = needed->next;
      free (tmp->name);
      free (tmp);
    }

  return result == 1;
}
