/* Copyright (C) 1998, 1999, 2000, 2001 Free Software Foundation, Inc.
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

#include <errno.h>
#include <fcntl.h>
#include <nss.h>
#include <grp.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <rpcsvc/yp.h>
#include <rpcsvc/ypclnt.h>
#include <rpcsvc/nis.h>
#include <sys/param.h>
#include <nsswitch.h>

#include "nss-nis.h"
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

struct response_t
{
  char *val;
  struct response_t *next;
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
    struct response_t *start;
    struct response_t *next;
};
typedef struct ent_t ent_t;


/* Prototypes for local functions.  */
static void blacklist_store_name (const char *, ent_t *);
static int in_blacklist (const char *, int, ent_t *);

static int
saveit (int instatus, char *inkey, int inkeylen, char *inval,
        int invallen, char *indata)
{
  ent_t *intern = (ent_t *) indata;

  if (instatus != YP_TRUE)
    return instatus;

  if (inkey && inkeylen > 0 && inval && invallen > 0)
    {
      if (intern->start == NULL)
        {
          intern->start = malloc (sizeof (struct response_t));
	  if (intern->start == NULL)
	    return YP_FALSE;
          intern->next = intern->start;
        }
      else
        {
          intern->next->next = malloc (sizeof (struct response_t));
	  if (intern->next->next == NULL)
	    return YP_FALSE;
          intern->next = intern->next->next;
        }
      intern->next->next = NULL;
      intern->next->val = malloc (invallen + 1);
      if (intern->next->val == NULL)
	return YP_FALSE;
      strncpy (intern->next->val, inval, invallen);
      intern->next->val[invallen] = '\0';
    }

  return 0;
}

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

  ent->start = NULL;
  ent->next = NULL;

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

  while (ent->start != NULL)
    {
      if (ent->start->val != NULL)
        free (ent->start->val);
      ent->next = ent->start;
      ent->start = ent->start->next;
      free (ent->next);
    }


  return NSS_STATUS_SUCCESS;
}

static enum nss_status
getgrent_next_nis (struct group *result, ent_t *ent, char *buffer,
		   size_t buflen, int *errnop)
{
  struct parser_data *data = (void *) buffer;
  char *domain, *p;
  int parse_res;

  if (yp_get_default_domain (&domain) != YPERR_SUCCESS)
    {
      ent->nis = 0;
      *errnop = ENOENT;
      return NSS_STATUS_NOTFOUND;
    }

  if (ent->start == NULL)
    {
      struct ypall_callback ypcb;
      enum nss_status status;

      ypcb.foreach = saveit;
      ypcb.data = (char *) ent;
      status = yperr2nss (yp_all (domain, "group.byname", &ypcb));
      ent->next = ent->start;

      if (ent->start == NULL || status != NSS_STATUS_SUCCESS)
	{
	  ent->nis = 0;
	  *errnop = ENOENT;
	  return NSS_STATUS_UNAVAIL;
	}
    }


  do
    {
      if (ent->next == NULL)
        {
          *errnop = ENOENT;
	  ent->nis = 0;
          return NSS_STATUS_NOTFOUND;
        }

      /* Copy the found data to our buffer...  */
      p = strncpy (buffer, ent->next->val, buflen);
      while (isspace (*p))
        ++p;

      parse_res = _nss_files_parse_grent (p, result, data, buflen, errnop);
      if (parse_res == -1)
	{
	  *errnop = ERANGE;
	  return NSS_STATUS_TRYAGAIN;
	}

      ent->next = ent->next->next;

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
	{
	  *errnop = ERANGE;
	  return NSS_STATUS_TRYAGAIN;
	}
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
_nss_compat_initgroups_dyn (const char *user, gid_t group, long int *start,
			    long int *size, gid_t **groupsp, long int limit,
			    int *errnop)
{
  struct group grpbuf, *g;
  size_t buflen = sysconf (_SC_GETPW_R_SIZE_MAX);
  char *tmpbuf;
  enum nss_status status;
  ent_t intern = {0, 0, NULL, 0, NULL, NULL, {NULL, 0, 0}};
  gid_t *groups = *groupsp;

  status = internal_setgrent (&intern);
  if (status != NSS_STATUS_SUCCESS)
    return status;

  tmpbuf = __alloca (buflen);

  do
    {
      while ((status =
              internal_getgrent_r (&grpbuf, &intern, tmpbuf, buflen,
				   errnop)) == NSS_STATUS_TRYAGAIN
             && *errnop == ERANGE)
        {
          buflen *= 2;
          tmpbuf = __alloca (buflen);
        }

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
