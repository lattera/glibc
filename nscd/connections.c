/* Copyright (c) 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Thorsten Kukuk <kukuk@vt.uni-paderborn.de>, 1998.

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
   Boston, MA 02111-1307, USA. */

#include <errno.h>
#include <error.h>
#include <fcntl.h>
#include <libintl.h>
#include <locale.h>
#include <pthread.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/un.h>

#include "nscd.h"
#include "dbg_log.h"

/* Socket 0 in the array is named and exported into the file namespace
   as a connection point for clients.  There's a one to one
   correspondence between sock[i] and read_polls[i].  */
static int sock[MAX_NUM_CONNECTIONS];
static int socks_active;
static struct pollfd read_polls[MAX_NUM_CONNECTIONS];
static pthread_mutex_t sock_lock = PTHREAD_MUTEX_INITIALIZER;


/* Cleanup.  */
void
close_sockets (void)
{
  int i;

  if (debug_flag)
    dbg_log (_("close_sockets called"));

  pthread_mutex_lock (&sock_lock);

  /* Close sockets.  */
  for (i = 0; i < MAX_NUM_CONNECTIONS; ++i)
    if (sock[i] != 0)
      {
	if (close (sock[i]))
	  dbg_log (_("socket [%d|%d] close: %s"), i, sock[i], strerror (errno));

	sock[i] = 0;
	read_polls[i].fd = -1;
	--socks_active;
      }

  pthread_mutex_unlock (&sock_lock);
}

void
close_socket (int conn)
{
  if (debug_flag > 2)
    dbg_log (_("close socket (%d|%d)"), conn, sock[conn]);

  pthread_mutex_lock (&sock_lock);

  close (sock[conn]);
  sock[conn] = 0;
  read_polls[conn].fd = -1;
  --socks_active;

  pthread_mutex_unlock (&sock_lock);
}

/* Local routine, assigns a socket to a new connection request.  */
static void
handle_new_connection (void)
{
  int i;

  if (debug_flag > 2)
    dbg_log (_("handle_new_connection"));

  pthread_mutex_lock (&sock_lock);

  if (socks_active < MAX_NUM_CONNECTIONS)
    /* Find a free socket entry to use.  */
    for (i = 1; i < MAX_NUM_CONNECTIONS; ++i)
      {
	if (sock[i] == 0)
	  {
	    if ((sock[i] = accept (sock[0], NULL, NULL)) < 0)
	      {
		dbg_log (_("socket accept: %s"), strerror (errno));
		return;
	      }
	    ++socks_active;
	    read_polls[i].fd = sock[i];
	    read_polls[i].events = POLLRDNORM;
	    if (debug_flag > 2)
	      dbg_log (_("handle_new_connection used socket %d|%d"), i,
		       sock[i]);
	    break;
	  }
      }
  else
    {
      int black_widow_sock;
      dbg_log (_("Supported number of simultaneous connections exceeded"));
      dbg_log (_("Ignoring client connect request"));
      /* There has to be a better way to ignore a connection request,..
	 when I get my hands on a sockets wiz I'll modify this.  */
      black_widow_sock  = accept (sock[0], NULL, NULL);
      close (black_widow_sock);
    }
  pthread_mutex_unlock (&sock_lock);
}

/* Local routine, reads a request off a socket indicated by read_polls.  */
static int
handle_new_request (int **connp, request_header **reqp, char **key)
{
  ssize_t nbytes;
  int i, found = 0;

  if (debug_flag)
    dbg_log ("handle_new_request");

  /* Find the descriptor.  */
  for (i = 1; i < MAX_NUM_CONNECTIONS; ++i) {
    if (read_polls[i].fd >= 0
	&& read_polls[i].revents & (POLLRDNORM|POLLERR|POLLNVAL))
      {
	found = i;
	break;
      }
    if (read_polls[i].fd >= 0 && (read_polls[i].revents & POLLHUP))
      {
	/* Don't close the socket, we still need to send data.  Just
	   stop polling for more data now.  */
	read_polls[i].fd = -1;
      }
  }

  if (found == 0)
    {
      dbg_log (_("No sockets with data found !"));
      return -1;
    }

  if (debug_flag > 2)
    dbg_log (_("handle_new_request uses socket %d"), i);

  /* Read from it.  */
  nbytes = read (sock[i], *reqp, sizeof (request_header));
  if (nbytes != sizeof (request_header))
    {
      /* Handle non-data read cases.  */
      if (nbytes == 0)
	{
	  /* Close socket down.  */
	  if (debug_flag > 2)
	    dbg_log (_("Real close socket %d|%d"), i, sock[i]);

	  pthread_mutex_lock (&sock_lock);
	  read_polls[i].fd = -1;
	  close (sock[i]);
	  sock[i] = 0;
	  --socks_active;
	  pthread_mutex_unlock (&sock_lock);
	}
      else
	if (nbytes < 0)
	  {
	    dbg_log (_("Read(%d|%d) error on get request: %s"),
		     i, sock[i], strerror (errno));
	    exit (1);
	  }
	else
	  dbg_log (_("Read, data < request buf size, ignoring data"));

      return -1;
    }
  else
    {
      *key = malloc ((*reqp)->key_len + 1);
      /* Read the key from it */
      nbytes = read (sock[i], *key, (*reqp)->key_len);
      if (nbytes != (*reqp)->key_len)
	{
	  /* Handle non-data read cases.  */
	  if (nbytes == 0)
	    {
	      /* Close socket down.  */
	      if (debug_flag > 2)
		dbg_log (_("Real close socket %d|%d"), i, sock[i]);

	      pthread_mutex_lock (&sock_lock);
	      read_polls[i].fd = -1;
	      close (sock[i]);
	      sock[i] = 0;
	      --socks_active;
	      pthread_mutex_unlock (&sock_lock);
	    }
	  else
	    if (nbytes < 0)
	      {
		perror (_("Read() error on get request"));
		return 0;
	      }
	    else
	      fputs (_("Read, data < request buf size, ignoring data"),
		     stderr);

	  free (*key);
	  return -1;
	}
      else
	{
	  /* Ok, have a live one, A real data req buf has been obtained.  */
	  (*key)[(*reqp)->key_len] = '\0';
	  **connp = i;
	  return 0;
	}
    }
}

void
get_request (int *conn, request_header *req, char **key)
{
  int done = 0;
  int nr;

  if (debug_flag)
    dbg_log ("get_request");

  /* loop, processing new connection requests until a client buffer
     is read in on an existing connection.  */
  while (!done)
    {
      /* Poll active connections.  */
      nr = poll (read_polls, MAX_NUM_CONNECTIONS, -1);
      if (nr <= 0)
	{
	  perror (_("Poll new reads"));
	  exit (1);
	}
      if (read_polls[0].revents & (POLLRDNORM|POLLERR|POLLHUP|POLLNVAL))
	/* Handle the case of a new connection request on the named socket.  */
	handle_new_connection ();
      else
	{
	  /* Read data from client specific descriptor.  */
	  if (handle_new_request (&conn, &req, key) == 0)
	    done = 1;
	}
    } /* While not_done.  */
}

void
init_sockets (void)
{
  struct sockaddr_un sock_addr;
  int i;

  /* Initialize the connections db.  */
  socks_active = 0;

  /* Initialize the poll array. */
  for (i = 0; i < MAX_NUM_CONNECTIONS; i++)
    read_polls[i].fd = -1;

  /* Create the socket.  */
  sock[0] = socket (AF_UNIX, SOCK_STREAM, 0);
  if (sock[0] < 0)
    {
      perror (_("cannot create socket"));
      exit (1);
    }
  /* Bind a name to the socket.  */
  sock_addr.sun_family = AF_UNIX;
  strcpy (sock_addr.sun_path, _PATH_NSCDSOCKET);
  if (bind (sock[0], (struct sockaddr *) &sock_addr, sizeof (sock_addr)) < 0)
    {
      dbg_log ("%s: %s", _PATH_NSCDSOCKET, strerror (errno));
      exit (1);
    }
  /* Set permissions for the socket.  */
  chmod (_PATH_NSCDSOCKET, 0666);

  /* Set the socket up to accept connections.  */
  if (listen (sock[0], MAX_NUM_CONNECTIONS) < 0)
    {
      perror (_("cannot enable socket to accept connections"));
      exit (1);
    }

  /* Add the socket to the server's set of active sockets.  */
  read_polls[0].fd = sock[0];
  read_polls[0].events = POLLRDNORM;
  ++socks_active;
}

void
pw_send_answer (int conn, struct passwd *pwd)
{
  struct iovec vec[6];
  pw_response_header resp;
  size_t total_len;
  int nblocks;

  resp.version = NSCD_VERSION;
  if (pwd != NULL)
    {
      resp.found = 1;
      resp.pw_name_len = strlen (pwd->pw_name);
      resp.pw_passwd_len = strlen (pwd->pw_passwd);
      resp.pw_uid = pwd->pw_uid;
      resp.pw_gid = pwd->pw_gid;
      resp.pw_gecos_len = strlen (pwd->pw_gecos);
      resp.pw_dir_len = strlen (pwd->pw_dir);
      resp.pw_shell_len = strlen (pwd->pw_shell);
    }
  else
    {
      resp.found = 0;
      resp.pw_name_len = 0;
      resp.pw_passwd_len = 0;
      resp.pw_uid = -1;
      resp.pw_gid = -1;
      resp.pw_gecos_len = 0;
      resp.pw_dir_len = 0;
      resp.pw_shell_len = 0;
    }
  if (sock[conn] == 0)
    {
      dbg_log (_("bad connection id on send response [%d|%d]"),
	       conn, sock[conn]);
      return;
    }

  /* Add response header.  */
  vec[0].iov_base = &resp;
  vec[0].iov_len = sizeof (pw_response_header);
  total_len = sizeof (pw_response_header);
  nblocks = 1;

  if (resp.found)
    {
      /* Add pw_name.  */
      vec[1].iov_base = pwd->pw_name;
      vec[1].iov_len = resp.pw_name_len;
      total_len += resp.pw_name_len;
      /* Add pw_passwd.  */
      vec[2].iov_base = pwd->pw_passwd;
      vec[2].iov_len = resp.pw_passwd_len;
      total_len += resp.pw_passwd_len;
      /* Add pw_gecos.  */
      vec[3].iov_base = pwd->pw_gecos;
      vec[3].iov_len = resp.pw_gecos_len;
      total_len += resp.pw_gecos_len;
      /* Add pw_dir.  */
      vec[4].iov_base = pwd->pw_dir;
      vec[4].iov_len = resp.pw_dir_len;
      total_len += resp.pw_dir_len;
      /* Add pw_shell.  */
      vec[5].iov_base = pwd->pw_shell;
      vec[5].iov_len = resp.pw_shell_len;
      total_len += resp.pw_shell_len;

      nblocks = 6;
    }

  /* Send all the data.  */
  if (writev (sock[conn], vec, nblocks) != total_len)
    dbg_log (_("write incomplete on send passwd answer: %s"),
	     strerror (errno));
}

void
pw_send_disabled (int conn)
{
  pw_response_header resp;

  resp.version = NSCD_VERSION;
  resp.found = -1;
  resp.pw_name_len = 0;
  resp.pw_passwd_len = 0;
  resp.pw_uid = -1;
  resp.pw_gid = -1;
  resp.pw_gecos_len = 0;
  resp.pw_dir_len = 0;
  resp.pw_shell_len = 0;

  if (sock[conn] == 0)
    {
      dbg_log (_("bad connection id on send response [%d|%d]"),
	       conn, sock[conn]);
      return;
    }

  /* Send response header.  */
  if (write (sock[conn], &resp, sizeof (pw_response_header))
      != sizeof (pw_response_header))
    dbg_log (_("write incomplete on send response: %s"), strerror (errno));
}

void
gr_send_answer (int conn, struct group *grp)
{
  struct iovec *vec;
  size_t *len;
  gr_response_header resp;
  size_t total_len;
  int nblocks;

  resp.version = NSCD_VERSION;
  if (grp != NULL)
    {
      resp.found = 1;
      resp.gr_name_len = strlen (grp->gr_name);
      resp.gr_passwd_len = strlen (grp->gr_passwd);
      resp.gr_gid = grp->gr_gid;
      resp.gr_mem_len = 0;
      while (grp->gr_mem[resp.gr_mem_len])
	++resp.gr_mem_len;
    }
  else
    {
      resp.found = 0;
      resp.gr_name_len = 0;
      resp.gr_passwd_len = 0;
      resp.gr_gid = -1;
      resp.gr_mem_len = 0;
    }
  if (sock[conn] == 0)
    {
      dbg_log (_("bad connection id on send response [%d|%d]"),
	       conn, sock[conn]);
      return;
    }

  /* We have no fixed number of records so allocate the IOV here.  */
  vec = alloca ((3 + 1 + resp.gr_mem_len) * sizeof (struct iovec));
  len = alloca (resp.gr_mem_len * sizeof (size_t));

  /* Add response header.  */
  vec[0].iov_base = &resp;
  vec[0].iov_len = sizeof (gr_response_header);
  total_len = sizeof (gr_response_header);
  nblocks = 1;

  if (resp.found)
    {
      unsigned int l = 0;

      /* Add gr_name.  */
      vec[1].iov_base = grp->gr_name;
      vec[1].iov_len = resp.gr_name_len;
      total_len += resp.gr_name_len;
      /* Add gr_passwd.  */
      vec[2].iov_base = grp->gr_passwd;
      vec[2].iov_len = resp.gr_passwd_len;
      total_len += resp.gr_passwd_len;
      nblocks = 3;

      if (grp->gr_mem[l])
	{
	  vec[3].iov_base = len;
	  vec[3].iov_len = resp.gr_mem_len * sizeof (size_t);
	  total_len += resp.gr_mem_len * sizeof (size_t);
	  nblocks = 4;

	  do
	    {
	      len[l] = strlen (grp->gr_mem[l]);

	      vec[nblocks].iov_base = grp->gr_mem[l];
	      vec[nblocks].iov_len = len[l];
	      total_len += len[l];

	      ++nblocks;
	    }
	  while (grp->gr_mem[++l]);
	}
    }

  /* Send all the data.  */
  while (nblocks > UIO_MAXIOV)
    {
      if (writev (sock[conn], vec, UIO_MAXIOV) != total_len)
	dbg_log (_("write incomplete on send group answer: %s"),
		 strerror (errno));
      vec += UIO_MAXIOV;
      nblocks -= UIO_MAXIOV;
    }
  if (writev (sock[conn], vec, nblocks) != total_len)
    dbg_log (_("write incomplete on send group answer: %s"),
	     strerror (errno));
}

void
gr_send_disabled (int conn)
{
  gr_response_header resp;

  resp.version = NSCD_VERSION;
  resp.found = -1;
  resp.gr_name_len = 0;
  resp.gr_passwd_len = 0;
  resp.gr_gid = -1;
  resp.gr_mem_len = 0;

  if (sock[conn] == 0)
    {
      dbg_log (_("bad connection id on send gr_disabled response [%d|%d]"),
	       conn, sock[conn]);
      return;
    }

  /* Send response header.  */
  if (write (sock[conn], &resp, sizeof (gr_response_header))
      != sizeof (gr_response_header))
    dbg_log (_("write incomplete on send gr_disabled response: %s"),
	     strerror (errno));
}

void
stat_send (int conn, stat_response_header *resp)
{
  if (sock[conn] == 0)
    {
      dbg_log (_("bad connection id on send stat response [%d|%d]"),
	       conn, sock[conn]);
      return;
    }

  /* send response header.  */
  if (write (sock[conn], resp, sizeof (stat_response_header))
      != sizeof (stat_response_header))
    dbg_log (_("write incomplete on send stat response: %s"),
	     strerror (errno));
}
