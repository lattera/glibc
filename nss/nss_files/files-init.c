/* Initialization in nss_files module.
   Copyright (C) 2011-2018 Free Software Foundation, Inc.
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

#ifdef USE_NSCD

#include <string.h>
#include <nscd/nscd.h>

#define PWD_FILENAME "/etc/passwd"
define_traced_file (pwd, PWD_FILENAME);

#define GRP_FILENAME "/etc/group"
define_traced_file (grp, GRP_FILENAME);

#define HST_FILENAME "/etc/hosts"
define_traced_file (hst, HST_FILENAME);

#define RESOLV_FILENAME "/etc/resolv.conf"
define_traced_file (resolv, RESOLV_FILENAME);

#define SERV_FILENAME "/etc/services"
define_traced_file (serv, SERV_FILENAME);

#define NETGR_FILENAME "/etc/netgroup"
define_traced_file (netgr, NETGR_FILENAME);

void
_nss_files_init (void (*cb) (size_t, struct traced_file *))
{
  init_traced_file (&pwd_traced_file.file, PWD_FILENAME, 0);
  cb (pwddb, &pwd_traced_file.file);

  init_traced_file (&grp_traced_file.file, GRP_FILENAME, 0);
  cb (grpdb, &grp_traced_file.file);

  init_traced_file (&hst_traced_file.file, HST_FILENAME, 0);
  cb (hstdb, &hst_traced_file.file);

  init_traced_file (&resolv_traced_file.file, RESOLV_FILENAME, 1);
  cb (hstdb, &resolv_traced_file.file);

  init_traced_file (&serv_traced_file.file, SERV_FILENAME, 0);
  cb (servdb, &serv_traced_file.file);

  init_traced_file (&netgr_traced_file.file, NETGR_FILENAME, 0);
  cb (netgrdb, &netgr_traced_file.file);
}

#endif
