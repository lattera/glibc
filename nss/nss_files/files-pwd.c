/* User file parser in nss_files module.
Copyright (C) 1996 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <pwd.h>

#define STRUCTURE	passwd
#define ENTNAME		pwent
#define DATAFILE	"/etc/passwd"
struct pwent_data {};

#include "files-parse.c"
LINE_PARSER
({
  STRING_FIELD (result->pw_name, ISCOLON, 0);
  STRING_FIELD (result->pw_passwd, ISCOLON, 0);
  INT_FIELD (result->pw_uid, ISCOLON, 0, 10,);
  INT_FIELD (result->pw_gid, ISCOLON, 0, 10,);
  STRING_FIELD (result->pw_gecos, ISCOLON, 0);
  STRING_FIELD (result->pw_dir, ISCOLON, 0);
  result->pw_shell = line;
  line = strchr (line, '\n');
  if (line)
    *line = '\0';
})

#include "files-XXX.c"

DB_LOOKUP (pwnam,
	   {
	     if (! strcmp (name, result->pw_name))
	       break;
	   }, const char *name)

DB_LOOKUP (pwuid,
	   {
	     if (result->pw_uid == uid)
	       break;
	   }, uid_t uid)
