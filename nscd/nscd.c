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

/* nscd - Name Service Cache Daemon. Caches passwd and group.  */

#include <errno.h>
#include <getopt.h>
#include <libintl.h>
#include <locale.h>
#include <pthread.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
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

static void termination_handler (int signum);
static int check_pid (const char *file);
static int write_pid (const char *file);
static void usage (int status) __attribute__ ((noreturn));
static void handle_requests (void);

int
main (int argc, char **argv)
{
  int go_background = 1;
  const char *conffile = _PATH_NSCDCONF;

  /* Set locale via LC_ALL.  */
  setlocale (LC_ALL, "");
  /* Set the text message domain.  */
  textdomain (PACKAGE);

  while (1)
    {
      int c;
      int option_index = 0;
      static struct option long_options[] = {
	{ "debug", no_argument, NULL, 'd' },
	{ "help", no_argument, NULL, 'h' },
	{ "version", no_argument, NULL, 'V' },
	{ "shutdown", no_argument, NULL, 'K' },
        {NULL, 0, NULL, '\0'}
      };

      c = getopt_long (argc, argv, "df:ghKV", long_options, &option_index);
      if (c == (-1))
        break;
      switch (c)
	{
	case 'd':
	  debug_flag = 1;
	  go_background = 0;
	  break;
	case 'f':
	  conffile = optarg;
	  break;
	case 'h':
	  usage (EXIT_SUCCESS);
	  break;
	case 'K':
	  if (getuid () != 0)
	    {
	      printf (_("Only root is allowed to use this option!\n\n"));
	      usage (EXIT_FAILURE);
	    }
	  {
	    int sock = __nscd_open_socket ();
	    request_header req;
	    ssize_t nbytes;

	    if (sock == -1)
	      exit (EXIT_FAILURE);

	    req.version = NSCD_VERSION;
	    req.type = SHUTDOWN;
	    req.key_len = 0;
	    nbytes = write (sock, &req, sizeof (request_header));
	    close (sock);
	    if (nbytes != req.key_len)
	      exit (EXIT_FAILURE);
	    else
	      exit (EXIT_SUCCESS);
	  }
	case 'g':
	  print_stat ();
	  exit (EXIT_SUCCESS);
	case 'V':
	  printf ("nscd (GNU %s) %s\n", PACKAGE, VERSION);
	  printf (_("\
Copyright (C) %s Free Software Foundation, Inc.\n\
This is free software; see the source for copying conditions.  There is NO\n\
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n\
"), "1998");
	  printf (_("Written by %s.\n"), "Thorsten Kukuk");
	  exit (EXIT_SUCCESS);
	default:
	  usage (EXIT_FAILURE);
	}
    }

  signal (SIGINT, termination_handler);
  signal (SIGQUIT, termination_handler);
  signal (SIGTERM, termination_handler);

  /* Check if we are already running. */
  if (check_pid (_PATH_NSCDPID))
    {
      fputs (_("already running"), stderr);
      exit (EXIT_FAILURE);
    }

  /* Behave like a daemon.  */
  if (go_background)
    {
      openlog ("nscd", LOG_CONS | LOG_ODELAY, LOG_DAEMON);

      if (daemon (0, 0) < 0)
	{
	  fprintf (stderr, _("connot auto-background: %s\n"),
		   strerror (errno));
	  exit (EXIT_FAILURE);
	}
      if (write_pid (_PATH_NSCDPID) < 0)
        dbg_log ("%s: %s", _PATH_NSCDPID, strerror (errno));

      /* Ignore job control signals */
      signal (SIGTTOU, SIG_IGN);
      signal (SIGTTIN, SIG_IGN);
      signal (SIGTSTP, SIG_IGN);
    }
  /* Cleanup files created by a previous `bind' */
  unlink (_PATH_NSCDSOCKET);

  nscd_parse_file (conffile);

  /* Create first sockets */
  init_sockets ();
  /* Init databases */
  cache_pwdinit ();
  cache_grpinit ();
  /* Handle incoming requests */
  handle_requests ();

  return 0;
}

/* Create a socket connected to a name. */
int
__nscd_open_socket (void)
{
  struct sockaddr_un addr;
  int sock;

  sock = socket (PF_UNIX, SOCK_STREAM, 0);
  if (sock < 0)
    return -1;

  addr.sun_family = AF_UNIX;
  strcpy (addr.sun_path, _PATH_NSCDSOCKET);
  if (connect (sock, (struct sockaddr *) &addr, sizeof (addr)) < 0)
    {
      close (sock);
      return -1;
    }

  return sock;
}

/* Cleanup.  */
static void
termination_handler (int signum)
{
  close_sockets ();

  /* Clean up the files created by `bind'.  */
  unlink (_PATH_NSCDSOCKET);

  /* Clean up pid file.  */
  unlink (_PATH_NSCDPID);

  exit (EXIT_SUCCESS);
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
  -h, --help            display this help and exit\n\
  -V, --version         output version information and exit\n\
  -f configuration-file read configuration data from the specified file.\n\
  -K, --shutdown        shut the server down.\n\
  -g                    Prints configuration and statistics to stdout.\n"),
              program_invocation_name);
      fputs (_("\
Report bugs using the `glibcbug' script to <bugs@gnu.org>.\n"),
             stdout);
    }
  exit (status);
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

      fscanf (fp, "%d", &pid);
      fclose (fp);

      if (kill (pid, 0) == 0)
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
  if (ferror (fp))
    return -1;

  fclose (fp);

  return 0;
}

/* Type of the lookup function for netname2user.  */
typedef int (*pwbyname_function) (const char *name, struct passwd *pw,
				   char *buffer, size_t buflen);

/* Hanlde incoming requests.  */
static
void handle_requests (void)
{
  request_header req;
  int conn; /* Handle on which connection (client) the request came from.  */
  int done = 0;
  char *key;

  while (!done)
    {
      key = NULL;
      get_request (&conn, &req, &key);
      if (debug_flag)
	dbg_log (_("handle_requests: request received (Version = %d)"),
		 req.version);
      switch (req.type)
	{
	case GETPWBYNAME:
	  {
	    param_t *param = malloc (sizeof (param_t));
	    pthread_t thread;

	    if (debug_flag)
	      dbg_log ("\tGETPWBYNAME (%s)", key);
	    param->key = key;
	    param->conn = conn;
	    if (disabled_passwd)
	      pthread_create (&thread, NULL, cache_pw_disabled, (void *)param);
	    else
	      pthread_create (&thread, NULL, cache_getpwnam, (void *)param);
	    pthread_detach (thread);
	  }
	  break;
	case GETPWBYUID:
	  {
	    param_t *param = malloc (sizeof (param_t));
	    pthread_t thread;

	    if (debug_flag)
	      dbg_log ("\tGETPWBYUID (%s)", key);
	    param->key = key;
	    param->conn = conn;
	    if (disabled_passwd)
	      pthread_create (&thread, NULL, cache_pw_disabled, (void *)param);
	    else
	      pthread_create (&thread, NULL, cache_getpwuid, (void *)param);
	    pthread_detach (thread);
	  }
	  break;
	case GETGRBYNAME:
	  {
	    param_t *param = malloc (sizeof (param_t));
	    pthread_t thread;

	    if (debug_flag)
	      dbg_log ("\tGETGRBYNAME (%s)", key);
	    param->key = key;
	    param->conn = conn;
	    if (disabled_group)
	      pthread_create (&thread, NULL, cache_gr_disabled, (void *)param);
	    else
	      pthread_create (&thread, NULL, cache_getgrnam, (void *)param);
	    pthread_detach (thread);
	  }
	  break;
	case GETGRBYGID:
	  {
	    param_t *param = malloc (sizeof (param_t));
	    pthread_t thread;

	    if (debug_flag)
	      dbg_log ("\tGETGRBYGID (%s)", key);
	    param->key = key;
	    param->conn = conn;
	    if (disabled_group)
	      pthread_create (&thread, NULL, cache_gr_disabled, (void *)param);
	    else
	      pthread_create (&thread, NULL, cache_getgrgid, (void *)param);
	    pthread_detach (thread);
	  }
	  break;
	case GETHOSTBYNAME:
	  /* Not yetimplemented.  */
	  close_socket (conn);
	  break;
	case GETHOSTBYADDR:
	  /* Not yet implemented. */
	  close_socket (conn);
	  break;
	case SHUTDOWN:
	  do_shutdown = 1;
	  close_socket (0);
	  close_socket (conn);
	  /* Clean up the files created by `bind'.  */
	  unlink (_PATH_NSCDSOCKET);
	  /* Clean up pid file.  */
	  unlink (_PATH_NSCDPID);
	  done = 1;
	  break;
	case GETSTAT:
	  {
	    stat_response_header resp;

	    if (debug_flag)
	      dbg_log ("\tGETSTAT");

	    get_pw_stat (&resp);
	    get_gr_stat (&resp);
	    resp.debug_level = debug_flag;
	    resp.pw_enabled = !disabled_passwd;
	    resp.gr_enabled = !disabled_group;

	    stat_send (conn, &resp);

	    close_socket (conn);
	  }
	  break;
	default:
	  dbg_log (_("Unknown request (%d)"), req.type);
	  break;
	}
    }
}
