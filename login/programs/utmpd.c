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

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <libintl.h>
#include <locale.h>
#include <pwd.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/param.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <syslog.h>
#include <unistd.h>

#include "utmpd.h"
#include "utmpd-private.h"

#ifndef DEFAULT_USER
#define DEFAULT_USER	"daemon"
#endif

/* Get libc version number.  */
#include <version.h>

#define PACKAGE _libc_intl_domainname

/* Long options.  */
static const struct option long_options[] =
{
  { "debug", no_argument, NULL, 'd' },
  { "help", no_argument, NULL, 'h' },
  { "version", no_argument, NULL, 'V' },
  { NULL, 0, NULL, 0}
};

/* The UTMP database.  */
utmp_database *utmp_db;

/* The socket for read only requests.  */
int ro_sock = -1;

/* The socket for read/write requests.  */
int rw_sock = -1;


/* Prototypes for the local functions.  */
static void usage (int status) __attribute__ ((noreturn));
static void drop_priviliges (void);
static int make_socket (const char *name);
static void handle_requests (void) __attribute__ ((noreturn));
static void termination_handler (int signum);
static int check_pid (const char *file);
static int write_pid (const char *file);


int
main (int argc, char *argv[])
{
  mode_t mask;
  int debug;
  int do_help;
  int do_version;
  int opt;

  /* Set locale via LC_ALL.  */
  setlocale (LC_ALL, "");

  /* Set the text message domain.  */
  textdomain (PACKAGE);

  /* Initialize local variables.  */
  debug = 0;
  do_help = 0;
  do_version = 0;

  while ((opt = getopt_long (argc, argv, "dhV", long_options, NULL)) != -1)
    switch (opt)
      {
      case '\0':		/* Long option.  */
	break;
      case 'h':
	do_help = 1;
	break;
      case 'd':
	debug = 1;
	break;
      case 'V':
	do_version = 1;
	break;
      default:
	usage (EXIT_FAILURE);
      }

  /* Version information is reequested.  */
  if (do_version)
    {
      printf ("utmpd (GNU %s) %s\n", PACKAGE, VERSION);
      printf (_("\
Copyright (C) %s Free Software Foundation, Inc.\n\
This is free software; see the source for copying conditions.  There is NO\n\
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n\
"), "1997");
      printf (_("Written by %s.\n"), "Mark Kettenis");

      exit (EXIT_SUCCESS);
    }

  /* Help is requested.  */
  if (do_help)
    usage (EXIT_SUCCESS);

  signal (SIGINT, termination_handler);
  signal (SIGQUIT, termination_handler);
  signal (SIGTERM, termination_handler);

  /* Check if we are already running.  */
  if (check_pid (_PATH_UTMPDPID))
    error (EXIT_FAILURE, 0, _("already running"));

  /* Cleanup files created by a previous `bind'.  */
  unlink (_PATH_UTMPD_RO);
  unlink (_PATH_UTMPD_RW);

  /* Open UTMP database.  */
  utmp_db = open_database (_PATH_UTMP "x", _PATH_UTMP);
  if (utmp_db == NULL)
    exit (EXIT_FAILURE);

  /* Create sockets, with the right permissions.  */
  mask = umask (S_IXUSR | S_IXGRP | S_IXOTH);
  ro_sock = make_socket (_PATH_UTMPD_RO);
  umask (S_IXUSR | S_IRWXG | S_IRWXO);
  rw_sock = make_socket (_PATH_UTMPD_RW);
  umask (mask);

  /* Set the sockets up to accept connections.  */
  if (listen (ro_sock, MAX_CONNECTIONS) < 0
      || listen (rw_sock, MAX_CONNECTIONS) < 0)
    error (EXIT_FAILURE, errno,
	   _("cannot enable socket to accept connections"));

  /* Behave like a daemon.  */
  if (!debug)
    {
      openlog ("utmpd", LOG_CONS | LOG_ODELAY, LOG_DAEMON);

      if (daemon (0, 0) < 0)
	error (EXIT_FAILURE, errno, _("cannot auto-background"));
      forked = 1;

      if (write_pid (_PATH_UTMPDPID) < 0)
	warning (errno, "%s", _PATH_UTMPDPID);

      /* Ignore job control signals.  */
      signal (SIGTTOU, SIG_IGN);
      signal (SIGTTIN, SIG_IGN);
      signal (SIGTSTP, SIG_IGN);
    }

  /* Drop priviliges.  */
  drop_priviliges ();

  /* Handle incoming requests.  */
  handle_requests ();
}


/* Display usage information and exit.  */
static void
usage (int status)
{
  if (status != EXIT_SUCCESS)
    fprintf (stderr, _("Try `%s --help' for more information.\n"),
	     program_invocation_name);
  else
    {
      printf (_("\
Usage: %s [OPTION]...\n\
  -d, --debug           do not fork and display messages on the current tty\n\
  -h, --help	        display this help and exit\n\
  -V, --version         output version information and exit\n"),
	      program_invocation_name);
      fputs (_("\
Report bugs using the `glibcbug' script to <bugs@gnu.org>.\n"),
	     stdout);
    }

  exit (status);
}


/* Drop priviliges.  */
static void
drop_priviliges (void)
{
  struct passwd *pw;

  pw = getpwnam (DEFAULT_USER);
  if (pw)
    {
      seteuid (pw->pw_uid);
      setegid (pw->pw_gid);
    }
}


/* Make a socket in the file namespace using the filename NAME as the
   socket's address.  */
static int
make_socket (const char *name)
{
  struct sockaddr_un addr;
  size_t size;
  int sock;

  /* Create the socket.  */
  sock = socket (PF_UNIX, SOCK_STREAM, 0);
  if (sock < 0)
    error (EXIT_FAILURE, errno, _("cannot create socket"));

  /* Bind a name to the socket.  */
  addr.sun_family = AF_UNIX;
  strcpy (addr.sun_path, name);

  /* The size of the address is the offset of the start
     of the filename, plus its length, plus one for the
     terminating null byte. */
  size = (offsetof (struct sockaddr_un, sun_path)
	  + strlen (addr.sun_path));


  if (bind (sock, (struct sockaddr *) &addr, size) < 0)
    error (EXIT_FAILURE, errno, "%s", name);

  return sock;
}


/* Hanlde incoming requests.  */
static
void handle_requests (void)
{
  client_connection *connection;
  fd_set active_read_fd_set;
  fd_set active_write_fd_set;
  fd_set read_fd_set;
  fd_set write_fd_set;
  int fd;
  int maxfd;  /* Highest used fd to optimize select/loop.  */

  /* Initialize the set of active sockets.  */
  FD_ZERO (&active_read_fd_set);
  FD_ZERO (&active_write_fd_set);
  FD_SET (rw_sock, &active_read_fd_set);
  FD_SET (ro_sock, &active_read_fd_set);

  maxfd = MAX (rw_sock, ro_sock);

  while (1)
    {
      /* Block until input arrives on one or more active sockets.  */
      read_fd_set = active_read_fd_set;
      write_fd_set = active_write_fd_set;
      if (select (maxfd + 1, &read_fd_set, &write_fd_set, NULL, NULL) < 0)
	error (EXIT_FAILURE, errno, _("cannot get input on sockets"));

      /* Service all the sockets with input pending.  */
      for (fd = 0; fd <= maxfd; ++fd)
	{
	  if (FD_ISSET (fd, &read_fd_set))
	    {
	      if (fd == ro_sock || fd == rw_sock)
		{
		  int access = ((fd == rw_sock) ? (R_OK | W_OK) : R_OK);

		  connection = accept_connection (fd, access);
		  if (connection == NULL)
		    error (0, errno, _("cannot accept connection"));

		  FD_SET (connection->sock, &active_read_fd_set);
		  maxfd = MAX (maxfd, connection->sock);
		}
	      else
		{
		  connection = find_connection (fd);
		  if (connection == NULL)
		    error (EXIT_FAILURE, 0, _("cannot find connection"));

		  if (read_data (connection) < 0)
		    {
		      close_connection (connection);
		      FD_CLR (fd, &active_read_fd_set);
		      FD_CLR (fd, &active_write_fd_set);
		    }

		  if (connection->write_ptr > connection->write_base)
		      FD_SET (fd, &active_write_fd_set);
		}
	    }
	  if (FD_ISSET (fd, &write_fd_set) &&
	      fd != rw_sock && fd != ro_sock)
	    {
	      connection = find_connection (fd);
	      if (connection == NULL)
		error (EXIT_FAILURE, 0, _("cannot find connection"));

	      if (write_data (connection) < 0)
		{
		  close_connection (connection);
		  FD_CLR (fd, &active_read_fd_set);
		  FD_CLR (fd, &active_write_fd_set);
		}

	      if (connection->write_ptr == connection->write_base)
		FD_CLR (fd, &active_write_fd_set);
	    }
	}

      /* Check if maxfd can be lowered.  */
      for (; maxfd >= 0; --maxfd)
	{
	  if (FD_ISSET (maxfd, &active_read_fd_set)
	      || FD_ISSET (maxfd, &active_write_fd_set))
	    break;
	}
    }
}


/* Cleanup.  */
static void
termination_handler (int signum)
{
  /* Close sockets.  */
  close (ro_sock);
  close (rw_sock);

  /* Restore user id.  */
  seteuid (getuid ());

  /* Clean up the files created by `bind'.  */
  unlink (_PATH_UTMPD_RO);
  unlink (_PATH_UTMPD_RW);

  if (utmp_db)
    close_database (utmp_db);

  /* Clean up pid file.  */
  unlink (_PATH_UTMPDPID);

  exit (EXIT_SUCCESS);
}


/* Returns 1 if the process in pid file FILE is running, 0 if not.  */
static int
check_pid (const char *file)
{
  FILE *fp;

  fp = fopen (_PATH_UTMPDPID, "r");
  if (fp)
    {
      pid_t pid;

      fscanf (fp, "%d", &pid);
      fclose (fp);

      if (kill (pid, 0) == 0)
	return 1;
    }

  return 0;
}

/* Write the current process id to the file FILE.  Returns 0 if
   successful, -1 if not.  */
static int
write_pid (const char *file)
{
  FILE *fp;

  fp = fopen (_PATH_UTMPDPID, "w");
  if (fp == NULL)
    return -1;

  fprintf (fp, "%d\n", getpid ());
  if (ferror (fp))
    return -1;

  fclose (fp);

  return 0;
}
