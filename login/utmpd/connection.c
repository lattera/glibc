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

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "utmpd-private.h"


/* Prototypes for the local functions.  */
static client_connection *alloc_connection (void);
static void free_connection (client_connection *connection);
static int set_nonblock_flag (int desc, int value);


/* The head of the connection list.  */
static client_connection *connection_list = NULL;


/* Accept connection on SOCK, with access permissions given by ACCESS.
   Returns a pointer to a newly allocated client_connection if
   successful, NULL if not.  */
client_connection *
accept_connection (int sock, int access)
{
  client_connection *connection;

  connection = alloc_connection ();
  if (connection == NULL)
    return NULL;

  connection->sock = accept (sock, NULL, NULL);
  connection->access = access;
  if (connection->sock < 0)
    {
      free_connection (connection);
      return NULL;
    }
  
  if (set_nonblock_flag (connection->sock, 1) < 0)
    {
      close_connection (connection);
      return NULL;
    }

  return connection;  
}


/* Close CONNECTION.  */
void
close_connection (client_connection *connection)
{
  close (connection->sock);
  free_connection (connection);
}


/* Return the connection for SOCK.  */
client_connection *
find_connection (int sock)
{
  client_connection *connection;

  for (connection = connection_list; connection;
       connection = connection->next)
    {
      if (connection->sock == sock)
	return connection;
    }

  return NULL;
}


static client_connection *
alloc_connection (void)
{
  client_connection *connection;
  size_t read_bufsize = 1024;
  size_t write_bufsize = 1024;

  connection = (client_connection *)malloc (sizeof (client_connection));
  if (connection == NULL)
    return NULL;

  memset (connection, 0, sizeof (client_connection));

  /* Allocate read buffer.  */
  connection->read_base = malloc (read_bufsize);
  connection->read_ptr = connection->read_base;
  connection->read_end = connection->read_base + read_bufsize;
  if (connection->read_base == NULL)
    {
      free (connection);
      return NULL;
    }

  /* Allocate write buffer.  */
  connection->write_base = malloc (write_bufsize);
  connection->write_ptr = connection->write_base;
  connection->write_end = connection->write_base + write_bufsize;
  if (connection->write_base == NULL)
    {
      free (connection->read_base);
      free (connection);
      return NULL;
    }

  /* Link connection.  */
  connection->next = connection_list;
  connection_list = connection;
  if (connection->next)
    connection->next->prev = connection;
  
  return connection;
}


static void
free_connection (client_connection *connection)
{
  /* Unlink connection.  */
  if (connection->next)
    connection->next->prev = connection->prev;
  if (connection->prev)
    connection->prev->next = connection->next;

  /* Take care of the head of the list.  */
  if (connection == connection_list)
    connection_list = connection->next;
  
  /* Free buffers.  */
  if (connection->read_base)
    free (connection->read_base);
  if (connection->write_base)
    free (connection->write_base);

  free (connection);
}


/* Set the `O_NONBLOCK' flag of DESC if VALUE is nonzero,
   or clear the flag if VALUE is 0.
   Return 0 on success, or -1 on error with `errno' set. */
static int
set_nonblock_flag (int desc, int value)
{
  int oldflags = fcntl (desc, F_GETFL, 0);
  /* If reading the flags failed, return error indication now. */
  if (oldflags == -1)
    return -1;
  /* Set just the flag we want to set. */
  if (value != 0)
    oldflags |= O_NONBLOCK;
  else
    oldflags &= ~O_NONBLOCK;
  /* Store modified flag word in the descriptor. */
  return fcntl (desc, F_SETFL, oldflags);
}
