/* Copyright (C) 1993, 1995, 1996, 1997, 1998 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#ifndef _RES_HCONF_H_
#define _RES_HCONF_H_

#include <netdb.h>

#define TRIMDOMAINS_MAX	4

enum Name_Service
{
  SERVICE_NONE = 0,
  SERVICE_BIND, SERVICE_HOSTS, SERVICE_NIS,
  SERVICE_MAX
};

struct hconf
{
  int initialized;
  int num_services;
  enum Name_Service service[SERVICE_MAX];
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
