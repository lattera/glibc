/* Common database open/close routines for nss_db.
   Copyright (C) 1999, 2000 Free Software Foundation, Inc.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#ifndef _NSS_DB_H
#define _NSS_DB_H	1

#include <stdint.h>

/* The error values kept the same values though new values were added.
   Define only those which we need.  */
#define DB_KEYEXIST	( -3)
#define DB_NOTFOUND	( -7)

/* Flags are also unchanged.  */
#define DB_CREATE	0x000001


/* Similarly we have to handle the cursor object.  It is also very
   different from version to version.  */
typedef struct
{
  void *cursor;
  int (*c_get) (void *, void *, void *, uint32_t);
} NSS_DBC;


/* This is the wrapper we put around the `DB' structures to provide a
   uniform interface to the higher-level functions.  */
typedef struct
{
  void *db;
  int (*close) (void *, uint32_t);
  int (*cursor) (void *, void *, NSS_DBC **);
  int (*fd) (void *, int *);
  int (*get) (void *, void *, void *, void *, uint32_t);
  int (*put) (void *, void *, void *, void *, uint32_t);
} NSS_DB;


/* The `DBT' type is the same in all versions we support.  */
typedef struct {
  void *data;
  uint32_t size;
  uint32_t ulen;
  uint32_t dlen;
  uint32_t doff;
  uint32_t flags;
} DBT;


/* Private routines to nss_db.
   You must have included nsswitch.h and db.h before this file.  */

extern enum nss_status internal_setent (const char *file, NSS_DB **dbp);
extern void internal_endent (NSS_DB **dbp);

#endif	/* nss_db.h */
