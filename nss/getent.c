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

/* getent: get entries from administrative database
   supported databases: passwd, group, hosts, services, protocols
   and networks */

#include <argp.h>
#include <grp.h>
#include <pwd.h>
#include <ctype.h>
#include <error.h>
#include <libintl.h>
#include <locale.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* Get libc version number.  */
#include <version.h>

#define PACKAGE _libc_intl_domainname

/* Name and version of program.  */
static void print_version (FILE *stream, struct argp_state *state);
void (*argp_program_version_hook) (FILE *, struct argp_state *) = print_version;

/* Short description of parameters.  */
static const char args_doc[] = N_("database [key ...]");

/* Short description of program.  */
static const char doc[] =
                N_("getent - get entries from administrative database.");

/* Data structure to communicate with argp functions.  */
static struct argp argp = {
  NULL, NULL, args_doc, doc,
};

/* Print the version information.  */
static void
print_version (FILE *stream, struct argp_state *state)
{
  fprintf (stream, "getent (GNU %s) %s\n", PACKAGE, VERSION);
  fprintf (stream, gettext ("\
Copyright (C) %s Free Software Foundation, Inc.\n\
This is free software; see the source for copying conditions.  There is NO\n\
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n\
"), "1998");
  fprintf (stream, gettext ("Written by %s.\n"), "Thorsten Kukuk");
}

/* This is for group */
static inline void
print_group (struct group *grp)
{
  unsigned int i = 0;

  printf ("%s:%s:%ld:", grp->gr_name ? grp->gr_name : "",
	  grp->gr_passwd ? grp->gr_passwd : "",
	  (unsigned long)grp->gr_gid);

  while (grp->gr_mem[i] != NULL)
    {
      fputs (grp->gr_mem[i], stdout);
      ++i;
      if (grp->gr_mem[i] != NULL)
	fputs (",", stdout);
    }
  fputs ("\n", stdout);
}

static inline int
group_keys (int number, char *key[])
{
  int result = 0;
  int i;

  for (i = 0; i < number; ++i)
    {
      struct group *grp;

      if (isdigit (key[i][0]))
	grp = getgrgid (atol (key[i]));
      else
	grp = getgrnam (key[i]);

      if (grp == NULL)
	result = 2;
      else
	print_group (grp);
    }

  return result;
}

/* This is for networks */
static inline void
print_networks (struct netent *net)
{
  unsigned int i;
  struct in_addr ip;
  ip.s_addr = htonl (net->n_net);

  fputs (net->n_name, stdout);
  for  (i = strlen (net->n_name); i < 22; ++i)
    fputs (" ", stdout);
  fputs (inet_ntoa (ip), stdout);

  i = 0;
  while (net->n_aliases[i] != NULL)
    {
      fputs (" ", stdout);
      fputs (net->n_aliases[i], stdout);
      ++i;
      if (net->n_aliases[i] != NULL)
	fputs (",", stdout);
    }
  fputs ("\n", stdout);
}

static inline int
networks_keys (int number, char *key[])
{
  int result = 0;
  int i;

  for (i = 0; i < number; ++i)
    {
      struct netent *net;

      if (isdigit (key[i][0]))
	net = getnetbyaddr (inet_addr (key[i]), AF_UNIX);
      else
	net = getnetbyname (key[i]);

      if (net == NULL)
	result = 2;
      else
	print_networks (net);
    }

  return result;
}

/* Now is all for passwd */
static inline void
print_passwd (struct passwd *pwd)
{
  printf ("%s:%s:%ld:%ld:%s:%s:%s\n",
	  pwd->pw_name ? pwd->pw_name : "",
	  pwd->pw_passwd ? pwd->pw_passwd : "",
	  (unsigned long)pwd->pw_uid,
	  (unsigned long)pwd->pw_gid,
	  pwd->pw_gecos ? pwd->pw_gecos : "",
	  pwd->pw_dir ? pwd->pw_dir : "",
	  pwd->pw_shell ? pwd->pw_shell : "");
}

static inline int
passwd_keys (int number, char *key[])
{
  int result = 0;
  int i;

  for (i = 0; i < number; ++i)
    {
      struct passwd *pwd;

      if (isdigit (key[i][0]))
	pwd = getpwuid (atol (key[i]));
      else
	pwd = getpwnam (key[i]);

      if (pwd == NULL)
	result = 2;
      else
	print_passwd (pwd);
    }

  return result;
}

/* This is for protocols */
static inline void
print_protocols (struct protoent *proto)
{
  unsigned int i;

  fputs (proto->p_name, stdout);
  for (i = strlen (proto->p_name); i < 22; ++i)
    fputs (" ", stdout);
  printf ("%d", proto->p_proto);

  i = 0;
  while (proto->p_aliases[i] != NULL)
    {
      fputs (" ", stdout);
      fputs (proto->p_aliases[i], stdout);
      ++i;
    }
  fputs ("\n", stdout);
}

static inline int
protocols_keys (int number, char *key[])
{
  int result = 0;
  int i;

  for (i = 0; i < number; ++i)
    {
      struct protoent *proto;

      if (isdigit (key[i][0]))
	proto = getprotobynumber (atol (key[i]));
      else
	proto = getprotobyname (key[i]);

      if (proto == NULL)
	result = 2;
      else
	print_protocols (proto);
    }

  return result;
}

/* This is for hosts */
static inline void
print_hosts (struct hostent *host)
{
  unsigned int i;
  char *ip = inet_ntoa(* (struct in_addr *) host->h_addr_list[0]);

  fputs (ip, stdout);
  for (i = strlen (ip); i < 16; ++i)
    fputs (" ", stdout);
  fputs (host->h_name, stdout);

  i = 0;
  while (host->h_aliases[i] != NULL)
    {
      fputs (" ", stdout);
      fputs (host->h_aliases[i], stdout);
      ++i;
    }
  fputs ("\n", stdout);
}

static inline int
hosts_keys (int number, char *key[])
{
  int result = 0;
  int i;

  for (i = 0; i < number; ++i)
    {
      struct hostent *host;

      if (isdigit (key[i][0]))
	{
	  struct in_addr addr;
	  addr.s_addr = inet_addr (key[i]);

	  host = gethostbyaddr ((char *)&addr, sizeof (struct in_addr),
				AF_INET);
	}
      else
	host = gethostbyname (key[i]);

      if (host == NULL)
	result = 2;
      else
	print_hosts (host);
    }

  return result;
}

/* for services */
static inline void
print_services (struct servent *serv)
{
  unsigned int i;

  fputs (serv->s_name, stdout);
  for (i = strlen (serv->s_name); i < 22; ++i)
    fputs (" ", stdout);
  printf ("%d/%s", ntohs (serv->s_port), serv->s_proto);

  i = 0;
  while (serv->s_aliases[i] != NULL)
    {
      fputs (" ", stdout);
      fputs (serv->s_aliases[i], stdout);
      ++i;
    }
  fputs ("\n", stdout);
}

static inline int
services_keys (int number, char *key[])
{
  int result = 0;
  int i;

  for (i = 0; i < number; ++i)
    {
      struct servent *serv;
      char *proto = strchr (key[i], '/');

      if (proto == NULL)
	{
	  setservent (0);
	  if (isdigit (key[i][0]))
	    {
	      int port = htons (atol (key[i]));
	      while ((serv = getservent ()) != NULL)
		if (serv->s_port == port)
		  {
		    print_services (serv);
		    break;
		  }
	    }
	  else
	    {
	      while ((serv = getservent ()) != NULL)
		if (strcmp (serv->s_name, key[i]) == 0)
		  {
		    print_services (serv);
		    break;
		  }
	    }
	  endservent ();
	}
      else
	{
	  *proto++ = '\0';

	  if (isdigit (key[i][0]))
	    serv = getservbyport (atol (key[i]), proto);
	  else
	    serv = getservbyname (key[i], proto);

	  if (serv == NULL)
	    result = 2;
	  else
	    print_services (serv);
	}
    }

  return result;
}

/* the main function */
int
main (int argc, char *argv[])
{
  int remaining;

  /* Set locale via LC_ALL.  */
  setlocale (LC_ALL, "");
  /* Set the text message domain.  */
  textdomain (PACKAGE);

  /* Parse and process arguments.  */
  argp_parse (&argp, argc, argv, 0, &remaining, NULL);

  if ((argc - remaining) < 1)
    {
      error (0, 0, gettext ("wrong number of arguments"));
      argp_help (&argp, stdout, ARGP_HELP_SEE, program_invocation_short_name);
      return 1;
    }

  switch(argv[1][0])
    {
    case 'g': /* group */
      if (strcmp (argv[1], "group") == 0)
	{
	  if (argc == 2)
	    {
	      struct group *grp;

	      setgrent ();
	      while ((grp = getgrent()) != NULL)
		print_group (grp);
	      endgrent ();
	    }
	  else
	    return group_keys (argc - 2, &argv[2]);
	}
      else
	goto error;
      break;
    case 'h': /* hosts */
      if (strcmp (argv[1], "hosts") == 0)
	{
	  if (argc == 2)
	    {
	      struct hostent *host;

	      sethostent (0);
	      while ((host = gethostent()) != NULL)
		print_hosts (host);
	      endhostent ();
	    }
	  else
	    return hosts_keys (argc - 2, &argv[2]);
	}
      else
	goto error;
      break;
    case 'n': /* networks */
      if (strcmp (argv[1], "networks") == 0)
	{
	  if (argc == 2)
	    {
	      struct netent *net;

	      setnetent (0);
	      while ((net = getnetent()) != NULL)
		print_networks (net);
	      endnetent ();
	    }
	  else
	    return networks_keys (argc - 2, &argv[2]);
	}
      else
	goto error;
      break;
    case 'p': /* passwd, protocols */
      if (strcmp (argv[1], "passwd") == 0)
	{
	  if (argc == 2)
	    {
	      struct passwd *pwd;

	      setpwent ();
	      while ((pwd = getpwent()) != NULL)
		print_passwd (pwd);
	      endpwent ();
	    }
	  else
	    return passwd_keys (argc - 2, &argv[2]);
	}
      else if (strcmp (argv[1], "protocols") == 0)
	{
	  if (argc == 2)
	    {
	      struct protoent *proto;

	      setprotoent (0);
	      while ((proto = getprotoent()) != NULL)
		print_protocols (proto);
	      endprotoent ();
	    }
	  else
	    return protocols_keys (argc - 2, &argv[2]);
	}
      else
	goto error;
      break;
    case 's': /* services */
      if (strcmp (argv[1], "services") == 0)
	{
	  if (argc == 2)
	    {
	      struct servent *serv;

	      setservent (0);
	      while ((serv = getservent()) != NULL)
		print_services (serv);
	      endservent ();
	    }
	  else
	    return services_keys (argc - 2, &argv[2]);
	}
      else
	goto error;
      break;
    default:
    error:
      fprintf (stderr, _("Unknown database: %s\n"), argv[1]);
      argp_help (&argp, stdout, ARGP_HELP_SEE, program_invocation_short_name);
      return 1;
    }

  return 0;
}
