/* Common database open/close routines for nss_db.
   Copyright (C) 1999, 2000 Free Software Foundation, Inc.
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

/* Variables which keep track of the error values.  */
extern int db_keyexist;
extern int db_notfound;

/* This flag is the same for all versions of the Berkeley DB library.  */
#define DB_CREATE	0x000001

/* But constants which vary from version to version are actually
   variables here.  */
extern int db_first;
extern int db_next;
extern int db_nooverwrite;
extern int db_truncate;
extern int db_rdonly;

/* The `DBT' type is the same in all versions we support.  */
typedef struct
{
  void *data;
  uint32_t size;
  uint32_t ulen;
  uint32_t dlen;
  uint32_t doff;
  uint32_t flags;
} DBT;

/* But the cursor object is very different from version to version.  */
typedef struct
{
  void *cursor;
  int (*c_get) (void *, DBT *, DBT *, uint32_t);
} NSS_DBC;

/* We need a helper function for it.  */
extern int db_cursor (void *db, void *txn, NSS_DBC **dbcp);

/* This is the wrapper we put around the `DB' structures to provide a
   uniform interface to the higher-level functions.  */
typedef struct
{
  void *db;
  int (*close) (void *, uint32_t);
  int (*cursor) (void *, void *, NSS_DBC **);
  int (*fd) (void *, int *);
  int (*get) (void *, void *, DBT *, DBT *, uint32_t);
  int (*put) (void *, void *, DBT *, DBT *, uint32_t);
} NSS_DB;

/* Open the database stored in FILE.  If succesful, store the database
   handle in *DBP and return NSS_STATUS_SUCCESS.  On failure, return
   the appropriate lookup status.  */
extern enum nss_status internal_setent (const char *file, NSS_DB **dbp);

/* Close the database *DBP.  */
extern void internal_endent (NSS_DB **dbp);

/* Dynamically load the Berkeley DB library.  Return zero if
   successful, non-zero if no suitable version of the library could be
   loaded.  */
extern enum nss_status load_db (void);

/* Open the database in FNAME, for access specified by FLAGS.  If
   opening the database causes the file FNAME to be created, it is
   created with MODE.  If succesful, store the database handle in *DBP
   and return NSS_STATUS_SUCCESS.  On failure, return the appropriate
   lookup status.  */
extern int dbopen (const char *fname, int oper, int mode, NSS_DB **dbp);

#endif	/* nss_db.h */
