/* Copyright (c) 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Thorsten Kukuk <kukuk@vt.uni-paderborn.de>, 1998.

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
   Boston, MA 02111-1307, USA. */

#include <ctype.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "dbg_log.h"
#include "nscd.h"

int
nscd_parse_file (const char *fname)
{
  FILE *fp;
  char *line, *cp, *entry, *arg1, *arg2;
  size_t len;

  /* Open the configuration file.  */
  fp = fopen (fname, "r");
  if (fp == NULL)
    return -1;

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
      cp = strchr (line, '#');
      if (cp != NULL)
	*cp = '\0';

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
	  if (strcmp (arg1, "passwd") == 0)
	    set_pos_pwd_ttl (atol (arg2));
	  else if (strcmp (arg1, "group") == 0)
	    set_pos_grp_ttl (atol (arg2));
	  else
	    dbg_log ("server %s is not supported\n", arg1);
	}
      else if (strcmp (entry, "negative-time-to-live") == 0)
	{
	  if (strcmp (arg1, "passwd") == 0)
	    set_neg_pwd_ttl (atol (arg2));
	  else if (strcmp (arg1, "group") == 0)
	    set_neg_grp_ttl (atol (arg2));
	  else
	    dbg_log (_("service %s is not supported"), arg1);
	}
      else if (strcmp (entry, "suggested-size") == 0)
	{
	  if (strcmp (arg1, "passwd") == 0)
	    set_pwd_modulo (atol (arg2));
	  else if (strcmp (arg1, "group") == 0)
	    set_grp_modulo (atol (arg2));
	  else
	    dbg_log (_("service %s is not supported"), arg1);
	}
      else if (strcmp (entry, "enable-cache") ==0)
	{
	  if (strcmp (arg1, "passwd") == 0
	      && strcmp (arg2, "no") == 0)
	    disabled_passwd = 1;
	  else if (strcmp (arg1, "group") == 0
		   && strcmp (arg2, "no") == 0)
	    disabled_group = 1;
	  else
	    dbg_log (_("service %s is not supported"), arg1);
	}
      else if (strcmp (entry, "logfile") == 0)
	{
	  if (!set_logfile (arg1))
	    dbg_log (_("Could not create log file \"%s\""), arg1);
	}
      else if (strcmp (entry, "debug-level") == 0)
	{
	  int level = atoi (arg1);
	  if (level > 0)
	    debug_flag = level;
	}
      else
	dbg_log (_("Unknown option: %s %s %s"), entry, arg1, arg2);
    }
  while (!feof (fp));

  /* Free the buffer.  */
  free (line);
  /* Close configuration file.  */
  fclose (fp);

  return 0;
}
