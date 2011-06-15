/* Common database open/close routines for nss_db.
   Copyright (C) 1999, 2000, 2011 Free Software Foundation, Inc.
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

#ifndef _NSS_DB_H
#define _NSS_DB_H	1

#include <nss.h>
#include <stdint.h>
#include <bits/libc-lock.h>


/* String table index type.  */
typedef uint32_t stridx_t;

/* Database file header.  */
struct nss_db_header
{
  uint32_t magic;
#define NSS_DB_MAGIC 0xdd110601
  uint32_t ndbs;
  uint64_t valstroffset;
  uint64_t valstrlen;
  uint64_t allocate;
  struct
  {
    char id;
    char pad[sizeof (uint32_t) - 1];
    uint32_t hashsize;
    uint64_t hashoffset;
    uint64_t keyidxoffset;
    uint64_t keystroffset;
  } dbs[0];
};


/* Information about mapped database.  */
struct nss_db_map
{
  struct nss_db_header *header;
  size_t len;
};


/* Open the database stored in FILE.  If succesful, store the database
   handle in *MAPPINGP or a file descriptor for the file in *FDP and
   return NSS_STATUS_SUCCESS.  On failure, return the appropriate
   lookup status.  */
enum nss_status internal_setent (const char *file,
				 struct nss_db_map *mappingp);

/* Close the database FD.  */
extern void internal_endent (struct nss_db_map *mapping);

#endif	/* nss_db.h */
