/* Copyright (c) 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Thorsten Kukuk <kukuk@vt.uni-paderborn.de>, 1998.

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
#include <error.h>
#include <langinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libintl.h>

#include "nscd.h"
#include "dbg_log.h"

/* We use this to make sure the receiver is the same.  */
static const char compilation[21] = __DATE__ " " __TIME__;

/* Statistic data for one database.  */
struct dbstat
{
  int enabled;
  int check_file;
  size_t module;

  unsigned long int postimeout;
  unsigned long int negtimeout;

  unsigned long int poshit;
  unsigned long int neghit;
  unsigned long int posmiss;
  unsigned long int negmiss;
};

/* Record for transmitting statistics.  */
struct statdata
{
  char version[sizeof (compilation)];
  int debug_level;
  int ndbs;
  struct dbstat dbs[lastdb];
};


void
send_stats (int fd, struct database dbs[lastdb])
{
  struct statdata data;
  int cnt;

  memcpy (data.version, compilation, sizeof (compilation));
  data.debug_level = debug_level;
  data.ndbs = lastdb;

  for (cnt = 0; cnt < lastdb; ++cnt)
    {
      data.dbs[cnt].enabled = dbs[cnt].enabled;
      data.dbs[cnt].check_file = dbs[cnt].check_file;
      data.dbs[cnt].module = dbs[cnt].module;
      data.dbs[cnt].postimeout = dbs[cnt].postimeout;
      data.dbs[cnt].negtimeout = dbs[cnt].negtimeout;
      data.dbs[cnt].poshit = dbs[cnt].poshit;
      data.dbs[cnt].neghit = dbs[cnt].neghit;
      data.dbs[cnt].posmiss = dbs[cnt].posmiss;
      data.dbs[cnt].negmiss = dbs[cnt].negmiss;
    }

  if (TEMP_FAILURE_RETRY (write (fd, &data, sizeof (data))) != sizeof (data))
    {
      char buf[256];
      dbg_log (_("cannot write statistics: %s"),
	       strerror_r (errno, buf, sizeof (buf)));
    }
}


int
receive_print_stats (void)
{
  struct statdata data;
  request_header req;
  ssize_t nbytes;
  int fd;
  int i;

  /* Open a socket to the running nscd.  */
  fd = nscd_open_socket ();
  if (fd == -1)
    error (EXIT_FAILURE, 0, _("nscd not running!\n"));

  /* Send the request.  */
  req.version = NSCD_VERSION;
  req.type = GETSTAT;
  req.key_len = 0;
  nbytes = TEMP_FAILURE_RETRY (write (fd, &req, sizeof (request_header)));
  if (nbytes != sizeof (request_header))
    {
      int err = errno;
      close (fd);
      error (EXIT_FAILURE, err, _("write incomplete"));
    }

  /* Read as much data as we expect.  */
  if (TEMP_FAILURE_RETRY (read (fd, &data, sizeof (data))) != sizeof (data)
      || (memcmp (data.version, compilation, sizeof (compilation)) != 0
	  /* Yes, this is an assignment!  */
	  && errno == EINVAL))
    {
      /* Not the right version.  */
      int err = errno;
      close (fd);
      error (EXIT_FAILURE, err, _("cannot read statistics data"));
    }

  printf (_("nscd configuration:\n\n%15d  server debug level\n"),
	  data.debug_level);

  for (i = 0; i < lastdb; ++i)
    {
      unsigned long int hit = data.dbs[i].poshit + data.dbs[i].neghit;
      unsigned long int all = hit + data.dbs[i].posmiss + data.dbs[i].negmiss;
      const char *enabled = nl_langinfo (data.dbs[i].enabled ? YESSTR : NOSTR);
      const char *check_file = nl_langinfo (data.dbs[i].check_file
					    ? YESSTR : NOSTR);

      if (enabled[0] == '\0')
	/* The locale does not provide this information so we have to
	   translate it ourself.  Since we should avoid short translation
	   terms we artifically increase the length.  */
	enabled = data.dbs[i].enabled ? _("     yes") : _("      no");
      if (check_file[0] == '\0')
	check_file = data.dbs[i].check_file ? _("     yes") : _("      no");

      if (all == 0)
	/* If nothing happened so far report a 0% hit rate.  */
	all = 1;

      printf (_("\n%s cache:\n\n"
		"%15s  cache is enabled\n"
		"%15Zd  suggested size\n"
		"%15ld  seconds time to live for positive entries\n"
		"%15ld  seconds time to live for negative entries\n"
		"%15ld  cache hits on positive entries\n"
		"%15ld  cache hits on negative entries\n"
		"%15ld  cache misses on positive entries\n"
		"%15ld  cache misses on negative entries\n"
		"%15ld%% cache hit rate\n"
		"%15s  check /etc/%s for changes\n"),
	      dbnames[i], enabled,
	      data.dbs[i].module,
	      data.dbs[i].postimeout, data.dbs[i].negtimeout,
	      data.dbs[i].poshit, data.dbs[i].neghit,
	      data.dbs[i].posmiss, data.dbs[i].negmiss,
	      (100 * hit) / all,
	      check_file, dbnames[i]);
    }

  close (fd);

  exit (0);
}
