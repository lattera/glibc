/* Copyright (c) 1998, 2000, 2003, 2004 Free Software Foundation, Inc.
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

#include <ctype.h>
#include <errno.h>
#include <libintl.h>
#include <malloc.h>
#include <pwd.h>
#include <stdio.h>
#include <stdio_ext.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/types.h>

#include "dbg_log.h"
#include "nscd.h"

/* Wrapper functions with error checking for standard functions.  */
extern char *xstrdup (const char *s);


/* Names of the databases.  */
const char *dbnames[lastdb] =
{
  [pwddb] = "passwd",
  [grpdb] = "group",
  [hstdb] = "hosts"
};

int
nscd_parse_file (const char *fname, struct database_dyn dbs[lastdb])
{
  FILE *fp;
  char *line, *cp, *entry, *arg1, *arg2;
  size_t len;
  int cnt;

  /* Open the configuration file.  */
  fp = fopen (fname, "r");
  if (fp == NULL)
    return -1;

  /* The stream is not used by more than one thread.  */
  (void) __fsetlocking (fp, FSETLOCKING_BYCALLER);

  line = NULL;
  len = 0;

  do
    {
      ssize_t n = getline (&line, &len, fp);
      if (n < 0)
	break;
      if (line[n - 1] == '\n')
	line[n - 1] = '\0';

      /* Because the file format does not know any form of quoting we
	 can search forward for the next '#' character and if found
	 make it terminating the line.  */
      *strchrnul (line, '#') = '\0';

      /* If the line is blank it is ignored.  */
      if (line[0] == '\0')
	continue;

      entry = line;
      while (isspace (*entry) && *entry != '\0')
	++entry;
      cp = entry;
      while (!isspace (*cp) && *cp != '\0')
	++cp;
      arg1 = cp;
      ++arg1;
      *cp = '\0';
      if (strlen (entry) == 0)
	dbg_log (_("Parse error: %s"), line);
      while (isspace (*arg1) && *arg1 != '\0')
	++arg1;
      cp = arg1;
      while (!isspace (*cp) && *cp != '\0')
	++cp;
      arg2 = cp;
      ++arg2;
      *cp = '\0';
      if (strlen (arg2) > 0)
	{
	  while (isspace (*arg2) && *arg2 != '\0')
	    ++arg2;
	  cp = arg2;
	  while (!isspace (*cp) && *cp != '\0')
	    ++cp;
	  *cp = '\0';
	}

      if (strcmp (entry, "positive-time-to-live") == 0)
	{
	  for (cnt = 0; cnt < lastdb; ++cnt)
	    if (strcmp (arg1, dbnames[cnt]) == 0)
	      {
		dbs[cnt].postimeout = atol (arg2);
		break;
	      }
	  if (cnt == lastdb)
	    dbg_log ("database %s is not supported\n", arg1);
	}
      else if (strcmp (entry, "negative-time-to-live") == 0)
	{
	  for (cnt = 0; cnt < lastdb; ++cnt)
	    if (strcmp (arg1, dbnames[cnt]) == 0)
	      {
		dbs[cnt].negtimeout = atol (arg2);
		break;
	      }
	  if (cnt == lastdb)
	    dbg_log ("database %s is not supported\n", arg1);
	}
      else if (strcmp (entry, "suggested-size") == 0)
	{
	  for (cnt = 0; cnt < lastdb; ++cnt)
	    if (strcmp (arg1, dbnames[cnt]) == 0)
	      {
		dbs[cnt].suggested_module = atol (arg2);
		break;
	      }
	  if (cnt == lastdb)
	    dbg_log ("database %s is not supported\n", arg1);
	}
      else if (strcmp (entry, "enable-cache") == 0)
	{
	  for (cnt = 0; cnt < lastdb; ++cnt)
	    if (strcmp (arg1, dbnames[cnt]) == 0)
	      {
		if (strcmp (arg2, "no") == 0)
		  dbs[cnt].enabled = 0;
		else if (strcmp (arg2, "yes") == 0)
		  dbs[cnt].enabled = 1;
		break;
	      }
	  if (cnt == lastdb)
	    dbg_log ("database %s is not supported\n", arg1);
	}
      else if (strcmp (entry, "check-files") == 0)
	{
	  for (cnt = 0; cnt < lastdb; ++cnt)
	    if (strcmp (arg1, dbnames[cnt]) == 0)
	      {
		if (strcmp (arg2, "no") == 0)
		  dbs[cnt].check_file = 0;
		else if (strcmp (arg2, "yes") == 0)
		  dbs[cnt].check_file = 1;
		break;
	      }
	  if (cnt == lastdb)
	    dbg_log ("database %s is not supported\n", arg1);
	}
      else if (strcmp (entry, "logfile") == 0)
	set_logfile (arg1);
      else if (strcmp (entry, "debug-level") == 0)
	{
	  int level = atoi (arg1);
	  if (level > 0)
	    debug_level = level;
	}
      else if (strcmp (entry, "threads") == 0)
	{
	  if (nthreads == -1)
	    nthreads = MAX (atol (arg1), lastdb);
	}
      else if (strcmp (entry, "max-threads") == 0)
	{
	  max_nthreads = MAX (atol (arg1), lastdb);
	}
      else if (strcmp (entry, "server-user") == 0)
        {
          if (!arg1)
            dbg_log (_("Must specify user name for server-user option"));
          else
            server_user = xstrdup (arg1);
        }
      else if (strcmp (entry, "stat-user") == 0)
        {
          if (arg1 == NULL)
            dbg_log (_("Must specify user name for stat-user option"));
          else
	    {
	      stat_user = xstrdup (arg1);

	      struct passwd *pw = getpwnam (stat_user);
	      if (pw != NULL)
		stat_uid = pw->pw_uid;
	    }
        }
      else if (strcmp (entry, "persistent") == 0)
	{
	  for (cnt = 0; cnt < lastdb; ++cnt)
	    if (strcmp (arg1, dbnames[cnt]) == 0)
	      {
		if (strcmp (arg2, "no") == 0)
		  dbs[cnt].persistent = 0;
		else if (strcmp (arg2, "yes") == 0)
		  dbs[cnt].persistent = 1;
		break;
	      }
	  if (cnt == lastdb)
	    dbg_log ("database %s is not supported\n", arg1);
	}
      else if (strcmp (entry, "shared") == 0)
	{
	  for (cnt = 0; cnt < lastdb; ++cnt)
	    if (strcmp (arg1, dbnames[cnt]) == 0)
	      {
		if (strcmp (arg2, "no") == 0)
		  dbs[cnt].shared = 0;
		else if (strcmp (arg2, "yes") == 0)
		  dbs[cnt].shared = 1;
		break;
	      }
	  if (cnt == lastdb)
	    dbg_log ("database %s is not supported\n", arg1);
	}
      else if (strcmp (entry, "reload-count") == 0)
	{
	  if (strcasecmp (arg1, "unlimited") == 0)
	    reload_count = UINT_MAX;
	  else
	    {
	      unsigned int count = strtoul (arg1, NULL, 0);
	      if (count > UINT8_MAX - 1)
		reload_count = UINT_MAX;
	      else if (count >= 0)
	    reload_count = count;
	      else
		dbg_log (_("invalid value for 'reload-count': %u"), count);
	    }
	}
      else if (strcmp (entry, "paranoia") == 0)
	{
	  if (strcmp (arg1, "no") == 0)
	    paranoia = 0;
	  else if (strcmp (arg1, "yes") == 0)
	    paranoia = 1;
	}
      else if (strcmp (entry, "restart-interval") == 0)
	{
	  if (arg1 != NULL)
	    restart_interval = atol (arg1);
	  else
            dbg_log (_("Must specify value for restart-interval option"));
	}
      else
	dbg_log (_("Unknown option: %s %s %s"), entry, arg1, arg2);
    }
  while (!feof_unlocked (fp));

  if (paranoia)
    {
      restart_time = time (NULL) + restart_interval;

      /* Save the old current workding directory if we are in paranoia
	 mode.  We have to change back to it.  */
      oldcwd = get_current_dir_name ();
      if (oldcwd == NULL)
	{
	  dbg_log (_("\
cannot get current working directory: %s; disabling paranoia mode"),
		   strerror (errno));
	  paranoia = 0;
	}
    }

  /* Enforce sanity.  */
  if (max_nthreads < nthreads)
    max_nthreads = nthreads;

  /* Free the buffer.  */
  free (line);
  /* Close configuration file.  */
  fclose (fp);

  return 0;
}
