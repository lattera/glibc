/*
 * Copyright (c) 2010, Oracle America, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials
 *       provided with the distribution.
 *     * Neither the name of the "Oracle America, Inc." nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *   FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *   COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 *   INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *   DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 *   GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 *   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * rpcinfo: ping a particular rpc program
 *     or dump the portmapper
 */

#include <getopt.h>
#include <string.h>
#include <unistd.h>
#include <rpc/rpc.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <rpc/pmap_prot.h>
#include <rpc/pmap_clnt.h>
#include <signal.h>
#include <ctype.h>
#include <locale.h>
#include <libintl.h>

#include "../version.h"
#define PACKAGE _libc_intl_domainname

#define MAXHOSTLEN 256

#define	MIN_VERS	((u_long) 0)
#define	MAX_VERS	((u_long) 4294967295UL)

static void udpping (u_short portflag, int argc, char **argv);
static void tcpping (u_short portflag, int argc, char **argv);
static int pstatus (CLIENT *client, u_long prognum, u_long vers);
static void pmapdump (int argc, char **argv);
static bool_t reply_proc (void *res, struct sockaddr_in *who);
static void brdcst (int argc, char **argv) __attribute__ ((noreturn));
static void deletereg (int argc, char **argv);
static void usage (FILE *stream);
static void print_version (void);
static u_long getprognum (char *arg);
static u_long getvers (char *arg);
static void get_inet_address (struct sockaddr_in *addr, char *host);

/*
 * Functions to be performed.
 */
#define	NONE		0	/* no function */
#define	PMAPDUMP	1	/* dump portmapper registrations */
#define	TCPPING		2	/* ping TCP service */
#define	UDPPING		3	/* ping UDP service */
#define	BRDCST		4	/* ping broadcast UDP service */
#define DELETES		5	/* delete registration for the service */

int
main (int argc, char **argv)
{
  register int c;
  int errflg;
  int function;
  u_short portnum;
  static const struct option long_options[] = {
    { "help", no_argument, NULL, 'H' },
    { "version", no_argument, NULL, 'V' },
    { NULL, 0, NULL, 0 }
  };

  setlocale (LC_ALL, "");
  textdomain (_libc_intl_domainname);

  function = NONE;
  portnum = 0;
  errflg = 0;
  while ((c = getopt_long (argc, argv, "ptubdn:", long_options, NULL)) != -1)
    {
      switch (c)
	{

	case 'p':
	  if (function != NONE)
	    errflg = 1;
	  else
	    function = PMAPDUMP;
	  break;

	case 't':
	  if (function != NONE)
	    errflg = 1;
	  else
	    function = TCPPING;
	  break;

	case 'u':
	  if (function != NONE)
	    errflg = 1;
	  else
	    function = UDPPING;
	  break;

	case 'b':
	  if (function != NONE)
	    errflg = 1;
	  else
	    function = BRDCST;
	  break;

	case 'n':
	  portnum = (u_short) atoi (optarg);	/* hope we don't get bogus # */
	  break;

	case 'd':
	  if (function != NONE)
	    errflg = 1;
	  else
	    function = DELETES;
	  break;

	case 'H':
	  usage (stdout);
	  return 0;

	case 'V':
	  print_version ();
	  return 0;

	case '?':
	  errflg = 1;
	}
    }

  if (errflg || function == NONE)
    {
      usage (stderr);
      return 1;
    }

  switch (function)
    {

    case PMAPDUMP:
      if (portnum != 0)
	{
	  usage (stderr);
	  return 1;
	}
      pmapdump (argc - optind, argv + optind);
      break;

    case UDPPING:
      udpping (portnum, argc - optind, argv + optind);
      break;

    case TCPPING:
      tcpping (portnum, argc - optind, argv + optind);
      break;

    case BRDCST:
      if (portnum != 0)
	{
	  usage (stderr);
	  return 1;
	}
      brdcst (argc - optind, argv + optind);
      break;

    case DELETES:
      deletereg (argc - optind, argv + optind);
      break;
    }

  return 0;
}

static void
udpping (portnum, argc, argv)
     u_short portnum;
     int argc;
     char **argv;
{
  struct timeval to;
  struct sockaddr_in addr;
  enum clnt_stat rpc_stat;
  CLIENT *client;
  u_long prognum, vers, minvers, maxvers;
  int sock = RPC_ANYSOCK;
  struct rpc_err rpcerr;
  int failure;

  if (argc < 2 || argc > 3)
    {
      usage (stderr);
      exit (1);
    }
  prognum = getprognum (argv[1]);
  get_inet_address (&addr, argv[0]);
  /* Open the socket here so it will survive calls to clnt_destroy */
  sock = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (sock < 0)
    {
      perror ("rpcinfo: socket");
      exit (1);
    }
  failure = 0;
  if (argc == 2)
    {
      /*
       * A call to version 0 should fail with a program/version
       * mismatch, and give us the range of versions supported.
       */
      addr.sin_port = htons (portnum);
      to.tv_sec = 5;
      to.tv_usec = 0;
      if ((client = clntudp_create (&addr, prognum, (u_long) 0,
				    to, &sock)) == NULL)
	{
	  clnt_pcreateerror ("rpcinfo");
	  printf (_("program %lu is not available\n"), prognum);
	  exit (1);
	}
      to.tv_sec = 10;
      to.tv_usec = 0;
      rpc_stat = clnt_call (client, NULLPROC, (xdrproc_t) xdr_void,
			    (char *) NULL, (xdrproc_t) xdr_void,
			    (char *) NULL, to);
      if (rpc_stat == RPC_PROGVERSMISMATCH)
	{
	  clnt_geterr (client, &rpcerr);
	  minvers = rpcerr.re_vers.low;
	  maxvers = rpcerr.re_vers.high;
	}
      else if (rpc_stat == RPC_SUCCESS)
	{
	  /*
	   * Oh dear, it DOES support version 0.
	   * Let's try version MAX_VERS.
	   */
	  addr.sin_port = htons (portnum);
	  to.tv_sec = 5;
	  to.tv_usec = 0;
	  if ((client = clntudp_create (&addr, prognum, MAX_VERS,
					to, &sock)) == NULL)
	    {
	      clnt_pcreateerror ("rpcinfo");
	      printf (_("program %lu version %lu is not available\n"),
		      prognum, MAX_VERS);
	      exit (1);
	    }
	  to.tv_sec = 10;
	  to.tv_usec = 0;
	  rpc_stat = clnt_call (client, NULLPROC, (xdrproc_t) xdr_void,
				NULL, (xdrproc_t) xdr_void, NULL, to);
	  if (rpc_stat == RPC_PROGVERSMISMATCH)
	    {
	      clnt_geterr (client, &rpcerr);
	      minvers = rpcerr.re_vers.low;
	      maxvers = rpcerr.re_vers.high;
	    }
	  else if (rpc_stat == RPC_SUCCESS)
	    {
	      /*
	       * It also supports version MAX_VERS.
	       * Looks like we have a wise guy.
	       * OK, we give them information on all
	       * 4 billion versions they support...
	       */
	      minvers = 0;
	      maxvers = MAX_VERS;
	    }
	  else
	    {
	      (void) pstatus (client, prognum, MAX_VERS);
	      exit (1);
	    }
	}
      else
	{
	  (void) pstatus (client, prognum, (u_long) 0);
	  exit (1);
	}
      clnt_destroy (client);
      for (vers = minvers; vers <= maxvers; vers++)
	{
	  addr.sin_port = htons (portnum);
	  to.tv_sec = 5;
	  to.tv_usec = 0;
	  if ((client = clntudp_create (&addr, prognum, vers,
					to, &sock)) == NULL)
	    {
	      clnt_pcreateerror ("rpcinfo");
	      printf (_("program %lu version %lu is not available\n"),
		      prognum, vers);
	      exit (1);
	    }
	  to.tv_sec = 10;
	  to.tv_usec = 0;
	  rpc_stat = clnt_call (client, NULLPROC, (xdrproc_t) xdr_void,
				NULL, (xdrproc_t) xdr_void, NULL, to);
	  if (pstatus (client, prognum, vers) < 0)
	    failure = 1;
	  clnt_destroy (client);
	}
    }
  else
    {
      vers = getvers (argv[2]);
      addr.sin_port = htons (portnum);
      to.tv_sec = 5;
      to.tv_usec = 0;
      if ((client = clntudp_create (&addr, prognum, vers,
				    to, &sock)) == NULL)
	{
	  clnt_pcreateerror ("rpcinfo");
	  printf (_("program %lu version %lu is not available\n"),
		  prognum, vers);
	  exit (1);
	}
      to.tv_sec = 10;
      to.tv_usec = 0;
      rpc_stat = clnt_call (client, 0, (xdrproc_t) xdr_void, NULL,
			    (xdrproc_t) xdr_void, NULL, to);
      if (pstatus (client, prognum, vers) < 0)
	failure = 1;
    }
  (void) close (sock);		/* Close it up again */
  if (failure)
    exit (1);
}

static void
tcpping (portnum, argc, argv)
     u_short portnum;
     int argc;
     char **argv;
{
  struct timeval to;
  struct sockaddr_in addr;
  enum clnt_stat rpc_stat;
  CLIENT *client;
  u_long prognum, vers, minvers, maxvers;
  int sock = RPC_ANYSOCK;
  struct rpc_err rpcerr;
  int failure;

  if (argc < 2 || argc > 3)
    {
      usage (stderr);
      exit (1);
    }
  prognum = getprognum (argv[1]);
  get_inet_address (&addr, argv[0]);
  failure = 0;
  if (argc == 2)
    {
      /*
       * A call to version 0 should fail with a program/version
       * mismatch, and give us the range of versions supported.
       */
      addr.sin_port = htons (portnum);
      if ((client = clnttcp_create (&addr, prognum, MIN_VERS,
				    &sock, 0, 0)) == NULL)
	{
	  clnt_pcreateerror ("rpcinfo");
	  printf (_("program %lu is not available\n"), prognum);
	  exit (1);
	}
      to.tv_sec = 10;
      to.tv_usec = 0;
      rpc_stat = clnt_call (client, NULLPROC, (xdrproc_t) xdr_void, NULL,
			    (xdrproc_t) xdr_void, NULL, to);
      if (rpc_stat == RPC_PROGVERSMISMATCH)
	{
	  clnt_geterr (client, &rpcerr);
	  minvers = rpcerr.re_vers.low;
	  maxvers = rpcerr.re_vers.high;
	}
      else if (rpc_stat == RPC_SUCCESS)
	{
	  /*
	   * Oh dear, it DOES support version 0.
	   * Let's try version MAX_VERS.
	   */
	  addr.sin_port = htons (portnum);
	  if ((client = clnttcp_create (&addr, prognum, MAX_VERS,
					&sock, 0, 0)) == NULL)
	    {
	      clnt_pcreateerror ("rpcinfo");
	      printf (_("program %lu version %lu is not available\n"),
		      prognum, MAX_VERS);
	      exit (1);
	    }
	  to.tv_sec = 10;
	  to.tv_usec = 0;
	  rpc_stat = clnt_call (client, NULLPROC, (xdrproc_t) xdr_void,
				NULL, (xdrproc_t) xdr_void, NULL, to);
	  if (rpc_stat == RPC_PROGVERSMISMATCH)
	    {
	      clnt_geterr (client, &rpcerr);
	      minvers = rpcerr.re_vers.low;
	      maxvers = rpcerr.re_vers.high;
	    }
	  else if (rpc_stat == RPC_SUCCESS)
	    {
	      /*
	       * It also supports version MAX_VERS.
	       * Looks like we have a wise guy.
	       * OK, we give them information on all
	       * 4 billion versions they support...
	       */
	      minvers = 0;
	      maxvers = MAX_VERS;
	    }
	  else
	    {
	      (void) pstatus (client, prognum, MAX_VERS);
	      exit (1);
	    }
	}
      else
	{
	  (void) pstatus (client, prognum, MIN_VERS);
	  exit (1);
	}
      clnt_destroy (client);
      (void) close (sock);
      sock = RPC_ANYSOCK;	/* Re-initialize it for later */
      for (vers = minvers; vers <= maxvers; vers++)
	{
	  addr.sin_port = htons (portnum);
	  if ((client = clnttcp_create (&addr, prognum, vers,
					&sock, 0, 0)) == NULL)
	    {
	      clnt_pcreateerror ("rpcinfo");
	      printf (_("program %lu version %lu is not available\n"),
		      prognum, vers);
	      exit (1);
	    }
	  to.tv_usec = 0;
	  to.tv_sec = 10;
	  rpc_stat = clnt_call (client, 0, (xdrproc_t) xdr_void, NULL,
				(xdrproc_t) xdr_void, NULL, to);
	  if (pstatus (client, prognum, vers) < 0)
	    failure = 1;
	  clnt_destroy (client);
	  (void) close (sock);
	  sock = RPC_ANYSOCK;
	}
    }
  else
    {
      vers = getvers (argv[2]);
      addr.sin_port = htons (portnum);
      if ((client = clnttcp_create (&addr, prognum, vers, &sock,
				    0, 0)) == NULL)
	{
	  clnt_pcreateerror ("rpcinfo");
	  printf (_("program %lu version %lu is not available\n"),
		  prognum, vers);
	  exit (1);
	}
      to.tv_usec = 0;
      to.tv_sec = 10;
      rpc_stat = clnt_call (client, 0, (xdrproc_t) xdr_void, NULL,
			    (xdrproc_t) xdr_void, NULL, to);
      if (pstatus (client, prognum, vers) < 0)
	failure = 1;
    }
  if (failure)
    exit (1);
}

/*
 * This routine should take a pointer to an "rpc_err" structure, rather than
 * a pointer to a CLIENT structure, but "clnt_perror" takes a pointer to
 * a CLIENT structure rather than a pointer to an "rpc_err" structure.
 * As such, we have to keep the CLIENT structure around in order to print
 * a good error message.
 */
static int
pstatus (client, prognum, vers)
     register CLIENT *client;
     u_long prognum;
     u_long vers;
{
  struct rpc_err rpcerr;

  clnt_geterr (client, &rpcerr);
  if (rpcerr.re_status != RPC_SUCCESS)
    {
      clnt_perror (client, "rpcinfo");
      printf (_("program %lu version %lu is not available\n"), prognum, vers);
      return -1;
    }
  else
    {
      printf (_("program %lu version %lu ready and waiting\n"), prognum, vers);
      return 0;
    }
}

static void
pmapdump (argc, argv)
     int argc;
     char **argv;
{
  struct sockaddr_in server_addr;
  register struct hostent *hp;
  struct pmaplist *head = NULL;
  int socket = RPC_ANYSOCK;
  struct timeval minutetimeout;
  register CLIENT *client;
  struct rpcent *rpc;

  if (argc > 1)
    {
      usage (stderr);
      exit (1);
    }
  if (argc == 1)
    get_inet_address (&server_addr, argv[0]);
  else
    {
      bzero ((char *) &server_addr, sizeof server_addr);
      server_addr.sin_family = AF_INET;
      if ((hp = gethostbyname ("localhost")) != NULL)
	memcpy ((caddr_t) & server_addr.sin_addr, hp->h_addr,
		 hp->h_length);
      else
	server_addr.sin_addr.s_addr = inet_addr ("0.0.0.0");
    }
  minutetimeout.tv_sec = 60;
  minutetimeout.tv_usec = 0;
  server_addr.sin_port = htons (PMAPPORT);
  if ((client = clnttcp_create (&server_addr, PMAPPROG,
				PMAPVERS, &socket, 50, 500)) == NULL)
    {
      clnt_pcreateerror (_("rpcinfo: can't contact portmapper"));
      exit (1);
    }
  if (clnt_call (client, PMAPPROC_DUMP, (xdrproc_t) xdr_void, NULL,
		 (xdrproc_t) xdr_pmaplist, (caddr_t) &head,
		 minutetimeout) != RPC_SUCCESS)
    {
      fputs (_("rpcinfo: can't contact portmapper"), stderr);
      fputs (": ", stderr);
      clnt_perror (client, "rpcinfo");
      exit (1);
    }
  if (head == NULL)
    {
      fputs (_("No remote programs registered.\n"), stdout);
    }
  else
    {
      fputs (_("   program vers proto   port\n"), stdout);
      for (; head != NULL; head = head->pml_next)
	{
	  printf ("%10ld%5ld",
		  head->pml_map.pm_prog,
		  head->pml_map.pm_vers);
	  if (head->pml_map.pm_prot == IPPROTO_UDP)
	    printf ("%6s", "udp");
	  else if (head->pml_map.pm_prot == IPPROTO_TCP)
	    printf ("%6s", "tcp");
	  else
	    printf ("%6ld", head->pml_map.pm_prot);
	  printf ("%7ld", head->pml_map.pm_port);
	  rpc = getrpcbynumber (head->pml_map.pm_prog);
	  if (rpc)
	    printf ("  %s\n", rpc->r_name);
	  else
	    printf ("\n");
	}
    }
}

/*
 * reply_proc collects replies from the broadcast.
 * to get a unique list of responses the output of rpcinfo should
 * be piped through sort(1) and then uniq(1).
 */

/*ARGSUSED */
static bool_t
reply_proc (res, who)
     void *res;			/* Nothing comes back */
     struct sockaddr_in *who;	/* Who sent us the reply */
{
  register struct hostent *hp;

  hp = gethostbyaddr ((char *) &who->sin_addr, sizeof who->sin_addr,
		      AF_INET);
  printf ("%s %s\n", inet_ntoa (who->sin_addr),
	  (hp == NULL) ? _("(unknown)") : hp->h_name);
  return FALSE;
}

static void
brdcst (argc, argv)
     int argc;
     char **argv;
{
  enum clnt_stat rpc_stat;
  u_long prognum, vers;

  if (argc != 2)
    {
      usage (stderr);
      exit (1);
    }
  prognum = getprognum (argv[0]);
  vers = getvers (argv[1]);
  rpc_stat = clnt_broadcast (prognum, vers, NULLPROC, (xdrproc_t) xdr_void,
			     NULL, (xdrproc_t) xdr_void, NULL,
			     (resultproc_t) reply_proc);
  if ((rpc_stat != RPC_SUCCESS) && (rpc_stat != RPC_TIMEDOUT))
    {
      fprintf (stderr, _("rpcinfo: broadcast failed: %s\n"),
	       clnt_sperrno (rpc_stat));
      exit (1);
    }
  exit (0);
}

static void
deletereg (argc, argv)
     int argc;
     char **argv;
{
  u_long prog_num, version_num;

  if (argc != 2)
    {
      usage (stderr);
      exit (1);
    }
  if (getuid ())
    {				/* This command allowed only to root */
      fputs (_("Sorry. You are not root\n"), stderr);
      exit (1);
    }
  prog_num = getprognum (argv[0]);
  version_num = getvers (argv[1]);
  if ((pmap_unset (prog_num, version_num)) == 0)
    {
      fprintf (stderr, _("rpcinfo: Could not delete registration for prog %s version %s\n"),
	       argv[0], argv[1]);
      exit (1);
    }
}

static void
usage (FILE *stream)
{
  fputs (_("Usage: rpcinfo [ -n portnum ] -u host prognum [ versnum ]\n"),
	 stream);
  fputs (_("       rpcinfo [ -n portnum ] -t host prognum [ versnum ]\n"),
	 stream);
  fputs (_("       rpcinfo -p [ host ]\n"), stream);
  fputs (_("       rpcinfo -b prognum versnum\n"), stream);
  fputs (_("       rpcinfo -d prognum versnum\n"), stream);
  fputc ('\n', stream);
  fprintf (stream, _("\
For bug reporting instructions, please see:\n\
%s.\n"), REPORT_BUGS_TO);
}

static void
print_version (void)
{
  printf ("rpcinfo %s%s\n", PKGVERSION, VERSION);
}

static u_long
getprognum (arg)
     char *arg;
{
  register struct rpcent *rpc;
  register u_long prognum;

  if (isalpha (*arg))
    {
      rpc = getrpcbyname (arg);
      if (rpc == NULL)
	{
	  fprintf (stderr, _("rpcinfo: %s is unknown service\n"), arg);
	  exit (1);
	}
      prognum = rpc->r_number;
    }
  else
    {
      prognum = (u_long) atoi (arg);
    }

  return prognum;
}

static u_long
getvers (arg)
     char *arg;
{
  register u_long vers;

  vers = (int) atoi (arg);
  return vers;
}

static void
get_inet_address (addr, host)
     struct sockaddr_in *addr;
     char *host;
{
  register struct hostent *hp;

  bzero ((char *) addr, sizeof *addr);
  addr->sin_addr.s_addr = (u_long) inet_addr (host);
  if (addr->sin_addr.s_addr == INADDR_NONE
      || addr->sin_addr.s_addr == INADDR_ANY)
    {
      if ((hp = gethostbyname (host)) == NULL)
	{
	  fprintf (stderr, _("rpcinfo: %s is unknown host\n"),
		   host);
	  exit (1);
	}
      memmove ((char *) &addr->sin_addr, hp->h_addr, hp->h_length);
    }
  addr->sin_family = AF_INET;
}
