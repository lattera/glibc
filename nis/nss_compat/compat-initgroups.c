/* Copyright (C) 1998-2003, 2004 Free Software Foundation, Inc.
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
#include <fcntl.h>
#include <grp.h>
#include <nss.h>
#include <stdio_ext.h>
#include <string.h>
#include <unistd.h>
#include <rpc/types.h>
#include <sys/param.h>
#include <nsswitch.h>
#include <bits/libc-lock.h>

static service_user *ni;
/* Type of the lookup function.  */
static enum nss_status (*nss_initgroups_dyn) (const char *, gid_t,
					      long int *, long int *,
					      gid_t **, long int, int *);
static enum nss_status (*nss_setgrent) (int stayopen);
static enum nss_status (*nss_getgrnam_r) (const char *name,
					  struct group * grp, char *buffer,
					  size_t buflen, int *errnop);
static enum nss_status (*nss_getgrgid_r) (gid_t gid, struct group * grp,
					  char *buffer, size_t buflen,
					  int *errnop);
static enum nss_status (*nss_getgrent_r) (struct group * grp, char *buffer,
					  size_t buflen, int *errnop);
static enum nss_status (*nss_endgrent) (void);

/* Protect global state against multiple changers.  */
__libc_lock_define_initialized (static, lock)


/* Get the declaration of the parser function.  */
#define ENTNAME grent
#define STRUCTURE group
#define EXTERN_PARSER
#include <nss/nss_files/files-parse.c>

/* Structure for remembering -group members ... */
#define BLACKLIST_INITIAL_SIZE 512
#define BLACKLIST_INCREMENT 256
struct blacklist_t
{
  char *data;
  int current;
  int size;
};

struct ent_t
{
  bool_t files;
  FILE *stream;
  struct blacklist_t blacklist;
};
typedef struct ent_t ent_t;


/* Prototypes for local functions.  */
static void blacklist_store_name (const char *, ent_t *);
static int in_blacklist (const char *, int, ent_t *);

/* Initialize the NSS interface/functions. The calling function must
   hold the lock.  */
static void
init_nss_interface (void)
{
  __libc_lock_lock (lock);

  /* Retest.  */
  if (ni == NULL
      && __nss_database_lookup ("group_compat", NULL, "nis", &ni) >= 0)
    {
      nss_initgroups_dyn = __nss_lookup_function (ni, "initgroups_dyn");
      nss_setgrent = __nss_lookup_function (ni, "setgrent");
      nss_getgrnam_r = __nss_lookup_function (ni, "getgrnam_r");
      nss_getgrgid_r = __nss_lookup_function (ni, "getgrgid_r");
      nss_getgrent_r = __nss_lookup_function (ni, "getgrent_r");
      nss_endgrent = __nss_lookup_function (ni, "endgrent");
    }

  __libc_lock_unlock (lock);
}

static enum nss_status
internal_setgrent (ent_t *ent)
{
  enum nss_status status = NSS_STATUS_SUCCESS;

  ent->files = TRUE;

  if (ni == NULL)
    init_nss_interface ();

  if (ent->blacklist.data != NULL)
    {
      ent->blacklist.current = 1;
      ent->blacklist.data[0] = '|';
      ent->blacklist.data[1] = '\0';
    }
  else
    ent->blacklist.current = 0;

  ent->stream = fopen ("/etc/group", "rm");

  if (ent->stream == NULL)
    status = errno == EAGAIN ? NSS_STATUS_TRYAGAIN : NSS_STATUS_UNAVAIL;
  else
    {
      /* We have to make sure the file is  `closed on exec'.  */
      int result, flags;

      result = flags = fcntl (fileno_unlocked (ent->stream), F_GETFD, 0);
      if (result >= 0)
	{
	  flags |= FD_CLOEXEC;
	  result = fcntl (fileno_unlocked (ent->stream), F_SETFD, flags);
	}
      if (result < 0)
	{
	  /* Something went wrong.  Close the stream and return a
	     failure.  */
	  fclose (ent->stream);
	  ent->stream = NULL;
	  status = NSS_STATUS_UNAVAIL;
	}
      else
	/* We take care of locking ourself.  */
	__fsetlocking (ent->stream, FSETLOCKING_BYCALLER);
    }

  return status;
}


static enum nss_status
internal_endgrent (ent_t *ent)
{
  if (ent->stream != NULL)
    {
      fclose (ent->stream);
      ent->stream = NULL;
    }

  if (ent->blacklist.data != NULL)
    {
      ent->blacklist.current = 1;
      ent->blacklist.data[0] = '|';
      ent->blacklist.data[1] = '\0';
    }
  else
    ent->blacklist.current = 0;

  return NSS_STATUS_SUCCESS;
}

/* This function checks, if the user is a member of this group and if
   yes, add the group id to the list.  */
static void
check_and_add_group (const char *user, gid_t group, long int *start,
		     long int *size, gid_t **groupsp, long int limit,
		     struct group *grp)
{
  gid_t *groups = *groupsp;
  char **member;

  /* Don't add main group to list of groups.  */
  if (grp->gr_gid == group)
    return;

  for (member = grp->gr_mem; *member != NULL; ++member)
    if (strcmp (*member, user) == 0)
      {
	/* Matches user.  Insert this group.  */
	if (*start == *size)
	  {
	    /* Need a bigger buffer.  */
	    gid_t *newgroups;
	    long int newsize;

	    if (limit > 0 && *size == limit)
	      /* We reached the maximum.  */
	      return;

	    if (limit <= 0)
	      newsize = 2 * *size;
	    else
	      newsize = MIN (limit, 2 * *size);

	    newgroups = realloc (groups, newsize * sizeof (*groups));
	    if (newgroups == NULL)
	      return;
	    *groupsp = groups = newgroups;
	    *size = newsize;
	  }

	groups[*start] = grp->gr_gid;
	*start += 1;

	break;
      }
}

/* Get the next group from NSS  (+ entry). If the NSS module supports
   initgroups_dyn, get all entries at once.  */
static enum nss_status
getgrent_next_nss (ent_t *ent, char *buffer, size_t buflen, const char *user,
		   gid_t group, long int *start, long int *size,
		   gid_t **groupsp, long int limit, int *errnop)
{
  enum nss_status status;
  struct group grpbuf;

  /* if this module does not support getgrent_r and initgroups_dyn,
     abort. We cannot find the needed group entries.  */
  if (nss_getgrent_r == NULL && nss_initgroups_dyn == NULL)
    return NSS_STATUS_UNAVAIL;

  /* Try nss_initgroups_dyn if supported. We also need getgrgid_r.
     If this function is not supported, step through the whole group
     database with getgrent_r.  */
  if (nss_initgroups_dyn && nss_getgrgid_r)
    {
      long int mystart = 0;
      long int mysize = limit <= 0 ? *size : limit;
      gid_t *mygroups = malloc (mysize * sizeof (gid_t));

      if (mygroups == NULL)
	return NSS_STATUS_TRYAGAIN;

      /* For every gid in the list we get from the NSS module,
         get the whole group entry. We need to do this, since we
         need the group name to check if it is in the blacklist.
         In worst case, this is as twice as slow as stepping with
         getgrent_r through the whole group database. But for large
         group databases this is faster, since the user can only be
         in a limited number of groups.  */
      if (nss_initgroups_dyn (user, group, &mystart, &mysize, &mygroups,
			      limit, errnop) == NSS_STATUS_SUCCESS)
	{
	  /* A temporary buffer. We use the normal buffer, until we find
	     an entry, for which this buffer is to small.  In this case, we
	     overwrite the pointer with one to a bigger buffer.  */
	  char *tmpbuf = buffer;
	  size_t tmplen = buflen;
	  int i;

	  for (i = 0; i < mystart; i++)
	    {
	      while ((status = nss_getgrgid_r (mygroups[i], &grpbuf, tmpbuf,
					       tmplen,
					       errnop)) == NSS_STATUS_TRYAGAIN
		     && *errnop == ERANGE)
		if (tmpbuf == buffer)
		  {
		    tmplen *= 2;
		    tmpbuf = __alloca (tmplen);
		  }
		else
		  tmpbuf = extend_alloca (tmpbuf, tmplen, 2 * tmplen);

	      if (!in_blacklist (grpbuf.gr_name,
				 strlen (grpbuf.gr_name), ent))
		check_and_add_group (user, group, start, size, groupsp,
				     limit, &grpbuf);
	    }

	  free (mygroups);

	  return NSS_STATUS_NOTFOUND;
	}

      free (mygroups);
    }

  /* If we come here, the NSS module does not support initgroups_dyn
     and we have to step through the whole list ourself.  */
  do
    {
      if ((status = nss_getgrent_r (&grpbuf, buffer, buflen, errnop)) !=
	  NSS_STATUS_SUCCESS)
	return status;
    }
  while (in_blacklist (grpbuf.gr_name, strlen (grpbuf.gr_name), ent));

  check_and_add_group (user, group, start, size, groupsp, limit, &grpbuf);
  return NSS_STATUS_SUCCESS;
}

static enum nss_status
internal_getgrent_r (ent_t *ent, char *buffer, size_t buflen, const char *user,
		     gid_t group, long int *start, long int *size,
		     gid_t **groupsp, long int limit, int *errnop)
{
  struct parser_data *data = (void *) buffer;
  struct group grpbuf;

  if (!ent->files)
    return getgrent_next_nss (ent, buffer, buflen, user, group,
			      start, size, groupsp, limit, errnop);

  while (1)
    {
      fpos_t pos;
      int parse_res = 0;
      char *p;

      do
	{
	  /* We need at least 3 characters for one line.  */
	  if (__builtin_expect (buflen < 3, 0))
	    {
	    erange:
	      *errnop = ERANGE;
	      return NSS_STATUS_TRYAGAIN;
	    }

	  fgetpos (ent->stream, &pos);
	  buffer[buflen - 1] = '\xff';
	  p = fgets_unlocked (buffer, buflen, ent->stream);
	  if (p == NULL && feof_unlocked (ent->stream))
	    return NSS_STATUS_NOTFOUND;

	  if (p == NULL || __builtin_expect (buffer[buflen - 1] != '\xff', 0))
	    {
	    erange_reset:
	      fsetpos (ent->stream, &pos);
	      goto erange;
	    }

	  /* Terminate the line for any case.  */
	  buffer[buflen - 1] = '\0';

	  /* Skip leading blanks.  */
	  while (isspace (*p))
	    ++p;
	}
      while (*p == '\0' || *p == '#' ||	/* Ignore empty and comment lines. */
	     /* Parse the line.  If it is invalid, loop to
	        get the next line of the file to parse.  */
	     !(parse_res = _nss_files_parse_grent (p, &grpbuf, data, buflen,
						   errnop)));

      if (__builtin_expect (parse_res == -1, 0))
	/* The parser ran out of space.  */
	goto erange_reset;

      if (grpbuf.gr_name[0] != '+' && grpbuf.gr_name[0] != '-')
	/* This is a real entry.  */
	break;

      /* -group */
      if (grpbuf.gr_name[0] == '-' && grpbuf.gr_name[1] != '\0'
	  && grpbuf.gr_name[1] != '@')
	{
	  blacklist_store_name (&grpbuf.gr_name[1], ent);
	  continue;
	}

      /* +group */
      if (grpbuf.gr_name[0] == '+' && grpbuf.gr_name[1] != '\0'
	  && grpbuf.gr_name[1] != '@')
	{
	  if (in_blacklist (&grpbuf.gr_name[1],
			    strlen (&grpbuf.gr_name[1]), ent))
	    continue;
	  /* Store the group in the blacklist for the "+" at the end of
	     /etc/group */
	  blacklist_store_name (&grpbuf.gr_name[1], ent);
	  if (nss_getgrnam_r == NULL)
	    return NSS_STATUS_UNAVAIL;
	  else if (nss_getgrnam_r (&grpbuf.gr_name[1], &grpbuf, buffer,
				   buflen, errnop) != NSS_STATUS_SUCCESS)
	    continue;

	  check_and_add_group (user, group, start, size, groupsp,
			       limit, &grpbuf);

	  return NSS_STATUS_SUCCESS;
	}

      /* +:... */
      if (grpbuf.gr_name[0] == '+' && grpbuf.gr_name[1] == '\0')
	{
	  ent->files = FALSE;
	  return getgrent_next_nss (ent, buffer, buflen, user, group,
				    start, size, groupsp, limit, errnop);
	}
    }

  check_and_add_group (user, group, start, size, groupsp, limit, &grpbuf);

  return NSS_STATUS_SUCCESS;
}


enum nss_status
_nss_compat_initgroups_dyn (const char *user, gid_t group, long int *start,
			    long int *size, gid_t **groupsp, long int limit,
			    int *errnop)
{
  size_t buflen = sysconf (_SC_GETPW_R_SIZE_MAX);
  char *tmpbuf;
  enum nss_status status;
  ent_t intern = { TRUE, NULL, {NULL, 0, 0} };

  status = internal_setgrent (&intern);
  if (status != NSS_STATUS_SUCCESS)
    return status;

  tmpbuf = __alloca (buflen);

  do
    {
      while ((status = internal_getgrent_r (&intern, tmpbuf, buflen,
					    user, group, start, size,
					    groupsp, limit, errnop))
	     == NSS_STATUS_TRYAGAIN && *errnop == ERANGE)
	tmpbuf = extend_alloca (tmpbuf, buflen, 2 * buflen);
    }
  while (status == NSS_STATUS_SUCCESS);

  internal_endgrent (&intern);

  return NSS_STATUS_SUCCESS;
}


/* Support routines for remembering -@netgroup and -user entries.
   The names are stored in a single string with `|' as separator. */
static void
blacklist_store_name (const char *name, ent_t *ent)
{
  int namelen = strlen (name);
  char *tmp;

  /* First call, setup cache.  */
  if (ent->blacklist.size == 0)
    {
      ent->blacklist.size = MAX (BLACKLIST_INITIAL_SIZE, 2 * namelen);
      ent->blacklist.data = malloc (ent->blacklist.size);
      if (ent->blacklist.data == NULL)
	return;
      ent->blacklist.data[0] = '|';
      ent->blacklist.data[1] = '\0';
      ent->blacklist.current = 1;
    }
  else
    {
      if (in_blacklist (name, namelen, ent))
	return;			/* no duplicates */

      if (ent->blacklist.current + namelen + 1 >= ent->blacklist.size)
	{
	  ent->blacklist.size += MAX (BLACKLIST_INCREMENT, 2 * namelen);
	  tmp = realloc (ent->blacklist.data, ent->blacklist.size);
	  if (tmp == NULL)
	    {
	      free (ent->blacklist.data);
	      ent->blacklist.size = 0;
	      return;
	    }
	  ent->blacklist.data = tmp;
	}
    }

  tmp = stpcpy (ent->blacklist.data + ent->blacklist.current, name);
  *tmp++ = '|';
  *tmp = '\0';
  ent->blacklist.current += namelen + 1;

  return;
}

/* returns TRUE if ent->blacklist contains name, else FALSE */
static bool_t
in_blacklist (const char *name, int namelen, ent_t *ent)
{
  char buf[namelen + 3];
  char *cp;

  if (ent->blacklist.data == NULL)
    return FALSE;

  buf[0] = '|';
  cp = stpcpy (&buf[1], name);
  *cp++ = '|';
  *cp = '\0';
  return strstr (ent->blacklist.data, buf) != NULL;
}
