/* Copyright (C) 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2004.

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
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <net/if.h>


static int
do_test (void)
{
  /* Get list of all interfaces.  */
  struct if_nameindex *il = if_nameindex ();
  if (il == NULL)
    {
      puts ("cannot get interface list, maybe the system does not support networking; bailing out");
      return 0;
    }

  /* Determine the highest interface number.  */
  unsigned int max = 0;
  for (int cnt = 0; il[cnt].if_name != NULL; ++cnt)
    if (il[cnt].if_index > max)
      max = il[cnt].if_index;

  /* Use the next higher value (if possible).  */
  if (max == UINT_MAX)
    {
      puts ("highest index too high; need more clever way to determine test index");
      return 0;
    }

  char buf[IF_NAMESIZE];
  char *cp = if_indextoname (max + 1, buf);
  if (cp != NULL)
    {
      printf ("invalid index returned result \"%s\"\n", cp);
      return 1;
    }
  else if (errno != ENXIO)
    {
      int err = errno;
      char errbuf1[256];
      char errbuf2[256];

      printf ("errno = %d (%s), expected %d (%s)\n",
	      err, strerror_r (err, errbuf1, sizeof (errbuf1)),
	      ENXIO, strerror_r (ENXIO, errbuf2, sizeof (errbuf2)));
      return 1;
    }

  return 0;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
