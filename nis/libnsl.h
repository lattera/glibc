/* Copyright (C) 2005-2018 Free Software Foundation, Inc.
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

#include <rpcsvc/nis.h>

#define NSS_FLAG_NETID_AUTHORITATIVE	1
#define NSS_FLAG_SERVICES_AUTHORITATIVE	2
#define NSS_FLAG_SETENT_BATCH_READ	4
#define NSS_FLAG_ADJUNCT_AS_SHADOW	8


/* Get current set of default flags.  */
extern int _nsl_default_nss (void);

/* Set up everything for a call to __do_niscall3.  */
extern nis_error __prepare_niscall (const_nis_name name, directory_obj **dirp,
				    dir_binding *bptrp, unsigned int flags);
libnsl_hidden_proto (__prepare_niscall)

extern struct ib_request *__create_ib_request (const_nis_name name,
					       unsigned int flags);
libnsl_hidden_proto (__create_ib_request)

extern nis_error __follow_path (char **tablepath, char **tableptr,
				struct ib_request *ibreq, dir_binding *bptr);
libnsl_hidden_proto (__follow_path)
