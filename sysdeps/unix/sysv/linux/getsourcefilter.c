/* Get source filter.  Linux version.
   Copyright (C) 2004 Free Software Foundation, Inc.
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

#include <alloca.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>


int
getsourcefilter (int s, uint32_t interface, struct sockaddr *group,
		 socklen_t grouplen, uint32_t *fmode, uint32_t *numsrc,
		 struct sockaddr_storage *slist)
{
  /* We have to create an struct ip_msfilter object which we can pass
     to the kernel.  */
  socklen_t needed = GROUP_FILTER_SIZE (*numsrc);
  int use_malloc = __libc_use_alloca (needed);

  struct group_filter *gf;
  if (use_malloc)
    {
      gf = (struct group_filter *) malloc (needed);
      if (gf == NULL)
	return -1;
    }
  else
    gf = (struct group_filter *) alloca (needed);

  gf->gf_interface = interface;
  memcpy (&gf->gf_group, group, grouplen);

  int result = __getsockopt (s, SOL_IP, MCAST_MSFILTER, gf, &needed);

  /* If successful, copy the results to the places the caller wants
     them in.  */
  if (result == 0)
    {
      *fmode = gf->gf_fmode;
      *numsrc = gf->gf_numsrc;
      memcpy (slist, gf->gf_slist,
	      gf->gf_numsrc * sizeof (struct sockaddr_storage));
    }

  if (use_malloc)
    {
      int save_errno = errno;
      free (gf);
      __set_errno (save_errno);
    }

  return result;
}
