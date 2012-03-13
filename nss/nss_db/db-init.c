/* Initialization in nss_db module.
   Copyright (C) 2011, 2012 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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

#include <paths.h>
#include <nscd/nscd.h>
#include <string.h>

static union
{
  struct traced_file file;
  char buf[sizeof (struct traced_file) + sizeof (_PATH_VARDB "passwd.db")];
} pwd_traced_file;

static union
{
  struct traced_file file;
  char buf[sizeof (struct traced_file) + sizeof (_PATH_VARDB "group.db")];
} grp_traced_file;

static union
{
  struct traced_file file;
  char buf[sizeof (struct traced_file) + sizeof (_PATH_VARDB "services.db")];
} serv_traced_file;


void
_nss_db_init (void (*cb) (size_t, struct traced_file *))
{
  strcpy (pwd_traced_file.file.fname,_PATH_VARDB  "passwd.db");
  cb (pwddb, &pwd_traced_file.file);

  strcpy (grp_traced_file.file.fname, _PATH_VARDB "group.db");
  cb (grpdb, &grp_traced_file.file);

  strcpy (serv_traced_file.file.fname, _PATH_VARDB "services.db");
  cb (servdb, &serv_traced_file.file);
}
