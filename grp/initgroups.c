/* Copyright (C) 1989,91,93,1996-1999,2000,01 Free Software Foundation, Inc.
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

#include <alloca.h>
#include <errno.h>
#include <grp.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <nsswitch.h>

/* Type of the lookup function.  */
typedef enum nss_status (*initgroups_dyn_function) (const char *, gid_t,
						    long int *, long int *,
						    gid_t **, long int, int *);
/* Prototype for the setgrent functions we use here.  */
typedef enum nss_status (*set_function) (void);

/* Prototype for the endgrent functions we use here.  */
typedef enum nss_status (*end_function) (void);

/* Prototype for the setgrent functions we use here.  */
typedef enum nss_status (*get_function) (struct group *, char *,
					 size_t, int *);

/* The lookup function for the first entry of this service.  */
extern int __nss_group_lookup (service_user **nip, const char *name,
				   void **fctp);
extern void *__nss_lookup_function (service_user *ni, const char *fct_name);

extern service_user *__nss_group_database;

static enum nss_status
compat_call (service_user *nip, const char *user, gid_t group, long int *start,
	     long int *size, gid_t **groupsp, long int limit, int *errnop)
{
  struct group grpbuf;
  size_t buflen = __sysconf (_SC_GETGR_R_SIZE_MAX);
  char *tmpbuf;
  enum nss_status status;
  set_function setgrent_fct;
  get_function getgrent_fct;
  end_function endgrent_fct;
  gid_t *groups = *groupsp;

  getgrent_fct = __nss_lookup_function (nip, "getgrent_r");
  if (getgrent_fct == NULL)
    return NSS_STATUS_UNAVAIL;

  setgrent_fct = __nss_lookup_function (nip, "setgrent");
  if (setgrent_fct)
    {
      status = DL_CALL_FCT (setgrent_fct, ());
      if (status != NSS_STATUS_SUCCESS)
	return status;
    }

  endgrent_fct = __nss_lookup_function (nip, "endgrent");

  tmpbuf = __alloca (buflen);

  do
    {
      while ((status = DL_CALL_FCT (getgrent_fct,
				     (&grpbuf, tmpbuf, buflen, errnop)),
	      status == NSS_STATUS_TRYAGAIN)
	     && *errnop == ERANGE)
        {
          buflen *= 2;
          tmpbuf = __alloca (buflen);
        }

      if (status != NSS_STATUS_SUCCESS)
        goto done;

      if (grpbuf.gr_gid != group)
        {
          char **m;

          for (m = grpbuf.gr_mem; *m != NULL; ++m)
            if (strcmp (*m, user) == 0)
              {
                /* Matches user.  Insert this group.  */
                if (__builtin_expect (*start == *size, 0))
                  {
                    /* Need a bigger buffer.  */
		    gid_t *newgroups;
		    long int newsize;

		    if (limit > 0 && *size == limit)
		      /* We reached the maximum.  */
		      goto done;

		    if (limit <= 0)
		      newsize = 2 * *size;
		    else
		      newsize = MIN (limit, 2 * *size);

                    newgroups = realloc (groups, newsize * sizeof (*groups));
                    if (newgroups == NULL)
                      goto done;
		    *groupsp = groups = newgroups;
                    *size = newsize;
                  }

                groups[*start] = grpbuf.gr_gid;
                *start += 1;

                break;
              }
        }
    }
  while (status == NSS_STATUS_SUCCESS);

 done:
  if (endgrent_fct)
    DL_CALL_FCT (endgrent_fct, ());

  return NSS_STATUS_SUCCESS;
}

/* Initialize the group set for the current user
   by reading the group database and using all groups
   of which USER is a member.  Also include GROUP.  */
int
initgroups (user, group)
     const char *user;
     gid_t group;
{
#if defined NGROUPS_MAX && NGROUPS_MAX == 0

  /* No extra groups allowed.  */
  return 0;

#else

  service_user *nip = NULL;
  initgroups_dyn_function fct;
  enum nss_status status = NSS_STATUS_UNAVAIL;
  int no_more;
  /* Start is one, because we have the first group as parameter.  */
  long int start = 1;
  long int size;
  gid_t *groups;
  int result;

  /* We always use sysconf even if NGROUPS_MAX is defined.  That way, the
     limit can be raised in the kernel configuration without having to
     recompile libc.  */
  long int limit = __sysconf (_SC_NGROUPS_MAX);

  if (limit > 0)
    size = limit;
  else
    {
      /* No fixed limit on groups.  Pick a starting buffer size.  */
      size = 16;
    }

  groups = (gid_t *) malloc (size * sizeof (gid_t));
  if (__builtin_expect (groups == NULL, 0))
    /* No more memory.  */
    return -1;

  groups[0] = group;

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
      fct = __nss_lookup_function (nip, "initgroups_dyn");

      if (fct == NULL)
	{
	  status = compat_call (nip, user, group, &start, &size, &groups,
				limit, &errno);

	  if (nss_next_action (nip, NSS_STATUS_UNAVAIL) != NSS_ACTION_CONTINUE)
	    break;
	}
      else
	status = DL_CALL_FCT (fct, (user, group, &start, &size, &groups,
				    limit, &errno));

      /* This is really only for debugging.  */
      if (NSS_STATUS_TRYAGAIN > status || status > NSS_STATUS_RETURN)
	__libc_fatal ("illegal status in " __FUNCTION__);

      if (status != NSS_STATUS_SUCCESS
	  && nss_next_action (nip, status) == NSS_ACTION_RETURN)
	 break;

      if (nip->next == NULL)
	no_more = -1;
      else
	nip = nip->next;
    }

  /* Try to set the maximum number of groups the kernel can handle.  */
  do
    result = setgroups (start, groups);
  while (result == -1 && errno == EINVAL && --start > 0);

  free (groups);

  return result;
#endif
}
