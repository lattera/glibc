/* User and group IDs.
   Copyright (C) 1993, 1994, 1997 Free Software Foundation, Inc.
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

#ifndef	_HURD_ID_H

#define	_HURD_ID_H	1
#include <features.h>

#include <cthreads.h>		/* For `struct mutex'.  */

/* Structure describing authorization data for the process.  */

struct hurd_id_data
  {
    struct mutex lock;

    int valid;			/* If following data are up to date.  */

    struct
      {
	uid_t *uids;
	gid_t *gids;
	mach_msg_type_number_t nuids, ngids;
      } gen, aux;

    auth_t rid_auth;		/* Cache used by access.  */
  };

/* Current data.  */

extern struct hurd_id_data _hurd_id;


/* Update _hurd_id (caller should be holding the lock).  */

extern error_t _hurd_check_ids (void);


#endif	/* hurd/id.h */
