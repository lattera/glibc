/* Copyright (C) 1998-2000, 2002, 2003, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Thorsten Kukuk <kukuk@suse.de>, 1998.

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
#include <ctype.h>
#include <errno.h>
#include <grp.h>
#include <nss.h>
#include <pwd.h>
#include <string.h>
#include <unistd.h>
#include <rpcsvc/yp.h>
#include <rpcsvc/ypclnt.h>
#include <sys/param.h>

#include "nss-nis.h"

/* Get the declaration of the parser function.  */
#define ENTNAME grent
#define STRUCTURE group
#define EXTERN_PARSER
#include <nss/nss_files/files-parse.c>

struct response_t
{
  struct response_t *next;
  char val[0];
};

struct intern_t
{
  struct response_t *start;
  struct response_t *next;
};
typedef struct intern_t intern_t;

static int
saveit (int instatus, char *inkey, int inkeylen, char *inval,
        int invallen, char *indata)
{
  intern_t *intern = (intern_t *) indata;

  if (instatus != YP_TRUE)
    return 1;

  if (inkey && inkeylen > 0 && inval && invallen > 0)
    {
      struct response_t *newp = malloc (sizeof (struct response_t)
					+ invallen + 1);
      if (newp == NULL)
	return 1; /* We have no error code for out of memory */

      if (intern->start == NULL)
	intern->start = newp;
      else
	intern->next->next = newp;
      intern->next = newp;

      newp->next = NULL;
      *((char *) mempcpy (newp->val, inval, invallen)) = '\0';
    }

  return 0;
}

static enum nss_status
internal_setgrent (char *domainname, intern_t *intern)
{
  struct ypall_callback ypcb;
  enum nss_status status;

  intern->start = NULL;

  ypcb.foreach = saveit;
  ypcb.data = (char *) intern;
  status = yperr2nss (yp_all (domainname, "group.byname", &ypcb));
  intern->next = intern->start;

  return status;
}

static enum nss_status
internal_getgrent_r (struct group *grp, char *buffer, size_t buflen,
		     int *errnop, intern_t *intern)
{
  struct parser_data *data = (void *) buffer;
  int parse_res;
  char *p;

  if (intern->start == NULL)
    return NSS_STATUS_NOTFOUND;

  /* Get the next entry until we found a correct one. */
  do
    {
      if (intern->next == NULL)
	return NSS_STATUS_NOTFOUND;

      p = strncpy (buffer, intern->next->val, buflen);
      while (isspace (*p))
        ++p;

      parse_res = _nss_files_parse_grent (p, grp, data, buflen, errnop);
      if (parse_res == -1)
        return NSS_STATUS_TRYAGAIN;
      intern->next = intern->next->next;
    }
  while (!parse_res);

  return NSS_STATUS_SUCCESS;
}


static int
get_uid (const char *user, uid_t *uidp)
{
  size_t buflen = sysconf (_SC_GETPW_R_SIZE_MAX);
  char *buf = (char *) alloca (buflen);

  while (1)
    {
      struct passwd result;
      struct passwd *resp;

      int r = getpwnam_r (user, &result, buf, buflen, &resp);
      if (r == 0 && resp != NULL)
	{
	  *uidp = resp->pw_uid;
	  return 0;
	}

      if (r != ERANGE)
	break;

      extend_alloca (buf, buflen, 2 * buflen);
    }

  return 1;
}


static enum nss_status
initgroups_netid (uid_t uid, gid_t group, long int *start, long int *size,
		  gid_t **groupsp, long int limit, int *errnop,
		  const char *domainname)
{
  /* Prepare the key.  The form is "unix.UID@DOMAIN" with the UID and
     DOMAIN field filled in appropriately.  */
  char key[sizeof ("unix.@") + sizeof (uid_t) * 3 + strlen (domainname)];
  ssize_t keylen = snprintf (key, sizeof (key), "unix.%lu@%s",
			     (unsigned long int) uid, domainname);

  enum nss_status retval;
  char *result;
  int reslen;
  retval = yperr2nss (yp_match (domainname, "netid.byname", key, keylen,
				&result, &reslen));
  if (retval != NSS_STATUS_SUCCESS)
    return retval;

  /* Parse the result: following the colon is a comma separated list of
     group IDs.  */
  char *cp = strchr (result, ':');
  if (cp == NULL)
    {
    errout:
      free (result);
      return NSS_STATUS_NOTFOUND;
    }
  /* Skip the colon.  */
  ++cp;

  gid_t *groups = *groupsp;
  while (*cp != '\0')
    {
      char *endp;
      unsigned long int gid = strtoul (cp, &endp, 0);
      if (cp == endp)
	goto errout;
      if (*endp == ',')
	++endp;
      else if (*endp != '\0')
	goto errout;
      cp = endp;

      if (gid == group)
	/* We do not need this group again.  */
	continue;

      /* Insert this group.  */
      if (*start == *size)
	{
	  /* Need a bigger buffer.  */
	  gid_t *newgroups;
	  long int newsize;

	  if (limit > 0 && *size == limit)
	    /* We reached the maximum.  */
	    break;

	  if (limit <= 0)
	    newsize = 2 * *size;
	  else
	    newsize = MIN (limit, 2 * *size);

	  newgroups = realloc (groups, newsize * sizeof (*groups));
	  if (newgroups == NULL)
	    goto errout;
	  *groupsp = groups = newgroups;
	  *size = newsize;
	}

      groups[*start] = gid;
      *start += 1;
    }

  free (result);

  return NSS_STATUS_SUCCESS;
}


enum nss_status
_nss_nis_initgroups_dyn (const char *user, gid_t group, long int *start,
			 long int *size, gid_t **groupsp, long int limit,
			 int *errnop)
{
  /* We always need the domain name.  */
  char *domainname;
  if (yp_get_default_domain (&domainname))
    return NSS_STATUS_UNAVAIL;

  /* Check whether we are supposed to use the netid.byname map.  */
  if (_nis_default_nss () & NSS_FLAG_NETID_AUTHORITATIVE)
    {
      /* We need the user ID.  */
      uid_t uid;

      if (get_uid (user, &uid) == 0
	  && initgroups_netid (uid, group, start, size, groupsp, limit,
			       errnop, domainname) == NSS_STATUS_SUCCESS)
	return NSS_STATUS_SUCCESS;
    }

  struct group grpbuf, *g;
  size_t buflen = sysconf (_SC_GETPW_R_SIZE_MAX);
  char *tmpbuf;
  enum nss_status status;
  intern_t intern = { NULL, NULL };
  gid_t *groups = *groupsp;

  status = internal_setgrent (domainname, &intern);
  if (status != NSS_STATUS_SUCCESS)
    return status;

  tmpbuf = __alloca (buflen);

  do
    {
      while ((status =
	      internal_getgrent_r (&grpbuf, tmpbuf, buflen, errnop,
				   &intern)) == NSS_STATUS_TRYAGAIN
             && *errnop == ERANGE)
	tmpbuf = extend_alloca (tmpbuf, buflen, 2 * buflen);

      if (status != NSS_STATUS_SUCCESS)
	goto done;


      g = &grpbuf;
      if (g->gr_gid != group)
        {
          char **m;

          for (m = g->gr_mem; *m != NULL; ++m)
            if (strcmp (*m, user) == 0)
              {
                /* Matches user.  Insert this group.  */
                if (*start == *size)
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

                groups[*start] = g->gr_gid;
		*start += 1;

                break;
              }
        }
    }
  while (status == NSS_STATUS_SUCCESS);

done:
  while (intern.start != NULL)
    {
      intern.next = intern.start;
      intern.start = intern.start->next;
      free (intern.next);
    }

  return NSS_STATUS_SUCCESS;
}
