/* Copyright (c) 1997, 1998, 1999 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Thorsten Kukuk <kukuk@suse.de>, 1997.

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

#include <string.h>
#include <rpcsvc/nis.h>

#include "nis_xdr.h"
#include "nis_intern.h"

fd_result *
__nis_finddirectory (directory_obj *dir, const_nis_name name)
{
  nis_error status;
  fd_args fd_args;
  fd_result *fd_res;

  fd_args.dir_name = (char *)name;
  fd_args.requester = nis_local_host();
  fd_res = calloc (1, sizeof (fd_result));
  if (fd_res == NULL)
    return NULL;

  status = __do_niscall2 (dir->do_servers.do_servers_val,
			  dir->do_servers.do_servers_len,
			  NIS_FINDDIRECTORY, (xdrproc_t) _xdr_fd_args,
			  (caddr_t) &fd_args, (xdrproc_t) _xdr_fd_result,
			  (caddr_t) fd_res, NO_AUTHINFO|USE_DGRAM, NULL);
  if (status != NIS_SUCCESS)
    fd_res->status = status;

  return fd_res;
}

/* This is from libc/db/hash/hash_func.c, hash3 is static there */
/*
 * This is INCREDIBLY ugly, but fast.  We break the string up into 8 byte
 * units.  On the first time through the loop we get the "leftover bytes"
 * (strlen % 8).  On every other iteration, we perform 8 HASHC's so we handle
 * all 8 bytes.  Essentially, this saves us 7 cmp & branch instructions.  If
 * this routine is heavily used enough, it's worth the ugly coding.
 *
 * OZ's original sdbm hash
 */
uint32_t
__nis_hash (const void *keyarg, register size_t len)
{
  register const u_char *key;
  register size_t loop;
  register uint32_t h;

#define HASHC   h = *key++ + 65599 * h

  h = 0;
  key = keyarg;
  if (len > 0)
    {
      loop = (len + 8 - 1) >> 3;
      switch (len & (8 - 1))
	{
	case 0:
	  do {
	    HASHC;
	    /* FALLTHROUGH */
	  case 7:
	    HASHC;
	    /* FALLTHROUGH */
	  case 6:
	    HASHC;
	    /* FALLTHROUGH */
	  case 5:
	    HASHC;
	    /* FALLTHROUGH */
	  case 4:
	    HASHC;
	    /* FALLTHROUGH */
	  case 3:
	    HASHC;
	    /* FALLTHROUGH */
	  case 2:
	    HASHC;
	    /* FALLTHROUGH */
	  case 1:
	    HASHC;
	  } while (--loop);
	}
    }
  return (h);
}
