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

#ifndef _UTMPD_H
#define _UTMPD_H		1

/* This is an *internal* header.  */

#include <stddef.h>
#include <utmp.h>


/* Paths to daemon sockets.  */
#define _PATH_UTMPD_RO	"/var/run/utmpd.ro"
#define _PATH_UTMPD_RW	"/var/run/utmpd.rw"


/* Path to PID file.  */
#define _PATH_UTMPDPID	"/var/run/utmpd.pid"


/* Version number of the daemon interface.  */
#define UTMPD_VERSION	1


/* Services provided.  */
typedef enum
{
  UTMPD_REQ_SETUTENT,
  UTMPD_REQ_GETUTENT,
  UTMPD_REQ_ENDUTENT,
  UTMPD_REQ_GETUTLINE,
  UTMPD_REQ_GETUTID,
  UTMPD_REQ_PUTUTLINE,
  UTMPD_REQ_UPDWTMP
} request_type;


/* Header common to all requests.  */
typedef struct
{
  /* Version number of the daemon interface.  */
  int version;
  /* Number of bytes in this request.  */
  size_t size;
  /* Service requested.  */
  request_type type;
} request_header;

typedef struct
{
  request_header header;
  /* File to use.  */
  char file[0];
} setutent_request;

typedef struct
{
  request_header header;
} getutent_request, endutent_request;

typedef struct
{
  request_header header;
  /* Entry to match.  */
  struct utmp line;
} getutline_request;

typedef struct
{
  request_header header;
  /* Entry to match.  */
  struct utmp id;
} getutid_request;

typedef struct
{
  request_header header;
  /* Entry to write.  */
  struct utmp utmp;
} pututline_request;

typedef struct
{
  request_header header;
  /* Entry to write.  */
  struct utmp utmp;
  /* File to use.  */
  char file[0];
} updwtmp_request;


/* Header common to all replies.  */
typedef struct
{
  /* Version number of the daemon interface.  */
  int version;
  /* Number of bytes in this reply.  */
  size_t size;
  /* Answer to the request.  */
  request_type type;
} reply_header;

typedef struct
{
  reply_header header;
  /* Error code.  */
  int errnum;
  /* Return value.  */
  int result;
} setutent_reply, endutent_reply, pututline_reply, updwtmp_reply;

typedef struct
{
  reply_header header;
  /* Found entry.  */
  struct utmp entry;
  /* Error code.  */
  int errnum;
  /* Return value.  */
  int result;
} getutent_reply, getutline_reply, getutid_reply;

#endif /* utmpd.h  */
