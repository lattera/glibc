/* Common database routines for nss_db.
   Copyright (C) 2000, 2011 Free Software Foundation, Inc.
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

#include <errno.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <not-cancel.h>

#include <kernel-features.h>
#include "nss_db.h"

/* Open the database stored in FILE.  If succesful, store either a
   pointer to the mapped file or a file handle for the file in H and
   return NSS_STATUS_SUCCESS.  On failure, return the appropriate
   lookup status.  */
enum nss_status
internal_setent (const char *file, struct nss_db_map *mapping)
{
  enum nss_status status = NSS_STATUS_UNAVAIL;

  int mode = O_RDONLY | O_LARGEFILE;
#ifdef O_CLOEXEC
  mode |= O_CLOEXEC;
#endif
  int fd = open_not_cancel_2 (file, mode);
  if (fd != -1)
    {
      struct nss_db_header header;

      if (read (fd, &header, sizeof (header)) == sizeof (header))
	{
	  mapping->header = mmap (NULL, header.allocate, PROT_READ,
				  MAP_PRIVATE, fd, 0);
	  mapping->len = header.allocate;
	  if (mapping->header != MAP_FAILED)
	    status = NSS_STATUS_SUCCESS;
	  else if (errno == ENOMEM)
	    status = NSS_STATUS_TRYAGAIN;
	}

      close_not_cancel_no_status (fd);
    }

  return status;
}


/* Close the database.  */
void
internal_endent (struct nss_db_map *mapping)
{
  munmap (mapping->header, mapping->len);
}
