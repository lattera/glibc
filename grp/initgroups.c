/* Copyright (C) 1989,91,93,1996-2003, 2004, 2005  Free Software Foundation, Inc.
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

#include <alloca.h>
#include <errno.h>
#include <grp.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/types.h>
#include <nsswitch.h>

#include "../nscd/nscd-client.h"
#include "../nscd/nscd_proto.h"


/* Type of the lookup function.  */
typedef enum nss_status (*initgroups_dyn_function) (const char *, gid_t,
						    long int *, long int *,
						    gid_t **, long int, int *);

/* The lookup function for the first entry of this service.  */
extern int __nss_group_lookup (service_user **nip, const char *name,
				   void **fctp);
extern void *__nss_lookup_function (service_user *ni, const char *fct_name);

extern service_user *__nss_group_database attribute_hidden;


#include "compat-initgroups.c"


static int
internal_getgrouplist (const char *user, gid_t group, long int *size,
		       gid_t **groupsp, long int limit)
{
#ifdef USE_NSCD
  if (__nss_not_use_nscd_group > 0
      && ++__nss_not_use_nscd_group > NSS_NSCD_RETRY)
    __nss_not_use_nscd_group = 0;
  if (!__nss_not_use_nscd_group)
    {
      int n = __nscd_getgrouplist (user, group, size, groupsp, limit);
      if (n >= 0)
	return n;

      /* nscd is not usable.  */
      __nss_not_use_nscd_group = 1;
    }
#endif

  service_user *nip = NULL;
  initgroups_dyn_function fct;
  enum nss_status status = NSS_STATUS_UNAVAIL;
  int no_more;
  /* Start is one, because we have the first group as parameter.  */
  long int start = 1;

  /* Never store more than the starting *SIZE number of elements.  */
  if (*size > 0)
    (*groupsp)[0] = group;

  if (__nss_group_database != NULL)
    {
      no_more = 0;
      nip = __nss_group_database;
    }
  else
    no_more = __nss_database_lookup ("group", NULL,
				     "compat [NOTFOUND=return] files", &nip);

  while (! no_more)
    {
      long int prev_start = start;

      fct = __nss_lookup_function (nip, "initgroups_dyn");

      if (fct == NULL)
	{
	  status = compat_call (nip, user, group, &start, size, groupsp,
				limit, &errno);

	  if (nss_next_action (nip, NSS_STATUS_UNAVAIL) != NSS_ACTION_CONTINUE)
	    break;
	}
      else
	status = DL_CALL_FCT (fct, (user, group, &start, size, groupsp,
				    limit, &errno));

      /* Remove duplicates.  */
      long int cnt = prev_start;
      while (cnt < start)
	{
	  long int inner;
	  for (inner = 0; inner < prev_start; ++inner)
	    if ((*groupsp)[inner] == (*groupsp)[cnt])
	      break;

	  if (inner < prev_start)
	    (*groupsp)[cnt] = (*groupsp)[--start];
	  else
	    ++cnt;
	}

      /* This is really only for debugging.  */
      if (NSS_STATUS_TRYAGAIN > status || status > NSS_STATUS_RETURN)
	__libc_fatal ("illegal status in internal_getgrouplist");

      if (status != NSS_STATUS_SUCCESS
	  && nss_next_action (nip, status) == NSS_ACTION_RETURN)
	 break;

      if (nip->next == NULL)
	no_more = -1;
      else
	nip = nip->next;
    }

  return start;
}

/* Store at most *NGROUPS members of the group set for USER into
   *GROUPS.  Also include GROUP.  The actual number of groups found is
   returned in *NGROUPS.  Return -1 if the if *NGROUPS is too small.  */
int
getgrouplist (const char *user, gid_t group, gid_t *groups, int *ngroups)
{
  gid_t *newgroups;
  long int size = MAX (1, *ngroups);
  int result;

  newgroups = (gid_t *) malloc ((size + 1) * sizeof (gid_t));
  if (__builtin_expect (newgroups == NULL, 0))
    /* No more memory.  */
    // XXX This is wrong.  The user provided memory, we have to use
    // XXX it.  The internal functions must be called with the user
    // XXX provided buffer and not try to increase the size if it is
    // XXX too small.  For initgroups a flag could say: increase size.
    return -1;

  result = internal_getgrouplist (user, group, &size, &newgroups, -1);

  memcpy (groups, newgroups, MIN (*ngroups, result) * sizeof (gid_t));

  if (result > *ngroups)
    {
      *ngroups = result;
      result = -1;
    }
  else
    *ngroups = result;

  free (newgroups);
  return result;
}

static_link_warning (getgrouplist)

/* Initialize the group set for the current user
   by reading the group database and using all groups
   of which USER is a member.  Also include GROUP.  */
int
initgroups (const char *user, gid_t group)
{
#if defined NGROUPS_MAX && NGROUPS_MAX == 0

  /* No extra groups allowed.  */
  return 0;

#else

  long int size;
  gid_t *groups;
  int ngroups;
  int result;

 /* We always use sysconf even if NGROUPS_MAX is defined.  That way, the
     limit can be raised in the kernel configuration without having to
     recompile libc.  */
  long int limit = __sysconf (_SC_NGROUPS_MAX);

  if (limit > 0)
    /* We limit the size of the intially allocated array.  */
    size = MIN (limit, 64);
  else
    /* No fixed limit on groups.  Pick a starting buffer size.  */
    size = 16;

  groups = (gid_t *) malloc (size * sizeof (gid_t));
  if (__builtin_expect (groups == NULL, 0))
    /* No more memory.  */
    return -1;

  ngroups = internal_getgrouplist (user, group, &size, &groups, limit);

  /* Try to set the maximum number of groups the kernel can handle.  */
  do
    result = setgroups (ngroups, groups);
  while (result == -1 && errno == EINVAL && --ngroups > 0);

  free (groups);

  return result;
#endif
}

static_link_warning (initgroups)
