/* Inner loops of cache daemon.
   Copyright (C) 1998, 1999, 2000, 2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1998.

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
#include <error.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <libintl.h>
#include <arpa/inet.h>
#include <sys/param.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>

#include "nscd.h"
#include "dbg_log.h"


/* Mapping of request type to database.  */
static const dbtype serv2db[LASTDBREQ + 1] =
{
  [GETPWBYNAME] = pwddb,
  [GETPWBYUID] = pwddb,
  [GETGRBYNAME] = grpdb,
  [GETGRBYGID] = grpdb,
  [GETHOSTBYNAME] = hstdb,
  [GETHOSTBYNAMEv6] = hstdb,
  [GETHOSTBYADDR] = hstdb,
  [GETHOSTBYADDRv6] = hstdb,
};

/* Map request type to a string.  */
const char *serv2str[LASTREQ] =
{
  [GETPWBYNAME] = "GETPWBYNAME",
  [GETPWBYUID] = "GETPWBYUID",
  [GETGRBYNAME] = "GETGRBYNAME",
  [GETGRBYGID] = "GETGRBYGID",
  [GETHOSTBYNAME] = "GETHOSTBYNAME",
  [GETHOSTBYNAMEv6] = "GETHOSTBYNAMEv6",
  [GETHOSTBYADDR] = "GETHOSTBYADDR",
  [GETHOSTBYADDRv6] = "GETHOSTBYADDRv6",
  [SHUTDOWN] = "SHUTDOWN",
  [GETSTAT] = "GETSTAT",
  [INVALIDATE] = "INVALIDATE"
};

/* The control data structures for the services.  */
static struct database dbs[lastdb] =
{
  [pwddb] = {
    lock: PTHREAD_RWLOCK_WRITER_NONRECURSIVE_INITIALIZER_NP,
    enabled: 0,
    check_file: 1,
    filename: "/etc/passwd",
    module: 211,
    disabled_iov: &pwd_iov_disabled,
    postimeout: 3600,
    negtimeout: 20
  },
  [grpdb] = {
    lock: PTHREAD_RWLOCK_WRITER_NONRECURSIVE_INITIALIZER_NP,
    enabled: 0,
    check_file: 1,
    filename: "/etc/group",
    module: 211,
    disabled_iov: &grp_iov_disabled,
    postimeout: 3600,
    negtimeout: 60
  },
  [hstdb] = {
    lock: PTHREAD_RWLOCK_WRITER_NONRECURSIVE_INITIALIZER_NP,
    enabled: 0,
    check_file: 1,
    filename: "/etc/hosts",
    module: 211,
    disabled_iov: &hst_iov_disabled,
    postimeout: 3600,
    negtimeout: 20
  }
};

/* Number of seconds between two cache pruning runs.  */
#define CACHE_PRUNE_INTERVAL	15

/* Number of threads to use.  */
int nthreads = -1;

/* Socket for incoming connections.  */
static int sock;


/* Initialize database information structures.  */
void
nscd_init (const char *conffile)
{
  struct sockaddr_un sock_addr;
  size_t cnt;

  /* Read the configuration file.  */
  if (nscd_parse_file (conffile, dbs) != 0)
    {
      /* We couldn't read the configuration file.  Disable all services
	 by shutting down the srever.  */
      dbg_log (_("cannot read configuration file; this is fatal"));
      exit (1);
    }
  if (nthreads == -1)
    /* No configuration for this value, assume a default.  */
    nthreads = 2 * lastdb;

  for (cnt = 0; cnt < lastdb; ++cnt)
    if (dbs[cnt].enabled)
      {
	pthread_rwlock_init (&dbs[cnt].lock, NULL);

	dbs[cnt].array = (struct hashentry **)
	  calloc (dbs[cnt].module, sizeof (struct hashentry *));
	if (dbs[cnt].array == NULL)
	  error (EXIT_FAILURE, errno, "while allocating cache");

	if (dbs[cnt].check_file)
	  {
	    /* We need the modification date of the file.  */
	    struct stat st;

	    if (stat (dbs[cnt].filename, &st) < 0)
	      {
		char buf[128];
		/* We cannot stat() the file, disable file checking.  */
		dbg_log (_("cannot stat() file `%s': %s"),
			 dbs[cnt].filename,
			 strerror_r (errno, buf, sizeof (buf)));
		dbs[cnt].check_file = 0;
	      }
	    else
	      dbs[cnt].file_mtime = st.st_mtime;
	  }
      }

  /* Create the socket.  */
  sock = socket (AF_UNIX, SOCK_STREAM, 0);
  if (sock < 0)
    {
      dbg_log (_("cannot open socket: %s"), strerror (errno));
      exit (1);
    }
  /* Bind a name to the socket.  */
  sock_addr.sun_family = AF_UNIX;
  strcpy (sock_addr.sun_path, _PATH_NSCDSOCKET);
  if (bind (sock, (struct sockaddr *) &sock_addr, sizeof (sock_addr)) < 0)
    {
      dbg_log ("%s: %s", _PATH_NSCDSOCKET, strerror (errno));
      exit (1);
    }

  /* Set permissions for the socket.  */
  chmod (_PATH_NSCDSOCKET, 0666);

  /* Set the socket up to accept connections.  */
  if (listen (sock, SOMAXCONN) < 0)
    {
      dbg_log (_("cannot enable socket to accept connections: %s"),
	       strerror (errno));
      exit (1);
    }
}


/* Close the connections.  */
void
close_sockets (void)
{
  close (sock);
}

static void
invalidate_cache (char *key)
{
  dbtype number;

  if (strcmp (key, "passwd") == 0)
    number = pwddb;
  else if (strcmp (key, "group") == 0)
    number = grpdb;
  else if (strcmp (key, "hosts") == 0)
    number = hstdb;
  else return;

  if (dbs[number].enabled)
    prune_cache (&dbs[number], LONG_MAX);
}


/* Handle new request.  */
static void
handle_request (int fd, request_header *req, void *key, uid_t uid)
{
  if (debug_level > 0)
    dbg_log (_("handle_request: request received (Version = %d)"),
	     req->version);

  if (req->version != NSCD_VERSION)
    {
      dbg_log (_("\
cannot handle old request version %d; current version is %d"),
	       req->version, NSCD_VERSION);
      return;
    }

  if (req->type >= GETPWBYNAME && req->type <= LASTDBREQ)
    {
      struct hashentry *cached;
      struct database *db = &dbs[serv2db[req->type]];

      if (debug_level > 0)
	{
	  if (req->type == GETHOSTBYADDR || req->type == GETHOSTBYADDRv6)
	    {
	      char buf[INET6_ADDRSTRLEN];

	      dbg_log ("\t%s (%s)", serv2str[req->type],
		       inet_ntop (req->type == GETHOSTBYADDR
				  ? AF_INET : AF_INET6,
				  key, buf, sizeof (buf)));
	    }
	  else
	    dbg_log ("\t%s (%s)", serv2str[req->type], (char *)key);
	}

      /* Is this service enabled?  */
      if (!db->enabled)
	{
	  /* No, sent the prepared record.  */
	  if (TEMP_FAILURE_RETRY (write (fd, db->disabled_iov->iov_base,
					 db->disabled_iov->iov_len))
	      != db->disabled_iov->iov_len)
	    {
	      /* We have problems sending the result.  */
	      char buf[256];
	      dbg_log (_("cannot write result: %s"),
		       strerror_r (errno, buf, sizeof (buf)));
	    }

	  return;
	}

      /* Be sure we can read the data.  */
      pthread_rwlock_rdlock (&db->lock);

      /* See whether we can handle it from the cache.  */
      cached = (struct hashentry *) cache_search (req->type, key, req->key_len,
						  db, uid);
      if (cached != NULL)
	{
	  /* Hurray it's in the cache.  */
	  if (TEMP_FAILURE_RETRY (write (fd, cached->packet, cached->total))
	      != cached->total)
	    {
	      /* We have problems sending the result.  */
	      char buf[256];
	      dbg_log (_("cannot write result: %s"),
		       strerror_r (errno, buf, sizeof (buf)));
	    }

	  pthread_rwlock_unlock (&db->lock);

	  return;
	}

      pthread_rwlock_unlock (&db->lock);
    }
  else if (debug_level > 0)
    {
      if (req->type == INVALIDATE)
	dbg_log ("\t%s (%s)", serv2str[req->type], (char *)key);
      else
	dbg_log ("\t%s", serv2str[req->type]);
    }

  /* Handle the request.  */
  switch (req->type)
    {
    case GETPWBYNAME:
      addpwbyname (&dbs[serv2db[req->type]], fd, req, key, uid);
      break;

    case GETPWBYUID:
      addpwbyuid (&dbs[serv2db[req->type]], fd, req, key, uid);
      break;

    case GETGRBYNAME:
      addgrbyname (&dbs[serv2db[req->type]], fd, req, key, uid);
      break;

    case GETGRBYGID:
      addgrbygid (&dbs[serv2db[req->type]], fd, req, key, uid);
      break;

    case GETHOSTBYNAME:
      addhstbyname (&dbs[serv2db[req->type]], fd, req, key, uid);
      break;

    case GETHOSTBYNAMEv6:
      addhstbynamev6 (&dbs[serv2db[req->type]], fd, req, key, uid);
      break;

    case GETHOSTBYADDR:
      addhstbyaddr (&dbs[serv2db[req->type]], fd, req, key, uid);
      break;

    case GETHOSTBYADDRv6:
      addhstbyaddrv6 (&dbs[serv2db[req->type]], fd, req, key, uid);
      break;

    case GETSTAT:
    case SHUTDOWN:
    case INVALIDATE:
      /* Accept shutdown, getstat and invalidate only from root */
      if (secure_in_use && uid == 0)
	{
	  if (req->type == GETSTAT)
	    send_stats (fd, dbs);
	  else if (req->type == INVALIDATE)
	    invalidate_cache (key);
	  else
	    termination_handler (0);
	}
      else
	{
	  struct ucred caller;
	  socklen_t optlen = sizeof (caller);

	  /* Some systems have no SO_PEERCRED implementation.  They don't
	     care about security so we don't as well.  */
#ifdef SO_PEERCRED
	  if (getsockopt (fd, SOL_SOCKET, SO_PEERCRED, &caller, &optlen) < 0)
	    {
	      char buf[256];

	      dbg_log (_("error getting callers id: %s"),
		       strerror_r (errno, buf, sizeof (buf)));
	    }
	  else
	    if (caller.uid == 0)
#endif
	      {
		if (req->type == GETSTAT)
		  send_stats (fd, dbs);
		else if (req->type == INVALIDATE)
		  invalidate_cache (key);
		else
		  termination_handler (0);
	      }
	}
      break;

    default:
      /* Ignore the command, it's nothing we know.  */
      break;
    }
}


/* This is the main loop.  It is replicated in different threads but the
   `poll' call makes sure only one thread handles an incoming connection.  */
static void *
__attribute__ ((__noreturn__))
nscd_run (void *p)
{
  long int my_number = (long int) p;
  struct pollfd conn;
  int run_prune = my_number < lastdb && dbs[my_number].enabled;
  time_t now = time (NULL);
  time_t next_prune = now + CACHE_PRUNE_INTERVAL;
  int timeout = run_prune ? 1000 * (next_prune - now) : -1;

  conn.fd = sock;
  conn.events = POLLRDNORM;

  while (1)
    {
      int nr = poll (&conn, 1, timeout);

      if (nr == 0)
	{
	  /* The `poll' call timed out.  It's time to clean up the cache.  */
	  assert (my_number < lastdb);
	  now = time (NULL);
	  prune_cache (&dbs[my_number], now);
	  next_prune = now + CACHE_PRUNE_INTERVAL;
	  timeout = 1000 * (next_prune - now);
	  continue;
	}

      /* We have a new incoming connection.  */
      if (conn.revents & (POLLRDNORM|POLLERR|POLLHUP|POLLNVAL))
	{
	  /* Accept the connection.  */
	  int fd = accept (conn.fd, NULL, NULL);
	  request_header req;
	  char buf[256];
	  uid_t uid = 0;

	  if (fd < 0)
	    {
	      dbg_log (_("while accepting connection: %s"),
		       strerror_r (errno, buf, sizeof (buf)));
	      continue;
	    }

	  /* Now read the request.  */
	  if (TEMP_FAILURE_RETRY (read (fd, &req, sizeof (req)))
	      != sizeof (req))
	    {
	      dbg_log (_("short read while reading request: %s"),
		       strerror_r (errno, buf, sizeof (buf)));
	      close (fd);
	      continue;
	    }

	  /* Some systems have no SO_PEERCRED implementation.  They don't
	     care about security so we don't as well.  */
#ifdef SO_PEERCRED
	  if (secure_in_use)
	    {
	      struct ucred caller;
	      socklen_t optlen = sizeof (caller);

	      if (getsockopt (fd, SOL_SOCKET, SO_PEERCRED,
			      &caller, &optlen) < 0)
		{
		  dbg_log (_("error getting callers id: %s"),
			   strerror_r (errno, buf, sizeof (buf)));
		  close (fd);
		  continue;
		}

	      if (req.type < GETPWBYNAME || req.type > LASTDBREQ
		  || secure[serv2db[req.type]])
		uid = caller.uid;
	    }
#endif

	  /* It should not be possible to crash the nscd with a silly
	     request (i.e., a terribly large key).  We limit the size
	     to 1kb.  */
	  if (req.key_len < 0 || req.key_len > 1024)
	    {
	      dbg_log (_("key length in request too long: %d"), req.key_len);
	      close (fd);
	      continue;
	    }
	  else
	    {
	      /* Get the key.  */
	      char keybuf[req.key_len];

	      if (TEMP_FAILURE_RETRY (read (fd, keybuf, req.key_len))
		  != req.key_len)
		{
		  dbg_log (_("short read while reading request key: %s"),
			   strerror_r (errno, buf, sizeof (buf)));
		  close (fd);
		  continue;
		}

	      /* Phew, we got all the data, now process it.  */
	      handle_request (fd, &req, keybuf, uid);

	      /* We are done.  */
	      close (fd);
	    }
	}

      if (run_prune)
	{
	  now = time (NULL);
	  timeout = now < next_prune ? 1000 * (next_prune - now) : 0;
	}
    }
}


/* Start all the threads we want.  The initial process is thread no. 1.  */
void
start_threads (void)
{
  long int i;
  pthread_attr_t attr;
  pthread_t th;

  pthread_attr_init (&attr);
  pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);

  /* We allow less than LASTDB threads only for debugging.  */
  if (debug_level == 0)
    nthreads = MAX (nthreads, lastdb);

  for (i = 1; i < nthreads; ++i)
    pthread_create (&th, &attr, nscd_run, (void *) i);

  pthread_attr_destroy (&attr);

  nscd_run ((void *) 0);
}
