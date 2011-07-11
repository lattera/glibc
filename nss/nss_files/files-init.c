/* Initialization in nss_files module.
   Copyright (C) 2011 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <nscd/nscd.h>


static union
{
  struct traced_file file;
  char buf[sizeof (struct traced_file) + sizeof ("/etc/passwd")];
} pwd_traced_file;

static union
{
  struct traced_file file;
  char buf[sizeof (struct traced_file) + sizeof ("/etc/group")];
} grp_traced_file;

static union
{
  struct traced_file file;
  char buf[sizeof (struct traced_file) + sizeof ("/etc/hosts")];
} hst_traced_file;

static union
{
  struct traced_file file;
  char buf[sizeof (struct traced_file) + sizeof ("/etc/resolv.conf")];
} resolv_traced_file;

static union
{
  struct traced_file file;
  char buf[sizeof (struct traced_file) + sizeof ("/etc/services")];
} serv_traced_file;


void
_nss_files_init (void (*cb) (size_t, struct traced_file *))
{
  strcpy (pwd_traced_file.file.fname, "/etc/passwd");
  cb (pwddb, &pwd_traced_file.file);

  strcpy (grp_traced_file.file.fname, "/etc/group");
  cb (grpdb, &grp_traced_file.file);

  strcpy (hst_traced_file.file.fname, "/etc/hosts");
  cb (hstdb, &hst_traced_file.file);

  resolv_traced_file.file.call_res_init = 1;
  strcpy (resolv_traced_file.file.fname, "/etc/resolv.conf");
  cb (hstdb, &resolv_traced_file.file);

  strcpy (serv_traced_file.file.fname, "/etc/services");
  cb (servdb, &serv_traced_file.file);
}
