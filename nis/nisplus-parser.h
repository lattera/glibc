/* Copyright (C) 1997, 2004 Free Software Foundation, Inc.
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

extern int _nss_nisplus_parse_pwent (nis_result *, struct passwd *,
				     char *, size_t, int *);
extern int _nss_nisplus_parse_grent (nis_result *, u_long, struct group *,
				     char *, size_t, int *);
extern int _nss_nisplus_parse_spent (nis_result *, struct spwd *,
				     char *, size_t, int *);

libnss_nisplus_hidden_proto (_nss_nisplus_parse_pwent)
libnss_nisplus_hidden_proto (_nss_nisplus_parse_grent)
libnss_nisplus_hidden_proto (_nss_nisplus_parse_spent)

#endif
