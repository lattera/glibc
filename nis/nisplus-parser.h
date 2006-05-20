/* Copyright (C) 1997, 2004, 2006 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Thorsten Kukuk <kukuk@vt.uni-paderborn.de>, 1997.

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

#ifndef __NISPLUS_PARSER_H
#define __NISPLUS_PARSER_H	1

#include <pwd.h>
#include <grp.h>
#include <shadow.h>

extern int _nss_nisplus_parse_pwent (nis_result *result, struct passwd *pw,
				     char *buffer, size_t buflen, int *errnop);

extern int _nss_nisplus_parse_grent (nis_result *result, struct group *gr,
				     char *buffer, size_t buflen, int *errnop);

extern int _nss_nisplus_parse_spent (nis_result *result, struct spwd *sp,
				     char *buffer, size_t buflen, int *errnop);

#endif
