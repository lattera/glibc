/* Inner loops of cache daemon.
   Copyright (C) 1998-2003, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1998.

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

#include <assert.h>
#include <atomic.h>
#include <error.h>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <pthread.h>
#include <pwd.h>
#include <resolv.h>
#include <stdio.h>
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

/* Wrapper functions with error checking for standard functions.  */
extern void *xmalloc (size_t n);
extern void *xcalloc (size_t n, size_t s);
extern void *xrealloc (void *o, size_t n);

/* Support to run nscd as an unprivileged user */
const char *server_user;
static uid_t server_uid;
static gid_t server_gid;
const char *stat_user;
uid_t stat_uid;
static gid_t *server_groups;
#ifndef NGROUPS
# define NGROUPS 32
#endif
static int server_ngroups = NGROUPS;

static void begin_drop_privileges (void);
static void finish_drop_privileges (void);


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
struct database dbs[lastdb] =
{
  [pwddb] = {
    .lock = PTHREAD_RWLOCK_WRITER_NONRECURSIVE_INITIALIZER_NP,
    .enabled = 0,
    .check_file = 1,
    .filename = "/etc/passwd",
    .module = 211,
    .disabled_iov = &pwd_iov_disabled,
    .postimeout = 3600,
    .negtimeout = 20
  },
  [grpdb] = {
    .lock = PTHREAD_RWLOCK_WRITER_NONRECURSIVE_INITIALIZER_NP,
    .enabled = 0,
    .check_file = 1,
    .filename = "/etc/group",
    .module = 211,
    .disabled_iov = &grp_iov_disabled,
    .postimeout = 3600,
    .negtimeout = 60
  },
  [hstdb] = {
    .lock = PTHREAD_RWLOCK_WRITER_NONRECURSIVE_INITIALIZER_NP,
    .enabled = 0,
    .check_file = 1,
    .filename = "/etc/hosts",
    .module = 211,
    .disabled_iov = &hst_iov_disabled,
    .postimeout = 3600,
    .negtimeout = 20
  }
};

/* Number of seconds between two cache pruning runs.  */
#define CACHE_PRUNE_INTERVAL	15

/* Number of threads to use.  */
int nthreads = -1;

/* Socket for incoming connections.  */
static int sock;

/* Number of times clients had to wait.  */
unsigned long int client_queued;


/* Initialize database information structures.  */
void
nscd_init (void)
{
  struct sockaddr_un sock_addr;
  size_t cnt;

  /* Secure mode and unprivileged mode are incompatible */
  if (server_user != NULL && secure_in_use)
    {
      dbg_log (_("Cannot run nscd in secure mode as unprivileged user"));
      exit (1);
    }

  /* Look up unprivileged uid/gid/groups before we start listening on the
     socket  */
  if (server_user != NULL)
    begin_drop_privileges ();

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
	  {
	    dbg_log (_("while allocating cache: %s"), strerror (errno));
	    exit (1);
	  }

	if (dbs[cnt].check_file)
	  {
	    /* We need the modification date of the file.  */
	    struct stat st;

	    if (stat (dbs[cnt].filename, &st) < 0)
	      {
		/* We cannot stat() the file, disable file checking.  */
		dbg_log (_("cannot stat() file `%s': %s"),
			 dbs[cnt].filename, strerror (errno));
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

  /* We don't wait for data otherwise races between threads can get
     them stuck on accept.  */
  int fl = fcntl (sock, F_GETFL);
  if (fl != -1)
    fcntl (sock, F_SETFL, fl | O_NONBLOCK);

  /* Set permissions for the socket.  */
  chmod (_PATH_NSCDSOCKET, 0666);

  /* Set the socket up to accept connections.  */
  if (listen (sock, SOMAXCONN) < 0)
    {
      dbg_log (_("cannot enable socket to accept connections: %s"),
	       strerror (errno));
      exit (1);
    }

  /* Change to unprivileged uid/gid/groups if specifed in config file */
  if (server_user != NULL)
    finish_drop_privileges ();
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
  else if (__builtin_expect (strcmp (key, "hosts"), 0) == 0)
    {
      number = hstdb;

      /* Re-initialize the resolver.  resolv.conf might have changed.  */
      res_init ();
    }
  else
    return;

  if (dbs[number].enabled)
    prune_cache (&dbs[number], LONG_MAX);
}


/* Handle new request.  */
static void
handle_request (int fd, request_header *req, void *key, uid_t uid)
{
  if (__builtin_expect (req->version, NSCD_VERSION) != NSCD_VERSION)
    {
      if (debug_level > 0)
	dbg_log (_("\
cannot handle old request version %d; current version is %d"),
		 req->version, NSCD_VERSION);
      return;
    }

  if (__builtin_expect (req->type, GETPWBYNAME) >= GETPWBYNAME
      && __builtin_expect (req->type, LASTDBREQ) <= LASTDBREQ)
    {
      struct hashentry *cached;
      struct database *db = &dbs[serv2db[req->type]];

      if (__builtin_expect (debug_level, 0) > 0)
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
	      != (ssize_t) db->disabled_iov->iov_len
	      && __builtin_expect (debug_level, 0) > 0)
	    {
	      /* We have problems sending the result.  */
	      char buf[256];
	      dbg_log (_("cannot write result: %s"),
		       strerror_r (errno, buf, sizeof (buf)));
	    }

	  return;
	}

      /* Be sure we can read the data.  */
      if (__builtin_expect (pthread_rwlock_tryrdlock (&db->lock) != 0, 0))
	{
	  ++db->rdlockdelayed;
	  pthread_rwlock_rdlock (&db->lock);
	}

      /* See whether we can handle it from the cache.  */
      cached = (struct hashentry *) cache_search (req->type, key, req->key_len,
						  db, uid);
      if (cached != NULL)
	{
	  /* Hurray it's in the cache.  */
	  if (TEMP_FAILURE_RETRY (write (fd, cached->packet, cached->total))
	      != cached->total
	      && __builtin_expect (debug_level, 0) > 0)
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
  else if (__builtin_expect (debug_level, 0) > 0)
    {
      if (req->type == INVALIDATE)
	dbg_log ("\t%s (%s)", serv2str[req->type], (char *)key);
      else if (req->type > LASTDBREQ && req->type < LASTREQ)
	dbg_log ("\t%s", serv2str[req->type]);
      else
	dbg_log (_("\tinvalid request type %d"), req->type);
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
      if (! secure_in_use)
	{
	  /* Get the callers credentials.  */
#ifdef SO_PEERCRED
	  struct ucred caller;
	  socklen_t optlen = sizeof (caller);

	  if (getsockopt (fd, SOL_SOCKET, SO_PEERCRED, &caller, &optlen) < 0)
	    {
	      char buf[256];

	      dbg_log (_("error getting callers id: %s"),
		       strerror_r (errno, buf, sizeof (buf)));
	      break;
	    }

	  uid = caller.uid;
#else
	  /* Some systems have no SO_PEERCRED implementation.  They don't
	     care about security so we don't as well.  */
	  uid = 0;
#endif
	}

      /* Accept shutdown, getstat and invalidate only from root.  For
	 the stat call also allow the user specified in the config file.  */
      if (req->type == GETSTAT)
	{
	  if (uid == 0 || uid == stat_uid)
	    send_stats (fd, dbs);
	}
      else if (uid == 0)
	{
	  if (req->type == INVALIDATE)
	    invalidate_cache (key);
	  else
	    termination_handler (0);
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
  time_t next_prune = run_prune ? time (NULL) + CACHE_PRUNE_INTERVAL : 0;
  static unsigned long int nready;

  conn.fd = sock;
  conn.events = POLLRDNORM;

  while (1)
    {
      int nr;
      time_t now = 0;

      /* One more thread available.  */
      atomic_increment (&nready);

    no_conn:
      do
	{
	  int timeout = -1;
	  if (run_prune)
	    {
	      now = time (NULL);
	      timeout = now < next_prune ? 1000 * (next_prune - now) : 0;
	    }

	  nr = poll (&conn, 1, timeout);

	  if (nr == 0)
	    {
	      /* The `poll' call timed out.  It's time to clean up the
		 cache.  */
	      atomic_decrement (&nready);
	      assert (my_number < lastdb);
	      prune_cache (&dbs[my_number], time(NULL));
	      now = time (NULL);
	      next_prune = now + CACHE_PRUNE_INTERVAL;
	      atomic_increment (&nready);
	      goto try_get;
	    }
	}
      while ((conn.revents & POLLRDNORM) == 0);

    got_data:;
      /* We have a new incoming connection.  Accept the connection.  */
      int fd = TEMP_FAILURE_RETRY (accept (conn.fd, NULL, NULL));
      request_header req;
      char buf[256];
      uid_t uid = -1;
#ifdef SO_PEERCRED
      pid_t pid = 0;
#endif

      if (__builtin_expect (fd, 0) < 0)
	{
	  if (errno != EAGAIN && errno != EWOULDBLOCK)
	    dbg_log (_("while accepting connection: %s"),
		     strerror_r (errno, buf, sizeof (buf)));
	  goto no_conn;
	}

      /* This thread is busy.  */
      atomic_decrement (&nready);

      /* Now read the request.  */
      if (__builtin_expect (TEMP_FAILURE_RETRY (read (fd, &req, sizeof (req)))
			    != sizeof (req), 0))
	{
	  if (debug_level > 0)
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

	  if (getsockopt (fd, SOL_SOCKET, SO_PEERCRED, &caller, &optlen) < 0)
	    {
	      dbg_log (_("error getting callers id: %s"),
		       strerror_r (errno, buf, sizeof (buf)));
	      close (fd);
	      continue;
	    }

	  if (req.type < GETPWBYNAME || req.type > LASTDBREQ
	      || secure[serv2db[req.type]])
	    uid = caller.uid;

	  pid = caller.pid;
	}
      else if (__builtin_expect (debug_level > 0, 0))
	{
	  struct ucred caller;
	  socklen_t optlen = sizeof (caller);

	  if (getsockopt (fd, SOL_SOCKET, SO_PEERCRED, &caller, &optlen) == 0)
	    pid = caller.pid;
	}
#endif

      /* It should not be possible to crash the nscd with a silly
	 request (i.e., a terribly large key).  We limit the size to 1kb.  */
      if (__builtin_expect (req.key_len, 1) < 0
	  || __builtin_expect (req.key_len, 1) > 1024)
	{
	  if (debug_level > 0)
	    dbg_log (_("key length in request too long: %d"), req.key_len);
	  close (fd);
	  continue;
	}
      else
	{
	  /* Get the key.  */
	  char keybuf[req.key_len];

	  if (__builtin_expect (TEMP_FAILURE_RETRY (read (fd, keybuf,
							  req.key_len))
				!= req.key_len, 0))
	    {
	      if (debug_level > 0)
		dbg_log (_("short read while reading request key: %s"),
			 strerror_r (errno, buf, sizeof (buf)));
	      close (fd);
	      continue;
	    }

	  if (__builtin_expect (debug_level, 0) > 0)
	    {
#ifdef SO_PEERCRED
	      if (pid != 0)
		dbg_log (_("\
handle_request: request received (Version = %d) from PID %ld"),
			 req.version, (long int) pid);
	      else
#endif
		dbg_log (_("\
handle_request: request received (Version = %d)"), req.version);
	    }

	  /* Phew, we got all the data, now process it.  */
	  handle_request (fd, &req, keybuf, uid);

	  /* We are done.  */
	  close (fd);
	}

      /* Just determine whether any data is present.  We do this to
	 measure whether clients are queued up.  */
    try_get:
      nr = poll (&conn, 1, 0);
      if (nr != 0)
	{
	  if (nready == 0)
	    ++client_queued;

	  atomic_increment (&nready);

	  goto got_data;
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


/* Look up the uid, gid, and supplementary groups to run nscd as. When
   this function is called, we are not listening on the nscd socket yet so
   we can just use the ordinary lookup functions without causing a lockup  */
static void
begin_drop_privileges (void)
{
  struct passwd *pwd;

  pwd = getpwnam (server_user);

  if (pwd == NULL)
    {
      dbg_log (_("Failed to run nscd as user '%s'"), server_user);
      error (EXIT_FAILURE, 0, _("Failed to run nscd as user '%s'"),
	     server_user);
    }

  server_uid = pwd->pw_uid;
  server_gid = pwd->pw_gid;

  server_groups = (gid_t *) xmalloc (server_ngroups * sizeof (gid_t));

  if (getgrouplist (server_user, server_gid, server_groups, &server_ngroups)
      == 0)
    return;

  server_groups = (gid_t *) xrealloc (server_groups,
				      server_ngroups * sizeof (gid_t));

  if (getgrouplist (server_user, server_gid, server_groups, &server_ngroups)
      == -1)
    {
      dbg_log (_("Failed to run nscd as user '%s'"), server_user);
      error (EXIT_FAILURE, errno, _("getgrouplist failed"));
    }
}


/* Call setgroups(), setgid(), and setuid() to drop root privileges and
   run nscd as the user specified in the configuration file.  */
static void
finish_drop_privileges (void)
{
  if (setgroups (server_ngroups, server_groups) == -1)
    {
      dbg_log (_("Failed to run nscd as user '%s'"), server_user);
      error (EXIT_FAILURE, errno, _("setgroups failed"));
    }

  if (setgid (server_gid) == -1)
    {
      dbg_log (_("Failed to run nscd as user '%s'"), server_user);
      perror ("setgid");
      exit (1);
    }

  if (setuid (server_uid) == -1)
    {
      dbg_log (_("Failed to run nscd as user '%s'"), server_user);
      perror ("setuid");
      exit (1);
    }
}
