/* Copyright (c) 1998, 1999 Free Software Foundation, Inc.
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

/* nscd - Name Service Cache Daemon. Caches passwd, group, and hosts.  */

#include <argp.h>
#include <assert.h>
#include <errno.h>
#include <error.h>
#include <libintl.h>
#include <locale.h>
#include <pthread.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "dbg_log.h"
#include "nscd.h"

/* Get libc version number.  */
#include <version.h>

#define PACKAGE _libc_intl_domainname

/* Structure used by main() thread to keep track of the number of
   active threads.  Used to limit how many threads it will create
   and under a shutdown condition to wait till all in-progress
   requests have finished before "turning off the lights".  */

typedef struct
{
  int             num_active;
  pthread_cond_t  thread_exit_cv;
  pthread_mutex_t mutex;
} thread_info_t;

thread_info_t thread_info;

int do_shutdown = 0;
int disabled_passwd = 0;
int disabled_group = 0;
int go_background = 1;
static const char *conffile = _PATH_NSCDCONF;

static int check_pid (const char *file);
static int write_pid (const char *file);

/* Name and version of program.  */
static void print_version (FILE *stream, struct argp_state *state);
void (*argp_program_version_hook) (FILE *, struct argp_state *) = print_version;

/* Definitions of arguments for argp functions.  */
static const struct argp_option options[] =
{
  { "config-file", 'f', N_("NAME"), 0,
    N_("Read configuration data from NAME") },
  { "debug", 'd', NULL, 0,
    N_("Do not fork and display messages on the current tty") },
  { "nthreads", 't', N_("NUMBER"), 0, N_("Start NUMBER threads") },
  { "shutdown", 'K', NULL, 0, N_("Shut the server down") },
  { "statistic", 'g', NULL, 0, N_("Print current configuration statistic") },
  { NULL, 0, NULL, 0, NULL }
};

/* Short description of program.  */
static const char doc[] = N_("Name Service Cache Daemon.");

/* Prototype for option handler.  */
static error_t parse_opt __P ((int key, char *arg, struct argp_state *state));

/* Data structure to communicate with argp functions.  */
static struct argp argp =
{
  options, parse_opt, NULL, doc,
};

int
main (int argc, char **argv)
{
  int remaining;

  /* Set locale via LC_ALL.  */
  setlocale (LC_ALL, "");
  /* Set the text message domain.  */
  textdomain (PACKAGE);

  /* Parse and process arguments.  */
  argp_parse (&argp, argc, argv, 0, &remaining, NULL);

  if (remaining != argc)
    {
      error (0, 0, gettext ("wrong number of arguments"));
      argp_help (&argp, stdout, ARGP_HELP_SEE, program_invocation_short_name);
      exit (EXIT_FAILURE);
    }

  /* Check if we are already running. */
  if (check_pid (_PATH_NSCDPID))
    error (EXIT_FAILURE, 0, _("already running"));

  /* Behave like a daemon.  */
  if (go_background)
    {
      int i;

      if (fork ())
	exit (0);

      for (i = 0; i < getdtablesize (); i++)
	close (i);

      if (fork ())
	exit (0);

      chdir ("/");

      openlog ("nscd", LOG_CONS | LOG_ODELAY, LOG_DAEMON);

      if (write_pid (_PATH_NSCDPID) < 0)
        dbg_log ("%s: %s", _PATH_NSCDPID, strerror (errno));

      /* Ignore job control signals.  */
      signal (SIGTTOU, SIG_IGN);
      signal (SIGTTIN, SIG_IGN);
      signal (SIGTSTP, SIG_IGN);
    }

  signal (SIGINT, termination_handler);
  signal (SIGQUIT, termination_handler);
  signal (SIGTERM, termination_handler);
  signal (SIGPIPE, SIG_IGN);

  /* Cleanup files created by a previous `bind'.  */
  unlink (_PATH_NSCDSOCKET);

  /* Init databases.  */
  nscd_init (conffile);

  /* Handle incoming requests */
  start_threads ();

  return 0;
}


/* Handle program arguments.  */
static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
  switch (key)
    {
    case 'd':
      ++debug_level;
      go_background = 0;
      break;

    case 'f':
      conffile = arg;
      break;

    case 'K':
      if (getuid () != 0)
	error (EXIT_FAILURE, 0, _("Only root is allowed to use this option!"));
      {
	int sock = nscd_open_socket ();
	request_header req;
	ssize_t nbytes;

	if (sock == -1)
	  exit (EXIT_FAILURE);

	req.version = NSCD_VERSION;
	req.type = SHUTDOWN;
	req.key_len = 0;
	nbytes = TEMP_FAILURE_RETRY (write (sock, &req,
					    sizeof (request_header)));
	close (sock);
	exit (nbytes != sizeof (request_header) ? EXIT_FAILURE : EXIT_SUCCESS);
      }

    case 'g':
      receive_print_stats ();
      /* Does not return.  */

    case 't':
      nthreads = atol (arg);
      break;

    default:
      return ARGP_ERR_UNKNOWN;
    }

  return 0;
}

/* Print the version information.  */
static void
print_version (FILE *stream, struct argp_state *state)
{
  fprintf (stream, "nscd (GNU %s) %s\n", PACKAGE, VERSION);
  fprintf (stream, gettext ("\
Copyright (C) %s Free Software Foundation, Inc.\n\
This is free software; see the source for copying conditions.  There is NO\n\
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n\
"), "1999");
  fprintf (stream, gettext ("Written by %s.\n"),
	   "Thorsten Kukuk and Ulrich Drepper");
}


/* Create a socket connected to a name.  */
int
nscd_open_socket (void)
{
  struct sockaddr_un addr;
  int sock;

  sock = socket (PF_UNIX, SOCK_STREAM, 0);
  if (sock < 0)
    return -1;

  addr.sun_family = AF_UNIX;
  assert (sizeof (addr.sun_path) >= sizeof (_PATH_NSCDSOCKET));
  strcpy (addr.sun_path, _PATH_NSCDSOCKET);
  if (connect (sock, (struct sockaddr *) &addr, sizeof (addr)) < 0)
    {
      close (sock);
      return -1;
    }

  return sock;
}

/* Cleanup.  */
void
termination_handler (int signum)
{
  close_sockets ();

  /* Clean up the file created by `bind'.  */
  unlink (_PATH_NSCDSOCKET);

  /* Clean up pid file.  */
  unlink (_PATH_NSCDPID);

  exit (EXIT_SUCCESS);
}

/* Returns 1 if the process in pid file FILE is running, 0 if not.  */
static int
check_pid (const char *file)
{
  FILE *fp;

  fp = fopen (file, "r");
  if (fp)
    {
      pid_t pid;
      int n;

      n = fscanf (fp, "%d", &pid);
      fclose (fp);

      if (n != 1 || kill (pid, 0) == 0)
        return 1;
    }

  return 0;
}

/* Write the current process id to the file FILE.
   Returns 0 if successful, -1 if not.  */
static int
write_pid (const char *file)
{
  FILE *fp;

  fp = fopen (file, "w");
  if (fp == NULL)
    return -1;

  fprintf (fp, "%d\n", getpid ());
  if (fflush (fp) || ferror (fp))
    return -1;

  fclose (fp);

  return 0;
}
