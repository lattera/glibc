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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include "nscd.h"

void
print_stat (void)
{
  int sock = __nscd_open_socket ();
  request_header req;
  stat_response_header resp;
  ssize_t nbytes;

  if (sock == -1)
    {
      fputs (_("nscd not running!\n"), stdout);
      exit (EXIT_FAILURE);
    }

  req.version = NSCD_VERSION;
  req.type = GETSTAT;
  req.key_len = 0;
  nbytes = write (sock, &req, sizeof (request_header));
  if (nbytes != sizeof (request_header))
    {
      perror (_("write incomplete"));
      close (sock);
      exit (EXIT_FAILURE);
    }

  nbytes = read (sock, &resp, sizeof (stat_response_header));
  if (nbytes != sizeof (stat_response_header))
    {
      perror (_("read incomplete"));
      close (sock);
      exit (EXIT_FAILURE);
    }

  close (sock);

  printf (_("nscd configuration:\n\n"));
  printf (_("%12d  server debug level\n\n"), resp.debug_level);

  printf (_("passwd cache:\n\n"));
  printf (_("%12s  cache is enabled\n"), resp.pw_enabled ? _("Yes") : _("No"));
  printf (_("%12ld  cache hits on positive entries\n"), resp.pw_poshit);
  printf (_("%12ld  cache hits on negative entries\n"), resp.pw_neghit);
  printf (_("%12ld  cache misses on positive entries\n"), resp.pw_posmiss);
  printf (_("%12ld  cache misses on negative entries\n"), resp.pw_negmiss);
  printf (_("%12ld  suggested size\n"), resp.pw_size);
  printf (_("%12ld  seconds time to live for positive entries\n"),
	  resp.pw_posttl);
  printf (_("%12ld  seconds time to live for negative entries\n\n"),
	  resp.pw_negttl);

  printf (_("group cache:\n\n"));
  printf (_("%12s  cache is enabled\n"), resp.gr_enabled ? _("Yes") : _("No"));
  printf (_("%12ld  cache hits on positive entries\n"), resp.gr_poshit);
  printf (_("%12ld  cache hits on negative entries\n"), resp.gr_neghit);
  printf (_("%12ld  cache misses on positive entries\n"), resp.gr_posmiss);
  printf (_("%12ld  cache misses on negative entries\n"), resp.gr_negmiss);
  printf (_("%12ld  suggested size\n"), resp.gr_size);
  printf (_("%12ld  seconds time to live for positive entries\n"),
	  resp.gr_posttl);
  printf (_("%12ld  seconds time to live for negative entries\n"),
	  resp.gr_negttl);
}
