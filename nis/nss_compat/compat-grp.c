/* Copyright (C) 1996 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Thorsten Kukuk <kukuk@vt.uni-paderborn.de>, 1996.

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
#include <nss.h>
#include <grp.h>
#include <ctype.h>
#include <libc-lock.h>
#include <string.h>
#include <rpcsvc/yp.h>
#include <rpcsvc/ypclnt.h>

/* Get the declaration of the parser function.  */
#define ENTNAME grent
#define STRUCTURE group
#define EXTERN_PARSER
#include "../../nss/nss_files/files-parse.c"

/* Structure for remembering -@netgroup and -user members ... */
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
    bool_t nis;
    bool_t nis_first;
    char *oldkey;
    int oldkeylen;
    FILE *stream;
    struct blacklist_t blacklist;
  };
typedef struct ent_t ent_t;

static ent_t ext_ent = {0, 0, NULL, 0, NULL, {NULL, 0, 0}};

/* Protect global state against multiple changers.  */
__libc_lock_define_initialized (static, lock)

/* Prototypes for local functions.  */
static void blacklist_store_name (const char *, ent_t *);
static int in_blacklist (const char *, int, ent_t *);

static enum nss_status
internal_setgrent (ent_t *ent)
{
  enum nss_status status = NSS_STATUS_SUCCESS;

  ent->nis = ent->nis_first = 0;

  if (ent->oldkey != NULL)
    {
      free (ent->oldkey);
      ent->oldkey = NULL;
      ent->oldkeylen = 0;
    }

  ent->blacklist.current = 0;
  if (ent->blacklist.data != NULL)
    ent->blacklist.data[0] = '\0';

  if (ent->stream == NULL)
    {
      ent->stream = fopen ("/etc/group", "r");

      if (ent->stream == NULL)
	status = errno == EAGAIN ? NSS_STATUS_TRYAGAIN : NSS_STATUS_UNAVAIL;
    }
  else
    rewind (ent->stream);

  return status;
}


enum nss_status
_nss_compat_setgrent (void)
{
  enum nss_status result;

  __libc_lock_lock (lock);

  result = internal_setgrent (&ext_ent);

  __libc_lock_unlock (lock);

  return result;
}


static enum nss_status
internal_endgrent (ent_t *ent)
{
  if (ent->stream != NULL)
    {
      fclose (ent->stream);
      ent->stream = NULL;
    }

  ent->nis = ent->nis_first = 0;

  if (ent->oldkey != NULL)
    {
      free (ent->oldkey);
      ent->oldkey = NULL;
      ent->oldkeylen = 0;
    }

  ent->blacklist.current = 0;
  if (ent->blacklist.data != NULL)
    ent->blacklist.data[0] = '\0';

  return NSS_STATUS_SUCCESS;
}

enum nss_status
_nss_compat_endgrent (void)
{
  enum nss_status result;

  __libc_lock_lock (lock);

  result = internal_endgrent (&ext_ent);

  __libc_lock_unlock (lock);

  return result;
}

static enum nss_status
getgrent_next_nis (struct group *result, ent_t *ent, char *buffer,
		   size_t buflen)
{
  struct parser_data *data = (void *) buffer;
  char *domain;
  char *outkey, *outval;
  int outkeylen, outvallen;
  char *p;

  if (yp_get_default_domain (&domain) != YPERR_SUCCESS)
    {
      ent->nis = 0;
      return NSS_STATUS_NOTFOUND;
    }

  do
    {
      if (ent->nis_first)
	{
	  if (yp_first (domain, "group.byname", &outkey, &outkeylen,
			&outval, &outvallen) != YPERR_SUCCESS)
	    {
	      ent->nis = 0;
	      return NSS_STATUS_UNAVAIL;
	    }

	  ent->oldkey = outkey;
	  ent->oldkeylen = outkeylen;
	  ent->nis_first = FALSE;
	}
      else
	{
	  if (yp_next (domain, "group.byname", ent->oldkey, ent->oldkeylen,
		       &outkey, &outkeylen, &outval, &outvallen)
	      != YPERR_SUCCESS)
	    {
	      ent->nis = 0;
	      return NSS_STATUS_NOTFOUND;
	    }

	  free (ent->oldkey);
	  ent->oldkey = outkey;
	  ent->oldkeylen = outkeylen;
	}

      /* Copy the found data to our buffer  */
      p = strncpy (buffer, outval, buflen);

      /* ...and free the data.  */
      free (outval);

      while (isspace (*p))
	++p;
    }
  while (!_nss_files_parse_grent (p, result, data, buflen));

  if (!in_blacklist (result->gr_name, strlen (result->gr_name), ent))
    return NSS_STATUS_SUCCESS;
  else
    return NSS_STATUS_NOTFOUND;
}


static enum nss_status
getgrent_next_file (struct group *result, ent_t *ent,
		    char *buffer, size_t buflen)
{
  struct parser_data *data = (void *) buffer;
  while (1)
    {
      char *p;

      do
	{
	  p = fgets (buffer, buflen, ent->stream);
	  if (p == NULL)
	    return NSS_STATUS_NOTFOUND;

	  /* Terminate the line for any case.  */
	  buffer[buflen - 1] = '\0';

	  /* Skip leading blanks.  */
	  while (isspace (*p))
	    ++p;
	}
      /* Ignore empty and comment lines.  */
      while (*p == '\0' || *p == '#' ||
      /* Parse the line.  If it is invalid, loop to
         get the next line of the file to parse.  */
	     !_nss_files_parse_grent (p, result, data, buflen));

      if (result->gr_name[0] != '+' && result->gr_name[0] != '-')
	/* This is a real entry.  */
	break;

      /* -group */
      if (result->gr_name[0] == '-' && result->gr_name[1] != '\0'
	  && result->gr_name[1] != '@')
	{
	  blacklist_store_name (&result->gr_name[1], ent);
	  continue;
	}

      /* +group */
      if (result->gr_name[0] == '+' && result->gr_name[1] != '\0'
	  && result->gr_name[1] != '@')
	{
	  char *domain;
	  char *outval;
	  int outvallen;

	  if (yp_get_default_domain (&domain) != YPERR_SUCCESS)
	    /* XXX Should we regard this as an fatal error?  I don't
	       think so.  Just continue working.  --drepper@gnu  */
	    continue;

	  if (yp_match (domain, "group.byname", &result->gr_name[1],
			strlen (result->gr_name) - 1, &outval, &outvallen)
	      != YPERR_SUCCESS)
	    continue;

	  p = strncpy (buffer, outval, buflen);
	  while (isspace (*p))
	    p++;
	  free (outval);
	  if (_nss_files_parse_grent (p, result, data, buflen))
	    /* We found the entry.  */
	    break;
	}

      /* +:... */
      if (result->gr_name[0] == '+' && result->gr_name[1] == '\0')
	{
	  ent->nis = TRUE;
	  ent->nis_first = TRUE;

	  return getgrent_next_nis (result, ent, buffer, buflen);
	}
    }

  return NSS_STATUS_SUCCESS;
}


static enum nss_status
internal_getgrent_r (struct group *gr, ent_t *ent, char *buffer,
		     size_t buflen)
{
  if (ent->nis)
    return getgrent_next_nis (gr, ent, buffer, buflen);
  else
    return getgrent_next_file (gr, ent, buffer, buflen);
}

enum nss_status
_nss_compat_getgrent_r (struct group *grp, char *buffer, size_t buflen)
{
  enum nss_status status = NSS_STATUS_SUCCESS;

  __libc_lock_lock (lock);

  /* Be prepared that the setgrent function was not called before.  */
  if (ext_ent.stream == NULL)
    status = internal_setgrent (&ext_ent);

  if (status == NSS_STATUS_SUCCESS)
    status = internal_getgrent_r (grp, &ext_ent, buffer, buflen);

  __libc_lock_unlock (lock);

  return status;
}


enum nss_status
_nss_compat_getgrnam_r (const char *name, struct group *grp,
			char *buffer, size_t buflen)
{
  ent_t ent = {0, 0, NULL, 0, NULL, {NULL, 0, 0}};
  enum nss_status status;

  if (name[0] == '-' || name[0] == '+')
    return NSS_STATUS_NOTFOUND;


  status = internal_setgrent (&ent);
  if (status != NSS_STATUS_SUCCESS)
    return status;

  while ((status = internal_getgrent_r (grp, &ent, buffer, buflen))
	 == NSS_STATUS_SUCCESS)
    if (strcmp (grp->gr_name, name) == 0)
      break;

  internal_endgrent (&ent);
  return status;
}


enum nss_status
_nss_compat_getgrgid_r (gid_t gid, struct group *grp,
			char *buffer, size_t buflen)
{
  ent_t ent = {0, 0, NULL, 0, NULL, {NULL, 0, 0}};
  enum nss_status status;

  status = internal_setgrent (&ent);
  if (status != NSS_STATUS_SUCCESS)
    return status;

  while ((status = internal_getgrent_r (grp, &ent, buffer, buflen))
	 == NSS_STATUS_SUCCESS)
    if (grp->gr_gid == gid && grp->gr_name[0] != '+' && grp->gr_name[0] != '-')
      break;

  internal_endgrent (&ent);
  return status;
}


/* Support routines for remembering -@netgroup and -user entries.
   The names are stored in a single string with `|' as separator. */
static void
blacklist_store_name (const char *name, ent_t *ent)
{
  int namelen = strlen (name);
  char *tmp;

  /* first call, setup cache */
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

  if (ent->blacklist.data == NULL)
    return FALSE;

  stpcpy (stpcpy (stpcpy (buf, "|"), name), "|");
  return strstr (ent->blacklist.data, buf) != NULL;
}
