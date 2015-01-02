/* Copyright (C) 1999-2015 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

static inline struct ifreq *
__if_nextreq (struct ifreq *ifr)
{
#ifdef _HAVE_SA_LEN
  if (ifr->ifr_addr.sa_len > sizeof ifr->ifr_addr)
    return (struct ifreq *) ((char *) &ifr->ifr_addr + ifr->ifr_addr.sa_len);
#endif
  return ifr + 1;
}

extern void __ifreq (struct ifreq **ifreqs, int *num_ifs, int sockfd);


static inline void
__if_freereq (struct ifreq *ifreqs, int num_ifs)
{
  munmap (ifreqs, num_ifs * sizeof (struct ifreq));
}
