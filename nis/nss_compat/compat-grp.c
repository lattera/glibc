/* Copyright (C) 1996, 1997, 1998, 1999, 2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Thorsten Kukuk <kukuk@suse.de>, 1996.

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
#include <fcntl.h>
#include <nss.h>
#include <grp.h>
#include <ctype.h>
#include <bits/libc-lock.h>
#include <string.h>
#include <rpcsvc/yp.h>
#include <rpcsvc/ypclnt.h>
#include <rpcsvc/nis.h>
#include <nsswitch.h>

#include "nss-nisplus.h"
#include "nisplus-parser.h"

static service_user *ni;
static bool_t use_nisplus; /* default: group_compat: nis */
static nis_name grptable; /* Name of the group table */
static size_t grptablelen;

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
    bool_t nis;
    bool_t nis_first;
    char *oldkey;
    int oldkeylen;
    nis_result *result;
    FILE *stream;
    struct blacklist_t blacklist;
};
typedef struct ent_t ent_t;

static ent_t ext_ent = {0, 0, NULL, 0, NULL, NULL, {NULL, 0, 0}};

/* Protect global state against multiple changers.  */
__libc_lock_define_initialized (static, lock)

/* Prototypes for local functions.  */
static void blacklist_store_name (const char *, ent_t *);
static int in_blacklist (const char *, int, ent_t *);

static enum nss_status
_nss_first_init (void)
{
  if (ni == NULL)
    {
      __nss_database_lookup ("group_compat", NULL, "nis", &ni);
      use_nisplus = (strcmp (ni->name, "nisplus") == 0);
    }

  if (grptable == NULL)
    {
      static const char key[] = "group.org_dir.";
      const char *local_dir = nis_local_directory ();
      size_t len_local_dir = strlen (local_dir);

      grptable = malloc (sizeof (key) + len_local_dir);
      if (grptable == NULL)
        return NSS_STATUS_TRYAGAIN;

      grptablelen = ((char *) mempcpy (mempcpy (grptable,
						key, sizeof (key) - 1),
				       local_dir, len_local_dir + 1)
		     - grptable) - 1;
    }

  return NSS_STATUS_SUCCESS;
}

static enum nss_status
internal_setgrent (ent_t *ent)
{
  enum nss_status status = NSS_STATUS_SUCCESS;

  ent->nis = ent->nis_first = 0;

  if (_nss_first_init () != NSS_STATUS_SUCCESS)
    return NSS_STATUS_UNAVAIL;

  if (ent->oldkey != NULL)
    {
      free (ent->oldkey);
      ent->oldkey = NULL;
      ent->oldkeylen = 0;
    }

  if (ent->result != NULL)
    {
      nis_freeresult (ent->result);
      ent->result = NULL;
    }

  if (ent->blacklist.data != NULL)
    {
      ent->blacklist.current = 1;
      ent->blacklist.data[0] = '|';
      ent->blacklist.data[1] = '\0';
    }
  else
    ent->blacklist.current = 0;

  if (ent->stream == NULL)
    {
      ent->stream = fopen ("/etc/group", "r");

      if (ent->stream == NULL)
	status = errno == EAGAIN ? NSS_STATUS_TRYAGAIN : NSS_STATUS_UNAVAIL;
      else
	{
	  /* We have to make sure the file is  `closed on exec'.  */
	  int result, flags;

	  result = flags = fcntl (fileno (ent->stream), F_GETFD, 0);
	  if (result >= 0)
	    {
	      flags |= FD_CLOEXEC;
	      result = fcntl (fileno (ent->stream), F_SETFD, flags);
	    }
	  if (result < 0)
	    {
	      /* Something went wrong.  Close the stream and return a
		 failure.  */
	      fclose (ent->stream);
	      ent->stream = NULL;
	      status = NSS_STATUS_UNAVAIL;
	    }
	}
    }
  else
    rewind (ent->stream);

  return status;
}


enum nss_status
_nss_compat_setgrent (int stayopen)
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

  if (ent->result != NULL)
    {
      nis_freeresult (ent->result);
      ent->result = NULL;
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
		   size_t buflen, int *errnop)
{
  struct parser_data *data = (void *) buffer;
  char *domain;
  char *outkey, *outval;
  int outkeylen, outvallen, parse_res;
  char *p;

  if (yp_get_default_domain (&domain) != YPERR_SUCCESS)
    {
      ent->nis = 0;
      *errnop = ENOENT;
      return NSS_STATUS_NOTFOUND;
    }

  do
    {
      char *save_oldkey;
      int save_oldlen;
      bool_t save_nis_first;

      if (ent->nis_first)
	{
	  if (yp_first (domain, "group.byname", &outkey, &outkeylen,
			&outval, &outvallen) != YPERR_SUCCESS)
	    {
	      ent->nis = 0;
	      return NSS_STATUS_UNAVAIL;
	    }

	  if ( buflen < ((size_t) outvallen + 1))
	    {
	      free (outval);
	      *errnop = ERANGE;
	      return NSS_STATUS_TRYAGAIN;
	    }

	  save_oldkey = ent->oldkey;
	  save_oldlen = ent->oldkeylen;
	  save_nis_first = TRUE;
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
	      *errnop = ENOENT;
	      return NSS_STATUS_NOTFOUND;
	    }

	  if ( buflen < ((size_t) outvallen + 1))
	    {
	      free (outval);
	      *errnop = ERANGE;
	      return NSS_STATUS_TRYAGAIN;
	    }

	  save_oldkey = ent->oldkey;
	  save_oldlen = ent->oldkeylen;
	  save_nis_first = FALSE;
	  ent->oldkey = outkey;
	  ent->oldkeylen = outkeylen;
	}

      /* Copy the found data to our buffer...  */
      p = strncpy (buffer, outval, buflen);

      /* ...and free the data.  */
      free (outval);

      while (isspace (*p))
	++p;

      parse_res = _nss_files_parse_grent (p, result, data, buflen, errnop);
      if (parse_res == -1)
	{
	  free (ent->oldkey);
	  ent->oldkey = save_oldkey;
	  ent->oldkeylen = save_oldlen;
	  ent->nis_first = save_nis_first;
	  *errnop = ERANGE;
	  return NSS_STATUS_TRYAGAIN;
	}
      else
	{
	  if (!save_nis_first)
	    free (save_oldkey);
	}

      if (parse_res &&
	  in_blacklist (result->gr_name, strlen (result->gr_name), ent))
	parse_res = 0; /* if result->gr_name in blacklist,search next entry */
    }
  while (!parse_res);

  return NSS_STATUS_SUCCESS;
}

static enum nss_status
getgrent_next_nisplus (struct group *result, ent_t *ent, char *buffer,
                       size_t buflen, int *errnop)
{
  int parse_res;

  do
    {
      nis_result *save_oldres;
      bool_t save_nis_first;

      if (ent->nis_first)
        {
	  save_oldres = ent->result;
	  save_nis_first = TRUE;
          ent->result = nis_first_entry(grptable);
          if (niserr2nss (ent->result->status) != NSS_STATUS_SUCCESS)
            {
              ent->nis = 0;
              return niserr2nss (ent->result->status);
            }
          ent->nis_first = FALSE;
        }
      else
        {
          nis_result *res;

	  save_oldres = ent->result;
	  save_nis_first = FALSE;
          res = nis_next_entry(grptable, &ent->result->cookie);
          ent->result = res;
          if (niserr2nss (ent->result->status) != NSS_STATUS_SUCCESS)
            {
	      ent->nis = 0;
	      return niserr2nss (ent->result->status);
            }
        }
      parse_res = _nss_nisplus_parse_grent (ent->result, 0, result,
					    buffer, buflen, errnop);
      if (parse_res == -1)
	{
	  nis_freeresult (ent->result);
	  ent->result = save_oldres;
	  ent->nis_first = save_nis_first;
	  *errnop = ERANGE;
	  return NSS_STATUS_TRYAGAIN;
	}
      else
	{
	  if (!save_nis_first)
	    nis_freeresult (save_oldres);
	}

      if (parse_res &&
          in_blacklist (result->gr_name, strlen (result->gr_name), ent))
        parse_res = 0; /* if result->gr_name in blacklist,search next entry */
    }
  while (!parse_res);

  return NSS_STATUS_SUCCESS;
}

/* This function handle the +group entrys in /etc/group */
static enum nss_status
getgrnam_plusgroup (const char *name, struct group *result, char *buffer,
		    size_t buflen, int *errnop)
{
  struct parser_data *data = (void *) buffer;
  int parse_res;

  if (use_nisplus) /* Do the NIS+ query here */
    {
      nis_result *res;
      char buf[strlen (name) + 24 + grptablelen];

      sprintf(buf, "[name=%s],%s", name, grptable);
      res = nis_list(buf, FOLLOW_PATH | FOLLOW_LINKS, NULL, NULL);
      if (niserr2nss (res->status) != NSS_STATUS_SUCCESS)
        {
          enum nss_status status =  niserr2nss (res->status);

          nis_freeresult (res);
          return status;
        }
      parse_res = _nss_nisplus_parse_grent (res, 0, result, buffer, buflen,
					    errnop);
      if (parse_res == -1)
	{
	  nis_freeresult (res);
	  *errnop = ERANGE;
	  return NSS_STATUS_TRYAGAIN;
	}
      nis_freeresult (res);
    }
  else /* Use NIS */
    {
      char *domain, *outval, *p;
      int outvallen;

      if (yp_get_default_domain (&domain) != YPERR_SUCCESS)
	{
	  *errnop = ENOENT;
	  return NSS_STATUS_NOTFOUND;
	}

      if (yp_match (domain, "group.byname", name, strlen (name),
		    &outval, &outvallen) != YPERR_SUCCESS)
	{
	  *errnop = ENOENT;
	  return NSS_STATUS_NOTFOUND;
	}

      if (buflen < ((size_t) outvallen + 1))
	{
	  free (outval);
	  *errnop = ERANGE;
	  return NSS_STATUS_TRYAGAIN;
	}

      /* Copy the found data to our buffer...  */
      p = strncpy (buffer, outval, buflen);

      /* ... and free the data.  */
      free (outval);
      while (isspace (*p))
        ++p;
      parse_res = _nss_files_parse_grent (p, result, data, buflen, errnop);
      if (parse_res == -1)
	return NSS_STATUS_TRYAGAIN;
    }

  if (parse_res)
    /* We found the entry.  */
    return NSS_STATUS_SUCCESS;
  else
    return NSS_STATUS_RETURN;
}

static enum nss_status
getgrent_next_file (struct group *result, ent_t *ent,
		    char *buffer, size_t buflen, int *errnop)
{
  struct parser_data *data = (void *) buffer;
  while (1)
    {
      fpos_t pos;
      int parse_res = 0;
      char *p;

      do
	{
	  fgetpos (ent->stream, &pos);
	  buffer[buflen - 1] = '\xff';
	  p = fgets (buffer, buflen, ent->stream);
	  if (p == NULL && feof (ent->stream))
	    {
	      *errnop = ENOENT;
	      return NSS_STATUS_NOTFOUND;
	    }
	  if (p == NULL || buffer[buflen - 1] != '\xff')
	    {
	      fsetpos (ent->stream, &pos);
	      *errnop = ERANGE;
	      return NSS_STATUS_TRYAGAIN;
	    }

	  /* Terminate the line for any case.  */
	  buffer[buflen - 1] = '\0';

	  /* Skip leading blanks.  */
	  while (isspace (*p))
	    ++p;
	}
      while (*p == '\0' || *p == '#' || /* Ignore empty and comment lines. */
      /* Parse the line.  If it is invalid, loop to
         get the next line of the file to parse.  */
	     !(parse_res = _nss_files_parse_grent (p, result, data, buflen,
						   errnop)));

      if (parse_res == -1)
	{
	  /* The parser ran out of space.  */
	  fsetpos (ent->stream, &pos);
	  *errnop = ERANGE;
	  return NSS_STATUS_TRYAGAIN;
	}

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
          enum nss_status status;

	  /* Store the group in the blacklist for the "+" at the end of
	     /etc/group */
	  blacklist_store_name (&result->gr_name[1], ent);
	  status = getgrnam_plusgroup (&result->gr_name[1], result, buffer,
				       buflen, errnop);
          if (status == NSS_STATUS_SUCCESS) /* We found the entry. */
            break;
          else
            if (status == NSS_STATUS_RETURN /* We couldn't parse the entry */
		|| status == NSS_STATUS_NOTFOUND) /* No group in NIS */
              continue;
            else
	      {
		if (status == NSS_STATUS_TRYAGAIN)
		  {
		    /* The parser ran out of space.  */
		    fsetpos (ent->stream, &pos);
		    *errnop = ERANGE;
		  }
		return status;
	      }
	}

      /* +:... */
      if (result->gr_name[0] == '+' && result->gr_name[1] == '\0')
	{
	  ent->nis = TRUE;
	  ent->nis_first = TRUE;

	  if (use_nisplus)
	    return getgrent_next_nisplus (result, ent, buffer, buflen, errnop);
	  else
	    return getgrent_next_nis (result, ent, buffer, buflen, errnop);
	}
    }

  return NSS_STATUS_SUCCESS;
}


static enum nss_status
internal_getgrent_r (struct group *gr, ent_t *ent, char *buffer,
		     size_t buflen, int *errnop)
{
  if (ent->nis)
    {
      if (use_nisplus)
	return getgrent_next_nisplus (gr, ent, buffer, buflen, errnop);
      else
	return getgrent_next_nis (gr, ent, buffer, buflen, errnop);
    }
  else
    return getgrent_next_file (gr, ent, buffer, buflen, errnop);
}

enum nss_status
_nss_compat_getgrent_r (struct group *grp, char *buffer, size_t buflen,
			int *errnop)
{
  enum nss_status status = NSS_STATUS_SUCCESS;

  __libc_lock_lock (lock);

  /* Be prepared that the setgrent function was not called before.  */
  if (ext_ent.stream == NULL)
    status = internal_setgrent (&ext_ent);

  if (status == NSS_STATUS_SUCCESS)
    status = internal_getgrent_r (grp, &ext_ent, buffer, buflen, errnop);

  __libc_lock_unlock (lock);

  return status;
}

/* Searches in /etc/group and the NIS/NIS+ map for a special group */
static enum nss_status
internal_getgrnam_r (const char *name, struct group *result, ent_t *ent,
		     char *buffer, size_t buflen, int *errnop)
{
  struct parser_data *data = (void *) buffer;
  while (1)
    {
      fpos_t pos;
      int parse_res = 0;
      char *p;

      do
	{
	  fgetpos (ent->stream, &pos);
	  buffer[buflen - 1] = '\xff';
	  p = fgets (buffer, buflen, ent->stream);
	  if (p == NULL && feof (ent->stream))
	    {
	      *errnop = ENOENT;
	      return NSS_STATUS_NOTFOUND;
	    }
	  if (p == NULL || buffer[buflen - 1] != '\xff')
	    {
	      fsetpos (ent->stream, &pos);
	      *errnop = ERANGE;
	      return NSS_STATUS_TRYAGAIN;
	    }

	  /* Terminate the line for any case.  */
	  buffer[buflen - 1] = '\0';

	  /* Skip leading blanks.  */
	  while (isspace (*p))
	    ++p;
	}
      while (*p == '\0' || *p == '#' || /* Ignore empty and comment lines. */
      /* Parse the line.  If it is invalid, loop to
         get the next line of the file to parse.  */
	     !(parse_res = _nss_files_parse_grent (p, result, data, buflen,
						   errnop)));

      if (parse_res == -1)
	{
	  /* The parser ran out of space.  */
	  fsetpos (ent->stream, &pos);
	  *errnop = ERANGE;
	  return NSS_STATUS_TRYAGAIN;
	}

      /* This is a real entry.  */
      if (result->gr_name[0] != '+' && result->gr_name[0] != '-')
	{
	  if (strcmp (result->gr_name, name) == 0)
	    return NSS_STATUS_SUCCESS;
	  else
	    continue;
	}

      /* -group */
      if (result->gr_name[0] == '-' && result->gr_name[1] != '\0')
	{
	  if (strcmp (&result->gr_name[1], name) == 0)
	    {
	      *errnop = ENOENT;
	      return NSS_STATUS_NOTFOUND;
	    }
	  else
	    continue;
	}

      /* +group */
      if (result->gr_name[0] == '+' && result->gr_name[1] != '\0')
	{
	  if (strcmp (name, &result->gr_name[1]) == 0)
	    {
	      enum nss_status status;

	      status = getgrnam_plusgroup (name, result, buffer, buflen,
					   errnop);
	      if (status == NSS_STATUS_RETURN)
		/* We couldn't parse the entry */
		continue;
	      else
		return status;
	    }
	}
      /* +:... */
      if (result->gr_name[0] == '+' && result->gr_name[1] == '\0')
	{
	  enum nss_status status;

	  status = getgrnam_plusgroup (name, result, buffer, buflen, errnop);
	  if (status == NSS_STATUS_RETURN)
	    /* We couldn't parse the entry */
	    continue;
	  else
	    return status;
	}
    }

  return NSS_STATUS_SUCCESS;
}

enum nss_status
_nss_compat_getgrnam_r (const char *name, struct group *grp,
			char *buffer, size_t buflen, int *errnop)
{
  ent_t ent = {0, 0, NULL, 0, NULL, NULL, {NULL, 0, 0}};
  enum nss_status status;

  if (name[0] == '-' || name[0] == '+')
    {
      *errnop = ENOENT;
      return NSS_STATUS_NOTFOUND;
    }

  __libc_lock_lock (lock);

  status = internal_setgrent (&ent);

  __libc_lock_unlock (lock);

  if (status != NSS_STATUS_SUCCESS)
    return status;

  status = internal_getgrnam_r (name, grp, &ent, buffer, buflen, errnop);

  internal_endgrent (&ent);

  return status;
}

/* This function handle the + entry in /etc/group */
static enum nss_status
getgrgid_plusgroup (gid_t gid, struct group *result, char *buffer,
		    size_t buflen, int *errnop)
{
  struct parser_data *data = (void *) buffer;
  int parse_res;

  if (use_nisplus) /* Do the NIS+ query here */
    {
      nis_result *res;
      char buf[24 + grptablelen];

      sprintf(buf, "[gid=%d],%s", gid, grptable);
      res = nis_list(buf, FOLLOW_PATH | FOLLOW_LINKS, NULL, NULL);
      if (niserr2nss (res->status) != NSS_STATUS_SUCCESS)
        {
          enum nss_status status =  niserr2nss (res->status);

          nis_freeresult (res);
          return status;
        }
      if ((parse_res = _nss_nisplus_parse_grent (res, 0, result, buffer,
						 buflen, errnop)) == -1)
	{
	  nis_freeresult (res);
	  *errnop = ERANGE;
	  return NSS_STATUS_TRYAGAIN;
	}
      nis_freeresult (res);
    }
  else /* Use NIS */
    {
      char buf[24];
      char *domain, *outval, *p;
      int outvallen;

      if (yp_get_default_domain (&domain) != YPERR_SUCCESS)
	{
	  *errnop = ENOENT;
	  return NSS_STATUS_NOTFOUND;
	}

      snprintf (buf, sizeof (buf), "%d", gid);

      if (yp_match (domain, "group.bygid", buf, strlen (buf),
		    &outval, &outvallen) != YPERR_SUCCESS)
	{
	  *errnop = ENOENT;
	  return NSS_STATUS_NOTFOUND;
	}

      if (buflen < ((size_t) outvallen + 1))
	{
	  free (outval);
	  *errnop = ERANGE;
	  return NSS_STATUS_TRYAGAIN;
	}

      /* Copy the found data to our buffer...  */
      p = strncpy (buffer, outval, buflen);

      /* ... and free the data.  */
      free (outval);

      while (isspace (*p))
        p++;
      parse_res = _nss_files_parse_grent (p, result, data, buflen, errnop);
      if (parse_res == -1)
	return NSS_STATUS_TRYAGAIN;
    }

  if (parse_res)
    /* We found the entry.  */
    return NSS_STATUS_SUCCESS;
  else
    return NSS_STATUS_RETURN;
}

/* Searches in /etc/group and the NIS/NIS+ map for a special group id */
static enum nss_status
internal_getgrgid_r (gid_t gid, struct group *result, ent_t *ent,
		     char *buffer, size_t buflen, int *errnop)
{
  struct parser_data *data = (void *) buffer;
  while (1)
    {
      fpos_t pos;
      int parse_res = 0;
      char *p;

      do
	{
	  fgetpos (ent->stream, &pos);
	  buffer[buflen - 1] = '\xff';
	  p = fgets (buffer, buflen, ent->stream);
	  if (p == NULL && feof (ent->stream))
	    {
	      *errnop = ENOENT;
	      return NSS_STATUS_NOTFOUND;
	    }
	  if (p == NULL || buffer[buflen - 1] != '\xff')
	    {
	      fsetpos (ent->stream, &pos);
	      *errnop = ERANGE;
	      return NSS_STATUS_TRYAGAIN;
	    }

	  /* Terminate the line for any case.  */
	  buffer[buflen - 1] = '\0';

	  /* Skip leading blanks.  */
	  while (isspace (*p))
	    ++p;
	}
      while (*p == '\0' || *p == '#' || /* Ignore empty and comment lines. */
      /* Parse the line.  If it is invalid, loop to
         get the next line of the file to parse.  */
	     !(parse_res = _nss_files_parse_grent (p, result, data, buflen,
						   errnop)));

      if (parse_res == -1)
	{
	  /* The parser ran out of space.  */
	  fsetpos (ent->stream, &pos);
	  *errnop = ERANGE;
	  return NSS_STATUS_TRYAGAIN;
	}

      /* This is a real entry.  */
      if (result->gr_name[0] != '+' && result->gr_name[0] != '-')
	{
	  if (result->gr_gid == gid)
	    return NSS_STATUS_SUCCESS;
	  else
	    continue;
	}

      /* -group */
      if (result->gr_name[0] == '-' && result->gr_name[1] != '\0')
	{
          blacklist_store_name (&result->gr_name[1], ent);
          continue;
	}

      /* +group */
      if (result->gr_name[0] == '+' && result->gr_name[1] != '\0')
	{
	  enum nss_status status;

	  /* Store the group in the blacklist for the "+" at the end of
             /etc/group */
          blacklist_store_name (&result->gr_name[1], ent);
	  status = getgrnam_plusgroup (&result->gr_name[1], result, buffer,
				      buflen, errnop);
	  if (status == NSS_STATUS_SUCCESS && result->gr_gid == gid)
	    break;
	  else
	    continue;
	}
      /* +:... */
      if (result->gr_name[0] == '+' && result->gr_name[1] == '\0')
	{
	  enum nss_status status;

	  status = getgrgid_plusgroup (gid, result, buffer, buflen, errnop);
	  if (status == NSS_STATUS_RETURN) /* We couldn't parse the entry */
	    {
	      *errnop = ENOENT;
	      return NSS_STATUS_NOTFOUND;
	    }
	  else
	    return status;
	}
    }

  return NSS_STATUS_SUCCESS;
}

enum nss_status
_nss_compat_getgrgid_r (gid_t gid, struct group *grp,
			char *buffer, size_t buflen, int *errnop)
{
  ent_t ent = {0, 0, NULL, 0, NULL, NULL, {NULL, 0, 0}};
  enum nss_status status;

  __libc_lock_lock (lock);

  status = internal_setgrent (&ent);

  __libc_lock_unlock (lock);

  if (status != NSS_STATUS_SUCCESS)
    return status;

  status = internal_getgrgid_r (gid, grp, &ent, buffer, buflen, errnop);

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
  char *cp;

  if (ent->blacklist.data == NULL)
    return FALSE;

  buf[0] = '|';
  cp = stpcpy (&buf[1], name);
  *cp++= '|';
  *cp = '\0';
  return strstr (ent->blacklist.data, buf) != NULL;
}
