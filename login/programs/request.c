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

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <utmp.h>

#include "utmpd.h"
#include "utmpd-private.h"


/* Prototypes for the local functions.  */
static int process_request (client_connection *connection);
static int send_reply (client_connection *connect, const reply_header *reply);

static int do_setutent (client_connection *connection);
static int do_getutent (client_connection *connection);
static int do_endutent (client_connection *connection);
static int do_getutline (client_connection *connection);
static int do_getutid (client_connection *connection);
static int do_pututline (client_connection *connection);
static int do_updwtmp (client_connection *connection);

static int internal_getut_r (client_connection *connection,
			     const struct utmp *id, struct utmp *buffer);


/* Read data from the client on CONNECTION.  */
int
read_data (client_connection *connection)
{
  ssize_t nbytes;

  assert (connection);
  assert ((connection->read_end - connection->read_ptr) > 0);

  /* Read data.  */
  nbytes = read (connection->sock, connection->read_ptr,
		 connection->read_end - connection->read_ptr);
  if (nbytes > 0)
    {
      size_t total_bytes;

      /* Update read pointer.  */
      connection->read_ptr += nbytes;

      /* Check if we have a complete request header.  */
      total_bytes = connection->read_ptr - connection->read_base;
      if (total_bytes >= sizeof (request_header))
	{
	  request_header *header;

	  /* Check if we have a complete request.  */
	  header = (request_header *)connection->read_base;
	  if (total_bytes >= header->size)
	    {
	      /* Process the request.  */
	      if (process_request (connection) < 0)
		return -1;

	      /* Adjust read pointer, and flush buffer.  */
	      connection->read_ptr -= header->size;
	      memmove (connection->read_base,
		       connection->read_base + header->size,
		       connection->read_ptr - connection->read_base);
	    }
	}

      return 0;
    }

  if (nbytes < 0)
    error (0, errno, "cannot read from client");

  return -1;
}


/* Write data to the client on CONNECTION.  */
int
write_data (client_connection *connection)
{
  ssize_t nbytes;

  assert (connection);
  assert ((connection->write_ptr - connection->write_base) > 0);

  /* Write data.  */
  nbytes = write (connection->sock, connection->write_base,
		  connection->write_ptr - connection->write_base);
  if (nbytes > 0)
    {
      /* Adjust write pointer and flush buffer.  */
      connection->write_ptr -= nbytes;
      memmove (connection->write_base, connection->write_base + nbytes,
	       connection->write_ptr - connection->write_base);

      return 0;
    }

  if (nbytes < 0)
    error (0, errno, "cannot write to client");

  return -1;
}


/* Process the request received on CONNECTION.  Returns 0 if
   successful, -1 if not.  */
static int
process_request (client_connection *connection)
{
  request_header *header;

  assert (connection);
  assert (connection->read_base);

  header = (request_header *)connection->read_base;
  if (header->version != UTMPD_VERSION)
    {
      warning (EINVAL, "invalid protocol version");
      return -1;
    }

  switch (header->type)
    {
    case UTMPD_REQ_SETUTENT:	return do_setutent (connection);
    case UTMPD_REQ_GETUTENT:	return do_getutent (connection);
    case UTMPD_REQ_ENDUTENT:	return do_endutent (connection);
    case UTMPD_REQ_GETUTLINE:	return do_getutline (connection);
    case UTMPD_REQ_GETUTID:	return do_getutid (connection);
    case UTMPD_REQ_PUTUTLINE:	return do_pututline (connection);
    case UTMPD_REQ_UPDWTMP:	return do_updwtmp (connection);
    default:
      warning (EINVAL, "invalid request type");
      return -1;
    }
}


/* Send the reply specified by HEADER to the client on CONNECTION.
   Returns 0 if successful, -1 if not.  */
static int
send_reply (client_connection *connection, const reply_header *reply)
{
  /* Check if the reply fits in the buffer.  */
  if ((size_t) (connection->write_end - connection->write_ptr) < reply->size)
    {
      error (0, 0, "buffer overflow");
      return -1;
    }

  /* Copy reply to buffer, and adjust write pointer.  */
  memcpy (connection->write_ptr, reply, reply->size);
  connection->write_ptr += reply->size;

  return 0;
}


static int
do_setutent (client_connection *connection)
{
  setutent_request *request;
  setutent_reply reply;

  request = (setutent_request *)connection->read_base;
  if (request->header.size != sizeof (setutent_request))
    {
      warning (EINVAL, "invalid request size");
      return -1;
    }

  /* Initialize reply.  */
  reply.header.version = UTMPD_VERSION;
  reply.header.size = sizeof (setutent_reply);
  reply.header.type = UTMPD_REQ_SETUTENT;

  /* Select database.  */
  if (!strncmp (request->file, _PATH_UTMP, sizeof request->file))
    connection->database = utmp_db;
  else
    {
      errno = EINVAL;
      goto return_error;
    }

  /* Initialize position pointer.  */
  connection->position = 0;

#if _HAVE_UT_TYPE - 0
  /* Make sure the entry won't match.  */
  connection->last_entry.ut_type = -1;
#endif

  reply.errnum = 0;
  reply.result = 0;
  return send_reply (connection, &reply.header);

return_error:
  reply.errnum = errno;
  reply.result = -1;
  return send_reply (connection, &reply.header);
}


static int
do_getutent (client_connection *connection)
{
  getutent_request *request;
  getutent_reply reply;

  request = (getutent_request *)connection->read_base;
  if (request->header.size != sizeof (getutent_request))
    {
      warning (EINVAL, "invalid request size");
      return -1;
    }

  /* Initialize reply.  */
  reply.header.version = UTMPD_VERSION;
  reply.header.size = sizeof (getutent_reply);
  reply.header.type = UTMPD_REQ_GETUTENT;

  if (connection->database == NULL || connection->position == -1)
    {
      errno = ESRCH;
      goto return_error;
    }

  /* Make sure we're in synch with the ordinary file.  */
  if (synchronize_database (connection->database) < 0)
    {
      errno = ESRCH;
      goto return_error;
    }

  /* Read the next entry from the database.  */
  if (read_entry (connection->database, connection->position,
		  &connection->last_entry) < 0)
    {
      connection->position = -1;
      errno = ESRCH;
      goto return_error;
    }

  /* Update position pointer.  */
  connection->position++;

  memcpy (&reply.entry, &connection->last_entry, sizeof (struct utmp));
  reply.errnum = 0;
  reply.result = 0;
  return send_reply (connection, (reply_header *)&reply);

return_error:
  memset (&reply.entry, 0, sizeof (struct utmp));
  reply.errnum = errno;
  reply.result = -1;
  return send_reply (connection, &reply.header);
}


static int
do_endutent (client_connection *connection)
{
  endutent_request *request;
  endutent_reply reply;

  request = (endutent_request *)connection->read_base;
  if (request->header.size != sizeof (endutent_request))
    {
      warning (EINVAL, "invalid request size");
      return -1;
    }

  /* Deselect database.  */
  connection->database = NULL;

  /* Formulate reply.  */
  reply.header.version = UTMPD_VERSION;
  reply.header.size = sizeof (endutent_reply);
  reply.header.type = UTMPD_REQ_ENDUTENT;
  reply.errnum = 0;
  reply.result = 0;

  return send_reply (connection, &reply.header);
}


static int
do_getutline (client_connection *connection)
{
  getutline_request *request;
  getutline_reply reply;

  request = (getutline_request *)connection->read_base;
  if (request->header.size != sizeof (getutline_request))
    {
      warning (EINVAL, "invalid request size");
      return -1;
    }

  /* Initialize reply.  */
  reply.header.version = UTMPD_VERSION;
  reply.header.size = sizeof (getutline_reply);
  reply.header.type = UTMPD_REQ_GETUTLINE;

  if (connection->database == NULL || connection->position == -1)
    {
      errno = ESRCH;
      goto return_error;
    }

  /* Make sure we're in synch with the ordinary file.  */
  if (synchronize_database (connection->database) < 0)
    {
      errno = ESRCH;
      goto return_error;
    }

  while (1)
    {
      /* Read the next entry.  */
      if (read_entry (connection->database, connection->position,
		      &connection->last_entry) < 0)
	{
	  connection->position = -1;
	  errno = ESRCH;
	  goto return_error;
	}
      connection->position++;

      /* Stop if we found a user or login entry.  */
      if (
#if _HAVE_UT_TYPE - 0
	  (connection->last_entry.ut_type == USER_PROCESS
	   || connection->last_entry.ut_type == LOGIN_PROCESS)
	  &&
#endif
	  !strncmp (request->line.ut_line, connection->last_entry.ut_line,
		    sizeof request->line.ut_line))
	break;
    }

  memcpy (&reply.entry, &connection->last_entry, sizeof (struct utmp));
  reply.errnum = 0;
  reply.result = 0;
  return send_reply (connection, &reply.header);

return_error:
  memset (&reply.entry, 0, sizeof (struct utmp));
  reply.errnum = errno;
  reply.result = -1;
  return send_reply (connection, &reply.header);
}


static int
do_getutid (client_connection *connection)
{
  getutid_request *request;
  getutid_reply reply;

  request = (getutid_request *)connection->read_base;
  if (request->header.size != sizeof (getutid_request))
    {
      warning (EINVAL, "invalid request size");
      return -1;
    }

  /* Initialize reply.  */
  reply.header.version = UTMPD_VERSION;
  reply.header.size = sizeof (getutid_reply);
  reply.header.type = UTMPD_REQ_GETUTID;

  if (connection->database == NULL || connection->position == -1)
    {
      errno = ESRCH;
      goto return_error;
    }

  /* Make sure we're in synch with the ordinary file.  */
  if (synchronize_database (connection->database) < 0)
    {
      errno = ESRCH;
      goto return_error;
    }

  if (internal_getut_r (connection, &request->id,
			&connection->last_entry) < 0)
    {
      errno = ESRCH;
      goto return_error;
    }

  reply.errnum = 0;
  reply.result = 0;
  memcpy (&reply.entry, &connection->last_entry, sizeof (struct utmp));
  return send_reply (connection, &reply.header);

return_error:
  memset (&reply.entry, 0, sizeof (struct utmp));
  reply.errnum = errno;
  reply.result = -1;
  return send_reply (connection, &reply.header);
}


static int
do_pututline (client_connection *connection)
{
  pututline_request *request;
  pututline_reply reply;
  struct utmp buffer;
  int found;

  request = (pututline_request *)connection->read_base;
  if (request->header.size != sizeof (pututline_request))
    {
      warning (EINVAL, "invalid request size");
      return -1;
    }

  /* Initialize reply.  */
  reply.header.version = UTMPD_VERSION;
  reply.header.size = sizeof (pututline_reply);
  reply.header.type = UTMPD_REQ_PUTUTLINE;

  if (!(connection->access & W_OK))
    {
      errno = EPERM;
      goto return_error;
    }

  if (connection->database == NULL || connection->position == -1)
    {
      errno = ESRCH;
      goto return_error;
    }

  /* Make sure we're in synch with the ordinary file.  */
  if (synchronize_database (connection->database) < 0)
    {
      errno = ESRCH;
      goto return_error;
    }

  /* Find the correct place to insert the data.  */
  if (connection->position > 0
    && (
#if _HAVE_UT_TYPE - 0
	(connection->last_entry.ut_type == request->utmp.ut_type
	 && (connection->last_entry.ut_type == RUN_LVL
	     || connection->last_entry.ut_type == BOOT_TIME
	     || connection->last_entry.ut_type == OLD_TIME
	     || connection->last_entry.ut_type == NEW_TIME))
	||
#endif
	proc_utmp_eq (&connection->last_entry, &request->utmp)))
    found = 1;
  else
    found = internal_getut_r (connection, &request->utmp, &buffer);

  if (found < 0)
    {
      /* We append the next entry.  */
      connection->position =
	append_entry (connection->database, &request->utmp);
      if (connection->position < 0)
	goto return_error;
    }
  else
    {
      /* We replace the just read entry.  */
      connection->position--;
      if (write_entry (connection->database, connection->position,
		       &request->utmp) < 0)
	goto return_error;
    }

  /* Write the entry to the compatibility file.  */
  write_old_entry (connection->database, connection->position, &request->utmp);

  /* Update position pointer.  */
  connection->position++;

  reply.errnum = 0;
  reply.result = 0;
  return send_reply (connection, &reply.header);

return_error:
  reply.errnum = errno;
  reply.result = -1;
  return send_reply (connection, &reply.header);
}


static int
do_updwtmp (client_connection *connection)
{
  updwtmp_request *request;
  updwtmp_reply reply;
  utmp_database *database;

  request = (updwtmp_request *)connection->read_base;
  if (request->header.size != sizeof (updwtmp_request))
    {
      warning (EINVAL, "invalid request size");
      return -1;
    }

  /* Initialize reply.  */
  reply.header.version = UTMPD_VERSION;
  reply.header.size = sizeof (updwtmp_reply);
  reply.header.type = UTMPD_REQ_UPDWTMP;

  if (!(connection->access & W_OK))
    {
      errno = EPERM;
      goto return_error;
    }

  /* Select database.  */
  if (!strncmp (request->file, _PATH_UTMP, sizeof request->file))
    database = utmp_db;
  else
    {
      errno = EINVAL;
      goto return_error;
    }

  /* Make sure we're in synch with the ordinary file.  */
  if (synchronize_database (database) < 0)
    {
      errno = ESRCH;
      goto return_error;
    }

  /* Append the entry.  */
  if (append_entry (database, &request->utmp) < 0)
    goto return_error;

  reply.errnum = 0;
  reply.result = 0;
  return send_reply (connection, &reply.header);

return_error:
  reply.errnum = errno;
  reply.result = -1;
  return send_reply (connection, &reply.header);
}


/* This function is identical to the one in login/utmp_file.c.  */
int
proc_utmp_eq (const struct utmp *entry, const struct utmp *match)
{
  return
    (
#if _HAVE_UT_TYPE - 0
     (entry->ut_type == INIT_PROCESS
      || entry->ut_type == LOGIN_PROCESS
      || entry->ut_type == USER_PROCESS
      || entry->ut_type == DEAD_PROCESS)
     &&
     (match->ut_type == INIT_PROCESS
      || match->ut_type == LOGIN_PROCESS
      || match->ut_type == USER_PROCESS
      || match->ut_type == DEAD_PROCESS)
     &&
#endif
#if _HAVE_UT_ID - 0
     (entry->ut_id[0] && match->ut_id[0]
      ? strncmp (entry->ut_id, match->ut_id, sizeof match->ut_id) == 0
      : strncmp (entry->ut_line, match->ut_line, sizeof match->ut_line) == 0)
#else
     strncmp (entry->ut_line, match->ut_line, sizeof match->ut_line) == 0
#endif
     );
}


/* This function is derived from the one in login/utmp_file.c.  */
static int
internal_getut_r (client_connection *connection,
		  const struct utmp *id, struct utmp *buffer)
{
#if _HAVE_UT_TYPE - 0
  if (id->ut_type == RUN_LVL || id->ut_type == BOOT_TIME
      || id->ut_type == OLD_TIME || id->ut_type == NEW_TIME)
    {
      /* Search for next entry with type RUN_LVL, BOOT_TIME,
	 OLD_TIME, or NEW_TIME.  */

      while (1)
	{
	  /* Read the next entry.  */
	  if (read_entry (connection->database, connection->position,
			  buffer) < 0)
	    {
	      connection->position = -1;
	      return -1;
	    }
	  connection->position++;

	  if (id->ut_type == buffer->ut_type)
	    break;
	}
    }
  else
#endif /* _HAVE_UT_TYPE */
    {
      /* Search for the next entry with the specified ID and with type
	 INIT_PROCESS, LOGIN_PROCESS, USER_PROCESS, or DEAD_PROCESS.  */

      while (1)
	{
	  /* Read the next entry.  */
	  if (read_entry (connection->database, connection->position,
			  buffer) < 0)
	    {
	      connection->position = -1;
	      return -1;
	    }
	  connection->position++;

	  if (proc_utmp_eq (buffer, id))
	    break;
	}
    }

  return 0;
}
