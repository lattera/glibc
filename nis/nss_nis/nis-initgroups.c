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
    return instatus;

  if (inkey && inkeylen > 0 && inval && invallen > 0)
    {
      struct response_t *newp = malloc (sizeof (struct response_t)
					+ invallen + 1);
      if (newp == NULL)
	return YP_FALSE; /* We have no error code for out of memory */

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
internal_setgrent (intern_t *intern)
{
  char *domainname;
  struct ypall_callback ypcb;
  enum nss_status status;

  if (yp_get_default_domain (&domainname))
    return NSS_STATUS_UNAVAIL;

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
    internal_setgrent (intern);

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

enum nss_status
_nss_nis_initgroups_dyn (const char *user, gid_t group, long int *start,
			 long int *size, gid_t **groupsp, long int limit,
			 int *errnop)
{
  struct group grpbuf, *g;
  size_t buflen = sysconf (_SC_GETPW_R_SIZE_MAX);
  char *tmpbuf;
  enum nss_status status;
  intern_t intern = { NULL, NULL };
  gid_t *groups = *groupsp;

  status = internal_setgrent (&intern);
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
