/* Copyright (C) 1997, 1998 Free Software Foundation, Inc.
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

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <utmp.h>

#include "utmp-private.h"
#include "programs/utmpd.h"


/* Descriptor for the socket.  */
static int daemon_sock = -1;


/* Functions defined here.  */
static int setutent_daemon (void);
static int getutent_r_daemon (struct utmp *buffer, struct utmp **result);
static int getutid_r_daemon (const struct utmp *line, struct utmp *buffer,
			     struct utmp **result);
static int getutline_r_daemon (const struct utmp *id, struct utmp *buffer,
			       struct utmp **result);
static struct utmp *pututline_daemon (const struct utmp *utmp);
static void endutent_daemon (void);
static int updwtmp_daemon (const char *file, const struct utmp *utmp);

/* Jump table for daemon functions.  */
struct utfuncs __libc_utmp_daemon_functions =
{
  setutent_daemon,
  getutent_r_daemon,
  getutid_r_daemon,
  getutline_r_daemon,
  pututline_daemon,
  endutent_daemon,
  updwtmp_daemon
};

static int do_setutent (int sock);
static int do_getutent (int sock, struct utmp *buffer);
static int do_getutid (int sock, const struct utmp *id,
			    struct utmp *buffer);
static int do_pututline (int sock, const struct utmp *utmp);
static int do_getutline (int sock, const struct utmp *line,
			 struct utmp *buffer);
static int do_pututline (int sock, const struct utmp *utmp);
static int do_endutent (int sock);
static int do_updwtmp (int sock, const char *file,
		       const struct utmp *utmp);

static int open_socket (const char *name);
static int send_request (int sock, const request_header *request,
			 reply_header *reply);


static int
setutent_daemon (void)
{
  if (access (_PATH_UTMPD_RW, F_OK) == -1
      && access (_PATH_UTMPD_RO, F_OK) == -1)
    return 0;

  if (daemon_sock < 0)
    {
      daemon_sock = open_socket (_PATH_UTMPD_RW);
      if (daemon_sock < 0)
	{
	  /* Hhm, read-write access did not work.  Try read-only.  */
	  daemon_sock = open_socket (_PATH_UTMPD_RO);
	  if (daemon_sock < 0)
	    return 0;
	}
    }

  /* Send request to the daemon.  */
  if (do_setutent (daemon_sock) < 0)
    return 0;

  return 1;
}


static int
getutent_r_daemon (struct utmp *buffer, struct utmp **result)
{
  assert (daemon_sock >= 0);

  /* Send request to the daemon.  */
  if (do_getutent (daemon_sock, buffer) < 0)
    {
      *result = NULL;
      return -1;;
    }

  *result = buffer;
  return 0;
}


static int
getutid_r_daemon (const struct utmp *id, struct utmp *buffer,
		  struct utmp **result)
{
  assert (daemon_sock >= 0);

  /* Send request to the daemon.  */
  if (do_getutid (daemon_sock, id, buffer) < 0)
    {
      *result = NULL;
      return -1;
    }

  *result = buffer;
  return 0;
}


static int
getutline_r_daemon (const struct utmp *line, struct utmp *buffer,
		    struct utmp **result)
{
  assert (daemon_sock >= 0);

  /* Send request to the daemon.  */
  if (do_getutline (daemon_sock, line, buffer) < 0)
    {
      *result = NULL;
      return -1;
    }

  *result = buffer;
  return 0;
}


static struct utmp *
pututline_daemon (const struct utmp *utmp)
{
  assert (daemon_sock >= 0);

  /* Send request to the daemon.  */
  if (do_pututline (daemon_sock, utmp) < 0)
    return NULL;

  return (struct utmp *)utmp;
}


static void
endutent_daemon (void)
{
  assert (daemon_sock >= 0);

  /* Send request to the daemon.  */
  do_endutent (daemon_sock);

  close (daemon_sock);
  daemon_sock = -1;
}


static int
updwtmp_daemon (const char *file, const struct utmp *utmp)
{
  int sock;

  /* Only try to open for both reading and writing.  */
  sock = open_socket (_PATH_UTMPD_RW);
  if (sock < 0)
    return -1;

  /* Send request to the daemon.  */
  if (do_updwtmp (sock, file, utmp) < 0)
    {
      close (sock);
      return -1;
    }

  close (sock);
  return 0;
}


static int
do_setutent (int sock)
{
  setutent_request *request;
  setutent_reply reply;
  size_t size;
  size_t name_len;

  name_len = strlen (__libc_utmp_file_name) + 1;
  size = sizeof (setutent_request) + name_len;

  request = malloc (size);
  if (request == NULL)
    return -1;

  request->header.version = UTMPD_VERSION;
  request->header.size = size;
  request->header.type = UTMPD_REQ_SETUTENT;
  memcpy (request->file, __libc_utmp_file_name, name_len);

  reply.header.version = UTMPD_VERSION;
  reply.header.size = sizeof (setutent_reply);
  reply.header.type = UTMPD_REQ_SETUTENT;

  if (send_request (sock, &request->header, &reply.header) < 0)
    {
      free (request);
      return -1;
    }

  if (reply.result < 0)
    __set_errno (reply.errnum);

  free (request);
  return reply.result;
}

static int
do_getutent (int sock, struct utmp *buffer)
{
  getutent_request request;
  getutent_reply reply;

  request.header.version = UTMPD_VERSION;
  request.header.size = sizeof (getutent_request);
  request.header.type = UTMPD_REQ_GETUTENT;

  reply.header.version = UTMPD_VERSION;
  reply.header.size = sizeof (getutent_reply);
  reply.header.type = UTMPD_REQ_GETUTENT;

  if (send_request (sock, &request.header, &reply.header) < 0)
    return -1;

  if (reply.result < 0)
    __set_errno (reply.errnum);
  else
    memcpy (buffer, &reply.entry, sizeof (struct utmp));

  return reply.result;
}

static int
do_getutid (int sock, const struct utmp *id, struct utmp *buffer)
{
  getutid_request request;
  getutid_reply reply;

  request.header.version = UTMPD_VERSION;
  request.header.size = sizeof (getutid_request);
  request.header.type = UTMPD_REQ_GETUTID;
  memcpy (&request.id, id, sizeof (struct utmp));

  reply.header.version = UTMPD_VERSION;
  reply.header.size = sizeof (getutid_reply);
  reply.header.type = UTMPD_REQ_GETUTID;

  if (send_request (sock, &request.header, &reply.header) < 0)
    return -1;

  if (reply.result < 0)
    __set_errno (reply.errnum);
  else
    memcpy (buffer, &reply.entry, sizeof (struct utmp));

  return reply.result;
}

static int
do_getutline (int sock, const struct utmp *line, struct utmp *buffer)
{
  getutline_request request;
  getutline_reply reply;

  request.header.version = UTMPD_VERSION;
  request.header.size = sizeof (getutline_request);
  request.header.type = UTMPD_REQ_GETUTLINE;
  memcpy (&request.line, line, sizeof (struct utmp));

  reply.header.version = UTMPD_VERSION;
  reply.header.size = sizeof (getutline_reply);
  reply.header.type = UTMPD_REQ_GETUTLINE;

  if (send_request (sock, &request.header, &reply.header) < 0)
    return -1;

  if (reply.result < 0)
    __set_errno (reply.errnum);
  else
    memcpy (buffer, &reply.entry, sizeof (struct utmp));

  return reply.result;
}

static int
do_pututline (int sock, const struct utmp *utmp)
{
  pututline_request request;
  pututline_reply reply;

  request.header.version = UTMPD_VERSION;
  request.header.size = sizeof (pututline_request);
  request.header.type = UTMPD_REQ_PUTUTLINE;
  memcpy (&request.utmp, utmp, sizeof (struct utmp));

  reply.header.version = UTMPD_VERSION;
  reply.header.size = sizeof (pututline_reply);
  reply.header.type = UTMPD_REQ_PUTUTLINE;

  if (send_request (sock, &request.header, &reply.header) < 0)
    return -1;

  if (reply.result < 0)
    __set_errno (reply.errnum);

  return reply.result;
}

static int
do_endutent (int sock)
{
  endutent_request request;
  endutent_reply reply;

  request.header.version = UTMPD_VERSION;
  request.header.size = sizeof (endutent_request);
  request.header.type = UTMPD_REQ_ENDUTENT;

  reply.header.version = UTMPD_VERSION;
  reply.header.size = sizeof (endutent_reply);
  reply.header.type = UTMPD_REQ_ENDUTENT;

  if (send_request (sock, &request.header, &reply.header) < 0)
    return -1;

  if (reply.result < 0)
    __set_errno (reply.errnum);

  return reply.result;
}

static int
do_updwtmp (int sock, const char *file, const struct utmp *utmp)
{
  updwtmp_request *request;
  updwtmp_reply reply;
  size_t size;
  size_t file_len;

  file_len = strlen (file) + 1;
  size = sizeof (updwtmp_request) + file_len;

  request = malloc (size);
  if (request == NULL)
    return -1;

  request->header.version = UTMPD_VERSION;
  request->header.size = size;
  request->header.type = UTMPD_REQ_UPDWTMP;
  memcpy (&request->utmp, utmp, sizeof (struct utmp));
  memcpy (request->file, file, file_len);

  reply.header.version = UTMPD_VERSION;
  reply.header.size = sizeof (updwtmp_reply);
  reply.header.type = UTMPD_REQ_UPDWTMP;

  if (send_request (sock, &request->header, &reply.header) < 0)
    {
      free (request);
      return -1;
    }

  if (reply.result < 0)
    __set_errno (reply.errnum);

  free (request);
  return reply.result;
}


/* Create a socket connected to NAME.  */
static int
open_socket (const char *name)
{
  struct sockaddr_un addr;
  int sock;

  sock = __socket (PF_UNIX, SOCK_STREAM, 0);
  if (sock < 0)
    return -1;

  addr.sun_family = AF_UNIX;
  strcpy (addr.sun_path, name);
  if (connect (sock, (struct sockaddr *) &addr, sizeof (addr)) < 0)
    {
      close (sock);
      return -1;
    }

  return sock;
}

/* Send REQUEST to SOCK, and wait for reply.  Returns 0 if successful,
   storing the reply in REPLY, and -1 if not.  */
static int
send_request (int sock, const request_header *request,
	      reply_header *reply)
{
  reply_header header;
  ssize_t nbytes;

  nbytes = write (sock, request, request->size);
  if (nbytes != (ssize_t) request->size)
    return -1;

  nbytes = read (sock, &header, sizeof (reply_header));
  if (nbytes != sizeof (reply_header))
    return -1;

  if (reply->version != header.version
      || reply->size != header.size
      || reply->type != header.type)
    return -1;

  nbytes = read (sock, reply + 1, reply->size - sizeof (reply_header));
  if (nbytes != (ssize_t) (reply->size - sizeof (reply_header)))
    return -1;

  return 0;
}
