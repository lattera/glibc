/* Copyright (C) 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Mark Kettenis <kettenis@phys.uva.nl>, 1997.

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

#ifndef _UTMPD_PRIVATE_H
#define _UTMPD_PRIVATE_H	1

#include <time.h>
#include <utmp.h>


/* The number of connections we allow.  */
#ifndef MAX_CONNECTIONS
#define MAX_CONNECTIONS	16
#endif


typedef struct utmp_database
{
  int fd;
  int old_fd;
  char *file;
  char *old_file;
  time_t mtime;
} utmp_database;


/* The databases we handle.  */
extern utmp_database *utmp_db;
extern utmp_database *wtmp_db;


typedef struct client_connection
{
  int sock;
  /* Access permissions.  */
  int access;

  /* Read pointer.  */
  void *read_base;
  void *read_ptr;
  void *read_end;

  /* Write buffer.  */
  void *write_base;
  void *write_ptr;
  void *write_end;

  /* Database to use for this connection.  */
  utmp_database *database;
  /* Position pointer.  */
  int position;
  
  /* Last read entry.  */
  struct utmp last_entry;

  /* Pointers to the next and previous connections in the list.  */
  struct client_connection *next;
  struct client_connection *prev;
} client_connection;


/* This variable indicates if we have forked.  If set, we log messages
   via the system logger.  Otherwise we simply print the program name
   and the message to standard error.  */
extern int forked;


/* Database functions.  */
utmp_database *open_database (const char *file, const char *old_file);
int synchronize_database (utmp_database *database);
void close_database (utmp_database *database);
int read_entry (utmp_database *database, int position, struct utmp *entry);
int write_entry (utmp_database *database, int position,
		 const struct utmp *entry);
int append_entry (utmp_database *database, const struct utmp *entry);
int read_old_entry (utmp_database *database, int position, struct utmp *entry);
int write_old_entry (utmp_database *database, int position,
		     const struct utmp *entry);

/* Connection oriented functions.  */
client_connection *accept_connection (int sock, int access);
client_connection *find_connection (int sock);
void close_connection (client_connection *connection);
int read_data (client_connection *connection);
int write_data (client_connection *connection);

int proc_utmp_eq (const struct utmp *entry, const struct utmp *match);

void error (int status, int errnum, const char *message, ...);
void warning (int errnum, const char *message, ...);


#endif /* utmpd-private.h  */

