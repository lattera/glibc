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
#include <libintl.h>
#include <pthread.h>
#include <pwd.h>
#include <resolv.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/mman.h>
#include <sys/param.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>

#include "nscd.h"
#include "dbg_log.h"
#include "selinux.h"


/* Number of bytes of data we initially reserve for each hash table bucket.  */
#define DEFAULT_DATASIZE_PER_BUCKET 1024


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
static int server_ngroups;

static void begin_drop_privileges (void);
static void finish_drop_privileges (void);

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
  [INVALIDATE] = "INVALIDATE",
  [GETFDPW] = "GETFDPW",
  [GETFDGR] = "GETFDGR",
  [GETFDHST] = "GETFDHST",
  [GETAI] = "GETAI"
};

/* The control data structures for the services.  */
struct database_dyn dbs[lastdb] =
{
  [pwddb] = {
    .lock = PTHREAD_RWLOCK_WRITER_NONRECURSIVE_INITIALIZER_NP,
    .enabled = 0,
    .check_file = 1,
    .persistent = 0,
    .shared = 0,
    .filename = "/etc/passwd",
    .db_filename = _PATH_NSCD_PASSWD_DB,
    .disabled_iov = &pwd_iov_disabled,
    .postimeout = 3600,
    .negtimeout = 20,
    .wr_fd = -1,
    .ro_fd = -1,
    .mmap_used = false
  },
  [grpdb] = {
    .lock = PTHREAD_RWLOCK_WRITER_NONRECURSIVE_INITIALIZER_NP,
    .enabled = 0,
    .check_file = 1,
    .persistent = 0,
    .shared = 0,
    .filename = "/etc/group",
    .db_filename = _PATH_NSCD_GROUP_DB,
    .disabled_iov = &grp_iov_disabled,
    .postimeout = 3600,
    .negtimeout = 60,
    .wr_fd = -1,
    .ro_fd = -1,
    .mmap_used = false
  },
  [hstdb] = {
    .lock = PTHREAD_RWLOCK_WRITER_NONRECURSIVE_INITIALIZER_NP,
    .enabled = 0,
    .check_file = 1,
    .persistent = 0,
    .shared = 0,
    .filename = "/etc/hosts",
    .db_filename = _PATH_NSCD_HOSTS_DB,
    .disabled_iov = &hst_iov_disabled,
    .postimeout = 3600,
    .negtimeout = 20,
    .wr_fd = -1,
    .ro_fd = -1,
    .mmap_used = false
  }
};


/* Mapping of request type to database.  */
static struct database_dyn *const serv2db[LASTREQ] =
{
  [GETPWBYNAME] = &dbs[pwddb],
  [GETPWBYUID] = &dbs[pwddb],
  [GETGRBYNAME] = &dbs[grpdb],
  [GETGRBYGID] = &dbs[grpdb],
  [GETHOSTBYNAME] = &dbs[hstdb],
  [GETHOSTBYNAMEv6] = &dbs[hstdb],
  [GETHOSTBYADDR] = &dbs[hstdb],
  [GETHOSTBYADDRv6] = &dbs[hstdb],
  [GETFDPW] = &dbs[pwddb],
  [GETFDGR] = &dbs[grpdb],
  [GETFDHST] = &dbs[hstdb],
  [GETAI] = &dbs[hstdb],
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
	pthread_mutex_init (&dbs[cnt].memlock, NULL);

	if (dbs[cnt].persistent)
	  {
	    /* Try to open the appropriate file on disk.  */
	    int fd = open (dbs[cnt].db_filename, O_RDWR);
	    if (fd != -1)
	      {
		struct stat64 st;
		void *mem;
		size_t total;
		struct database_pers_head head;
		ssize_t n = TEMP_FAILURE_RETRY (read (fd, &head,
						      sizeof (head)));
		if (n != sizeof (head) || fstat64 (fd, &st) != 0)
		  {
		  fail_db:
		    dbg_log (_("invalid persistent database file \"%s\": %s"),
			     dbs[cnt].db_filename, strerror (errno));
		    dbs[cnt].persistent = 0;
		  }
		else if (head.module == 0 && head.data_size == 0)
		  {
		    /* The file has been created, but the head has not been
		       initialized yet.  Remove the old file.  */
		    unlink (dbs[cnt].db_filename);
		  }
		else if (head.header_size != (int) sizeof (head))
		  {
		    dbg_log (_("invalid persistent database file \"%s\": %s"),
			     dbs[cnt].db_filename,
			     _("header size does not match"));
		    dbs[cnt].persistent = 0;
		  }
		else if ((total = (sizeof (head)
				   + roundup (head.module * sizeof (ref_t),
					      ALIGN)
				   + head.data_size))
			 > st.st_size)
		  {
		    dbg_log (_("invalid persistent database file \"%s\": %s"),
			     dbs[cnt].db_filename,
			     _("file size does not match"));
		    dbs[cnt].persistent = 0;
		  }
		else if ((mem = mmap (NULL, total, PROT_READ | PROT_WRITE,
				      MAP_SHARED, fd, 0)) == MAP_FAILED)
		  goto fail_db;
		else
		  {
		    /* Success.  We have the database.  */
		    dbs[cnt].head = mem;
		    dbs[cnt].memsize = total;
		    dbs[cnt].data = (char *)
		      &dbs[cnt].head->array[roundup (dbs[cnt].head->module,
						     ALIGN / sizeof (ref_t))];
		    dbs[cnt].mmap_used = true;

		    if (dbs[cnt].suggested_module > head.module)
		      dbg_log (_("suggested size of table for database %s larger than the persistent database's table"),
			       dbnames[cnt]);

		    dbs[cnt].wr_fd = fd;
		    fd = -1;
		    /* We also need a read-only descriptor.  */
		    if (dbs[cnt].shared)
		      {
			dbs[cnt].ro_fd = open (dbs[cnt].db_filename, O_RDONLY);
			if (dbs[cnt].ro_fd == -1)
			  dbg_log (_("\
cannot create read-only descriptor for \"%s\"; no mmap"),
				   dbs[cnt].db_filename);
		      }

		    // XXX Shall we test whether the descriptors actually
		    // XXX point to the same file?
		  }

		/* Close the file descriptors in case something went
		   wrong in which case the variable have not been
		   assigned -1.  */
		if (fd != -1)
		  close (fd);
	      }
	  }

	if (dbs[cnt].head == NULL)
	  {
	    /* No database loaded.  Allocate the data structure,
	       possibly on disk.  */
	    struct database_pers_head head;
	    size_t total = (sizeof (head)
			    + roundup (dbs[cnt].suggested_module
				       * sizeof (ref_t), ALIGN)
			    + (dbs[cnt].suggested_module
			       * DEFAULT_DATASIZE_PER_BUCKET));

	    /* Try to create the database.  If we do not need a
	       persistent database create a temporary file.  */
	    int fd;
	    int ro_fd = -1;
	    if (dbs[cnt].persistent)
	      {
		fd = open (dbs[cnt].db_filename,
			   O_RDWR | O_CREAT | O_EXCL | O_TRUNC,
			   S_IRUSR | S_IWUSR);
		if (fd != -1 && dbs[cnt].shared)
		  ro_fd = open (dbs[cnt].db_filename, O_RDONLY);
	      }
	    else
	      {
		size_t slen = strlen (dbs[cnt].db_filename);
		char fname[slen + 8];
		strcpy (mempcpy (fname, dbs[cnt].db_filename, slen),
			".XXXXXX");
		fd = mkstemp (fname);

		/* We do not need the file name anymore after we
		   opened another file descriptor in read-only mode.  */
		if (fd != -1 && dbs[cnt].shared)
		  {
		    ro_fd = open (fname, O_RDONLY);

		    unlink (fname);
		  }
	      }

	    if (fd == -1)
	      {
		if (errno == EEXIST)
		  {
		    dbg_log (_("database for %s corrupted or simultaneously used; remove %s manually if necessary and restart"),
			     dbnames[cnt], dbs[cnt].db_filename);
		    // XXX Correct way to terminate?
		    exit (1);
		  }

		if  (dbs[cnt].persistent)
		  dbg_log (_("cannot create %s; no persistent database used"),
			   dbs[cnt].db_filename);
		else
		  dbg_log (_("cannot create %s; no sharing possible"),
			   dbs[cnt].db_filename);

		dbs[cnt].persistent = 0;
		// XXX remember: no mmap
	      }
	    else
	      {
		/* Tell the user if we could not create the read-only
		   descriptor.  */
		if (ro_fd == -1 && dbs[cnt].shared)
		  dbg_log (_("\
cannot create read-only descriptor for \"%s\"; no mmap"),
			   dbs[cnt].db_filename);

		/* Before we create the header, initialiye the hash
		   table.  So that if we get interrupted if writing
		   the header we can recognize a partially initialized
		   database.  */
		size_t ps = sysconf (_SC_PAGESIZE);
		char tmpbuf[ps];
		assert (~ENDREF == 0);
		memset (tmpbuf, '\xff', ps);

		size_t remaining = dbs[cnt].suggested_module * sizeof (ref_t);
		off_t offset = sizeof (head);

		size_t towrite;
		if (offset % ps != 0)
		  {
		    towrite = MIN (remaining, ps - (offset % ps));
		    pwrite (fd, tmpbuf, towrite, offset);
		    offset += towrite;
		    remaining -= towrite;
		  }

		while (remaining > ps)
		  {
		    pwrite (fd, tmpbuf, ps, offset);
		    offset += ps;
		    remaining -= ps;
		  }

		if (remaining > 0)
		  pwrite (fd, tmpbuf, remaining, offset);

		/* Create the header of the file.  */
		struct database_pers_head head =
		  {
		    .version = DB_VERSION,
		    .header_size = sizeof (head),
		    .module = dbs[cnt].suggested_module,
		    .data_size = (dbs[cnt].suggested_module
				  * DEFAULT_DATASIZE_PER_BUCKET),
		    .first_free = 0
		  };
		void *mem;

		if ((TEMP_FAILURE_RETRY (write (fd, &head, sizeof (head)))
		     != sizeof (head))
		    || ftruncate (fd, total) != 0
		    || (mem = mmap (NULL, total, PROT_READ | PROT_WRITE,
				    MAP_SHARED, fd, 0)) == MAP_FAILED)
		  {
		    unlink (dbs[cnt].db_filename);
		    dbg_log (_("cannot write to database file %s: %s"),
			     dbs[cnt].db_filename, strerror (errno));
		    dbs[cnt].persistent = 0;
		  }
		else
		  {
		    /* Success.  */
		    dbs[cnt].head = mem;
		    dbs[cnt].data = (char *)
		      &dbs[cnt].head->array[roundup (dbs[cnt].head->module,
						     ALIGN / sizeof (ref_t))];
		    dbs[cnt].memsize = total;
		    dbs[cnt].mmap_used = true;

		    /* Remember the descriptors.  */
		    dbs[cnt].wr_fd = fd;
		    dbs[cnt].ro_fd = ro_fd;
		    fd = -1;
		    ro_fd = -1;
		  }

		if (fd != -1)
		  close (fd);
		if (ro_fd != -1)
		  close (ro_fd);
	      }
	  }

	if (dbs[cnt].head == NULL)
	  {
	    /* We do not use the persistent database.  Just
	       create an in-memory data structure.  */
	    assert (! dbs[cnt].persistent);

	    dbs[cnt].head = xmalloc (sizeof (struct database_pers_head)
				     + (dbs[cnt].suggested_module
					* sizeof (ref_t)));
	    memset (dbs[cnt].head, '\0', sizeof (dbs[cnt].head));
	    assert (~ENDREF == 0);
	    memset (dbs[cnt].head->array, '\xff',
		    dbs[cnt].suggested_module * sizeof (ref_t));
	    dbs[cnt].head->module = dbs[cnt].suggested_module;
	    dbs[cnt].head->data_size = (DEFAULT_DATASIZE_PER_BUCKET
					* dbs[cnt].head->module);
	    dbs[cnt].data = xmalloc (dbs[cnt].head->data_size);
	    dbs[cnt].head->first_free = 0;

	    dbs[cnt].shared = 0;
	    assert (dbs[cnt].ro_fd == -1);
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
  chmod (_PATH_NSCDSOCKET, DEFFILEMODE);

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


#ifdef SCM_RIGHTS
static void
send_ro_fd (struct database_dyn *db, char *key, int fd)
{
  /* If we do not have an read-only file descriptor do nothing.  */
  if (db->ro_fd == -1)
    return;

  /* We need to send some data along with the descriptor.  */
  struct iovec iov[1];
  iov[0].iov_base = key;
  iov[0].iov_len = strlen (key) + 1;

  /* Prepare the control message to transfer the descriptor.  */
  char buf[CMSG_SPACE (sizeof (int))];
  struct msghdr msg = { .msg_iov = iov, .msg_iovlen = 1,
			.msg_control = buf, .msg_controllen = sizeof (buf) };
  struct cmsghdr *cmsg = CMSG_FIRSTHDR (&msg);

  cmsg->cmsg_level = SOL_SOCKET;
  cmsg->cmsg_type = SCM_RIGHTS;
  cmsg->cmsg_len = CMSG_LEN (sizeof (int));

  *(int *) CMSG_DATA (cmsg) = db->ro_fd;

  msg.msg_controllen = cmsg->cmsg_len;

  /* Send the control message.  We repeat when we are interrupted but
     everything else is ignored.  */
  (void) TEMP_FAILURE_RETRY (sendmsg (fd, &msg, 0));

  if (__builtin_expect (debug_level > 0, 0))
    dbg_log (_("provide access to FD %d, for %s"), db->ro_fd, key);
}
#endif	/* SCM_RIGHTS */


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

  /* Make the SELinux check before we go on to the standard checks.  We
     need to verify that the request type is valid, since it has not
     yet been checked at this point.  */
  if (selinux_enabled
      && __builtin_expect (req->type, GETPWBYNAME) >= GETPWBYNAME
      && __builtin_expect (req->type, LASTREQ) < LASTREQ
      && nscd_request_avc_has_perm (fd, req->type) != 0)
    return;

  struct database_dyn *db = serv2db[req->type];

  if ((__builtin_expect (req->type, GETPWBYNAME) >= GETPWBYNAME
       && __builtin_expect (req->type, LASTDBREQ) <= LASTDBREQ)
      || req->type == GETAI)
    {
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
	    dbg_log ("\t%s (%s)", serv2str[req->type], (char *) key);
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
	  ++db->head->rdlockdelayed;
	  pthread_rwlock_rdlock (&db->lock);
	}

      /* See whether we can handle it from the cache.  */
      struct datahead *cached;
      cached = (struct datahead *) cache_search (req->type, key, req->key_len,
						 db, uid);
      if (cached != NULL)
	{
	  /* Hurray it's in the cache.  */
	  if (TEMP_FAILURE_RETRY (write (fd, cached->data, cached->recsize))
	      != cached->recsize
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
	dbg_log ("\t%s (%s)", serv2str[req->type], (char *) key);
      else
	dbg_log ("\t%s", serv2str[req->type]);
    }

  /* Handle the request.  */
  switch (req->type)
    {
    case GETPWBYNAME:
      addpwbyname (db, fd, req, key, uid);
      break;

    case GETPWBYUID:
      addpwbyuid (db, fd, req, key, uid);
      break;

    case GETGRBYNAME:
      addgrbyname (db, fd, req, key, uid);
      break;

    case GETGRBYGID:
      addgrbygid (db, fd, req, key, uid);
      break;

    case GETHOSTBYNAME:
      addhstbyname (db, fd, req, key, uid);
      break;

    case GETHOSTBYNAMEv6:
      addhstbynamev6 (db, fd, req, key, uid);
      break;

    case GETHOSTBYADDR:
      addhstbyaddr (db, fd, req, key, uid);
      break;

    case GETHOSTBYADDRv6:
      addhstbyaddrv6 (db, fd, req, key, uid);
      break;

    case GETAI:
      addhstai (db, fd, req, key, uid);
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

    case GETFDPW:
    case GETFDGR:
    case GETFDHST:
#ifdef SCM_RIGHTS
      send_ro_fd (serv2db[req->type], key, fd);
#endif
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

  if (run_prune)
    setup_thread (&dbs[my_number]);

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
	      /* NB: we do not flush the timestamp update using msync since
		 this value doesnot matter on disk.  */
	      dbs[my_number].head->timestamp = now = time (NULL);
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

      /* Check whether this is a valid request type.  */
      if (req.type < GETPWBYNAME || req.type >= LASTREQ)
	goto close_and_out;

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
	      goto close_and_out;
	    }

	  if (req.type < GETPWBYNAME || req.type > LASTDBREQ
	      || serv2db[req.type]->secure)
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
	}

    close_and_out:
      /* We are done.  */
      close (fd);

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
  struct passwd *pwd = getpwnam (server_user);

  if (pwd == NULL)
    {
      dbg_log (_("Failed to run nscd as user '%s'"), server_user);
      error (EXIT_FAILURE, 0, _("Failed to run nscd as user '%s'"),
	     server_user);
    }

  server_uid = pwd->pw_uid;
  server_gid = pwd->pw_gid;

  if (getgrouplist (server_user, server_gid, NULL, &server_ngroups) == 0)
    {
      /* This really must never happen.  */
      dbg_log (_("Failed to run nscd as user '%s'"), server_user);
      error (EXIT_FAILURE, errno, _("initial getgrouplist failed"));
    }

  server_groups = (gid_t *) xmalloc (server_ngroups * sizeof (gid_t));

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
