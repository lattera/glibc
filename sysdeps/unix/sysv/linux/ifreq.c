/* Copyright (C) 1999, 2002, 2003, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Andreas Jaeger <aj@suse.de>.

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

#include "ifreq.h"

/* Variable to signal whether SIOCGIFCONF is not available.  */
#if __ASSUME_SIOCGIFNAME == 0 || 1
static int old_siocgifconf;
#else
# define old_siocgifconf 0
#endif


void
__ifreq (struct ifreq **ifreqs, int *num_ifs, int sockfd)
{
  int fd = sockfd;
  struct ifconf ifc;
  int rq_len;
  int nifs;
# define RQ_IFS	4

  if (fd < 0)
    fd = __opensock ();
  if (fd < 0)
    {
      *num_ifs = 0;
      *ifreqs = NULL;
      return;
    }

  ifc.ifc_buf = NULL;

  /* We may be able to get the needed buffer size directly, rather than
     guessing.  */
  if (! old_siocgifconf)
    {
      ifc.ifc_buf = NULL;
      ifc.ifc_len = 0;
      if (__ioctl (fd, SIOCGIFCONF, &ifc) < 0 || ifc.ifc_len == 0)
	{
# if __ASSUME_SIOCGIFNAME == 0
	  old_siocgifconf = 1;
# endif
	  rq_len = RQ_IFS * sizeof (struct ifreq);
	}
      else
	rq_len = ifc.ifc_len;
    }
  else
    rq_len = RQ_IFS * sizeof (struct ifreq);

  /* Read all the interfaces out of the kernel.  */
  while (1)
    {
      ifc.ifc_len = rq_len;
      void *newp = realloc (ifc.ifc_buf, ifc.ifc_len);
      if (newp == NULL
	  || (ifc.ifc_buf = newp, __ioctl (fd, SIOCGIFCONF, &ifc)) < 0)
	{
	  free (ifc.ifc_buf);

	  if (fd != sockfd)
	    __close (fd);

	  *num_ifs = 0;
	  *ifreqs = NULL;
	  return;
	}

      if (!old_siocgifconf || ifc.ifc_len < rq_len)
	break;

      rq_len *= 2;
    }

  nifs = ifc.ifc_len / sizeof (struct ifreq);

  if (fd != sockfd)
    __close (fd);

  *num_ifs = nifs;
  *ifreqs = realloc (ifc.ifc_buf, nifs * sizeof (struct ifreq));
}
