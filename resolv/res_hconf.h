/* Copyright (C) 1993-2015 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by David Mosberger (davidm@azstarnet.com).

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

#ifndef _RES_HCONF_H_
#define _RES_HCONF_H_

#include <netdb.h>

#define TRIMDOMAINS_MAX	4

struct hconf
{
  int initialized;
  int unused1;
  int unused2[4];
  int num_trimdomains;
  const char *trimdomain[TRIMDOMAINS_MAX];
  unsigned int flags;
#  define HCONF_FLAG_INITED	(1 << 0) /* initialized? */
#  define HCONF_FLAG_SPOOF	(1 << 1) /* refuse spoofed addresses */
#  define HCONF_FLAG_SPOOFALERT	(1 << 2) /* syslog warning of spoofed */
#  define HCONF_FLAG_REORDER	(1 << 3) /* list best address first */
#  define HCONF_FLAG_MULTI	(1 << 4) /* see comments for gethtbyname() */
};
extern struct hconf _res_hconf;

extern void _res_hconf_init (void);
extern void _res_hconf_trim_domain (char *domain);
extern void _res_hconf_trim_domains (struct hostent *hp);
extern void _res_hconf_reorder_addrs (struct hostent *hp);

#endif /* _RES_HCONF_H_ */
