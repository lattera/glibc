/* Copyright (C) 1993, 1995, 1996, 1997, 1998 Free Software Foundation, Inc.
   Contributed by David Mosberger (davidm@azstarnet.com).

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

/* This file provides a Linux /etc/host.conf compatible front end to
the various name resolvers (/etc/hosts, named, NIS server, etc.).
Though mostly compatibly, the following differences exist compared
to the original implementation:

	- new command "spoof" takes an arguments like RESOLV_SPOOF_CHECK
	  environment variable (i.e., `off', `nowarn', or `warn').

	- line comments can appear anywhere (not just at the beginning of
	  a line)
*/
#include <ctype.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <net/if.h>

#include "res_hconf.h"

#define _PATH_HOSTCONF	"/etc/host.conf"

/* Environment vars that all user to override default behavior:  */
#define ENV_HOSTCONF	"RESOLV_HOST_CONF"
#define ENV_SERVORDER	"RESOLV_SERV_ORDER"
#define ENV_SPOOF	"RESOLV_SPOOF_CHECK"
#define ENV_TRIM_OVERR	"RESOLV_OVERRIDE_TRIM_DOMAINS"
#define ENV_TRIM_ADD	"RESOLV_ADD_TRIM_DOMAINS"
#define ENV_MULTI	"RESOLV_MULTI"
#define ENV_REORDER	"RESOLV_REORDER"

static const char *arg_service_list (const char *, int, const char *,
				     unsigned int);
static const char *arg_trimdomain_list (const char *, int, const char *,
					unsigned int);
static const char *arg_spoof (const char *, int, const char *, unsigned int);
static const char *arg_bool (const char *, int, const char *, unsigned int);

static struct cmd
{
  const char *name;
  const char *(*parse_args) (const char * filename, int line_num,
			     const char * args, unsigned int arg);
  unsigned int arg;
} cmd[] =
{
  {"order",		arg_service_list,	0},
  {"trim",		arg_trimdomain_list,	0},
  {"spoof",		arg_spoof,		0},
  {"multi",		arg_bool,		HCONF_FLAG_MULTI},
  {"nospoof",		arg_bool,		HCONF_FLAG_SPOOF},
  {"spoofalert",	arg_bool,		HCONF_FLAG_SPOOFALERT},
  {"reorder",		arg_bool,		HCONF_FLAG_REORDER}
};

/* Structure containing the state.  */
struct hconf _res_hconf;

/* Skip white space.  */
static const char *
skip_ws (const char *str)
{
  while (isspace (*str)) ++str;
  return str;
}


/* Skip until whitespace, comma, end of line, or comment character.  */
static const char *
skip_string (const char *str)
{
  while (*str && !isspace (*str) && *str != '#' && *str != ',')
    ++str;
  return str;
}


static const char *
arg_service_list (const char *fname, int line_num, const char *args,
		  unsigned int arg)
{
  enum Name_Service service;
  const char *start;
  size_t len;
  int i;
  static struct
  {
    const char * name;
    enum Name_Service service;
  } svcs[] =
    {
      {"bind",	SERVICE_BIND},
      {"hosts",	SERVICE_HOSTS},
      {"nis",	SERVICE_NIS},
    };

  do
    {
      start = args;
      args = skip_string (args);
      len = args - start;

      service = SERVICE_NONE;
      for (i = 0; i < sizeof (svcs) / sizeof (svcs[0]); ++i)
	{
	  if (__strncasecmp (start, svcs[i].name, len) == 0
	      && len == strlen (svcs[i].name))
	  {
	    service = svcs[i].service;
	    break;
	  }
      }
      if (service == SERVICE_NONE)
	{
	  fprintf (stderr, "%s: line %d: expected service, found `%s'\n",
		   fname, line_num, start);
	  return 0;
	}
      if (_res_hconf.num_services >= SERVICE_MAX)
	{
	  fprintf (stderr, "%s: line %d: cannot specify more than %d services",
		   fname, line_num, SERVICE_MAX);
	  return 0;
	}
      _res_hconf.service[_res_hconf.num_services++] = service;

      args = skip_ws (args);
      switch (*args)
	{
	case ',':
	case ';':
	case ':':
	  args = skip_ws (++args);
	  if (!*args || *args == '#')
	    {
	      fprintf (stderr,
		       "%s: line %d: list delimiter not followed by keyword",
		       fname, line_num);
	      return 0;
	    }
	default:
	  break;
	}
    }
  while (*args && *args != '#');
  return args;
}


static const char *
arg_trimdomain_list (const char *fname, int line_num, const char *args,
		     unsigned int flag)
{
  const char * start;
  size_t len;

  do
    {
      start = args;
      args = skip_string (args);
      len = args - start;

      if (_res_hconf.num_trimdomains >= TRIMDOMAINS_MAX)
	{
	  fprintf (stderr,
		   "%s: line %d: cannot specify more than %d trim domains",
		   fname, line_num, TRIMDOMAINS_MAX);
	  return 0;
	}
      _res_hconf.trimdomain[_res_hconf.num_trimdomains++] =
	__strndup (start, len);
      args = skip_ws (args);
      switch (*args)
	{
	case ',': case ';': case ':':
	  args = skip_ws (++args);
	  if (!*args || *args == '#')
	    {
	      fprintf (stderr,
		       "%s: line %d: list delimiter not followed by domain",
		       fname, line_num);
	      return 0;
	    }
	default:
	  break;
	}
    }
  while (*args && *args != '#');
  return args;
}


static const char *
arg_spoof (const char *fname, int line_num, const char *args, unsigned flag)
{
  const char *start = args;
  size_t len;

  args = skip_string (args);
  len = args - start;

  if (len == 3 && __strncasecmp (start, "off", len) == 0)
    _res_hconf.flags &= ~(HCONF_FLAG_SPOOF | HCONF_FLAG_SPOOFALERT);
  else
    {
      _res_hconf.flags |= (HCONF_FLAG_SPOOF | HCONF_FLAG_SPOOFALERT);
      if ((len == 6 && __strncasecmp (start, "nowarn", len) == 0)
	  || !(len == 4 && __strncasecmp (start, "warn", len) == 0))
	_res_hconf.flags &= ~HCONF_FLAG_SPOOFALERT;
    }
  return args;
}


static const char *
arg_bool (const char *fname, int line_num, const char *args, unsigned flag)
{
  if (__strncasecmp (args, "on", 2) == 0)
    {
      args += 2;
      _res_hconf.flags |= flag;
    }
  else if (__strncasecmp (args, "off", 3) == 0)
    {
      args += 3;
      _res_hconf.flags &= ~flag;
    }
  else
    {
      fprintf (stderr, "%s: line %d: expected `on' or `off', found `%s'\n",
	       fname, line_num, args);
      return 0;
    }
  return args;
}


static void
parse_line (const char *fname, int line_num, const char *str)
{
  const char *start;
  struct cmd *c = 0;
  size_t len;
  int i;

  str = skip_ws (str);

  if (*str == '#') return;		/* skip line comment */

  start = str;
  str = skip_string (str);
  len = str - start;

  for (i = 0; i < sizeof (cmd) / sizeof (cmd[0]); ++i)
    {
      if (strncasecmp (start, cmd[i].name, len) == 0
	  && strlen (cmd[i].name) == len)
	{
	  c = &cmd[i];
	  break;
	}
    }
  if (c == NULL)
    {
      fprintf (stderr, "%s: line %d: bad command `%s'\n",
	       fname, line_num, start);
      return;
    }

  /* process args: */
  str = skip_ws (str);
  str = (*c->parse_args) (fname, line_num, str, c->arg);
  if (!str)
    return;

  /* rest of line must contain white space or comment only: */
  while (*str)
    {
      if (!isspace (*str)) {
	if (*str != '#')
	  fprintf (stderr, "%s: line %d: ignoring trailing garbage `%s'\n",
		   fname, line_num, str);
	break;
      }
      ++str;
    }
}


/* Initialize hconf datastructure by reading host.conf file and
   environment variables.  */
void
_res_hconf_init (void)
{
  const char *hconf_name;
  int line_num = 0;
  char buf[256], *end, *envval;
  FILE *fp;

  if (_res_hconf.initialized)
    return;

  memset (&_res_hconf, '\0', sizeof (_res_hconf));

  hconf_name = getenv (ENV_HOSTCONF);
  if (hconf_name == NULL)
    hconf_name = _PATH_HOSTCONF;

  fp = fopen (hconf_name, "r");
  if (!fp)
    /* make up something reasonable: */
    _res_hconf.service[_res_hconf.num_services++] = SERVICE_BIND;
  else
    {
      while (fgets_unlocked (buf, sizeof (buf), fp))
	{
	  ++line_num;
	  end = strchr (buf, '\n');
	  if (end)
	    *end = '\0';
	  parse_line (hconf_name, line_num, buf);
	}
      fclose (fp);
    }

  envval = getenv (ENV_SERVORDER);
  if (envval)
    {
      _res_hconf.num_services = 0;
      arg_service_list (ENV_SERVORDER, 1, envval, 0);
    }

  envval = getenv (ENV_SPOOF);
  if (envval)
    arg_spoof (ENV_SPOOF, 1, envval, 0);

  envval = getenv (ENV_MULTI);
  if (envval)
    arg_bool (ENV_MULTI, 1, envval, HCONF_FLAG_MULTI);

  envval = getenv (ENV_REORDER);
  if (envval)
    arg_bool (ENV_REORDER, 1, envval, HCONF_FLAG_REORDER);

  envval = getenv (ENV_TRIM_ADD);
  if (envval)
    arg_trimdomain_list (ENV_TRIM_ADD, 1, envval, 0);

  envval = getenv (ENV_TRIM_OVERR);
  if (envval)
    {
      _res_hconf.num_trimdomains = 0;
      arg_trimdomain_list (ENV_TRIM_OVERR, 1, envval, 0);
    }

  _res_hconf.initialized = 1;
}


/* Reorder addresses returned in a hostent such that the first address
   is an address on the local subnet, if there is such an address.
   Otherwise, nothing is changed.  */

void
_res_hconf_reorder_addrs (struct hostent *hp)
{
#if defined SIOCGIFCONF && defined SIOCGIFNETMASK
  static int num_ifs = -1;	/* number of interfaces */
  static struct netaddr
  {
    int addrtype;
    union
    {
      struct
      {
	u_int32_t	addr;
	u_int32_t	mask;
      } ipv4
    } u;
  } *ifaddrs;

  if (hp->h_addrtype != AF_INET)
    return;	/* can't deal with anything but IPv4 for now... */

  if (num_ifs <= 0)
    {
      struct ifconf ifs;
      struct ifreq *ifr;
      size_t size, num;
      int sd;

      /* initialize interface table: */

      num_ifs = 0;

      sd = __socket (AF_INET, SOCK_DGRAM, 0);
      if (sd < 0)
	return;

      /* Now get list of interfaces.  Since we don't know how many
	 interfaces there are, we keep increasing the buffer size
	 until we have at least sizeof(struct ifreq) too many bytes.
	 That implies that the ioctl() return because it ran out of
	 interfaces, not memory */
      size = 0;
      ifs.ifc_buf = 0;
      do
	{
	  size += 4 * sizeof (struct ifreq);
	  ifs.ifc_buf = realloc (ifs.ifs_buf, size);
	  if (ifs.ifc_buf == NULL)
	    {
	      close (sd);
	      return;
	    }
	  ifs.ifc_len = size;
	  if (__ioctl (sd, SIOCGIFCONF, &ifs) < 0)
	    goto cleanup;
	}
      while (size - ifs.ifc_len < sizeof (struct ifreq));

      num = ifs.ifc_len / sizeof (struct ifreq);

      ifaddrs = malloc (num * sizeof (ifaddrs[0]));
      if (!ifaddrs)
	goto cleanup;

      ifr = ifs.ifc_req;
      for (i = 0; i < num; ++i)
	{
	  if (ifr->ifr_addr.sa_family != AF_INET)
	    continue;
	  ifaddrs[num_ifs].addrtype = AF_INET;

	  memcpy (&ifaddrs[num_ifs].u.ipv4.addr,
		  &((struct sockaddr_in *)ifr->ifr_addr)->sin_addr, 4);

	  if (__ioctl (sd, SIOCGIFNETMASK, if) < 0)
	    continue;
	  memcpy (&ifaddrs[num_ifs].u.ipv4.mask,
		  ((struct sockaddr_in *)ifr->ifr_mask)->sin_addr, 4);

	  ++num_ifs;	/* now we're committed to this entry */
	}
      /* just keep enough memory to hold all the interfaces we want: */
      ifaddrs = realloc (ifaddrs, num_ifs * sizeof (ifaddrs[0]));

    cleanup:
      close (sd);
      free (ifs.ifc_buf);
    }

  if (num_ifs == 0)
    return;

  /* find an address for which we have a direct connection: */
  for (i = 0; hp->h_addr_list[i]; ++i)
    {
      h_addr = (struct in_addr *) hp->h_addr_list[i];

      for (j = 0; j < num_ifs; ++j)
	{
	  if_addr    = ifaddrs[j].u.ipv4.addr;
	  if_netmask = ifaddrs[j].u.ipv4.mask;

	  if (((h_addr->s_addr ^ if_addr) & if_netmask) == 0)
	    {
	      void *tmp;

	      tmp = hp->h_addr_list[i];
	      hp->h_addr_list[i] = hp->h_addr_list[0];
	      hp->h_addr_list[0] = tmp;
	      return;
	    }
	}
    }
#endif /* defined(SIOCGIFCONF) && ... */
}


/* If HOSTNAME has a postfix matching any of the trimdomains, trim away
   that postfix.  Notice that HOSTNAME is modified inplace.  Also, the
   original code applied all trimdomains in order, meaning that the
   same domainname could be trimmed multiple times.  I believe this
   was unintentional.  */
void
_res_hconf_trim_domain (char *hostname)
{
  size_t hostname_len, trim_len;
  int i;

  hostname_len = strlen (hostname);

  for (i = 0; i < _res_hconf.num_trimdomains; ++i)
    {
      const char *trim = _res_hconf.trimdomain[i];

      trim_len = strlen (trim);
      if (hostname_len > trim_len
	  && __strcasecmp (&hostname[hostname_len - trim_len], trim) == 0)
	{
	  hostname[hostname_len - trim_len] = '\0';
	  break;
	}
    }
}


/* Trim all hostnames/aliases in HP according to the trimdomain list.
   Notice that HP is modified inplace!  */
void
_res_hconf_trim_domains (struct hostent *hp)
{
  int i;

  if (_res_hconf.num_trimdomains == 0)
    return;

  _res_hconf_trim_domain (hp->h_name);
  for (i = 0; hp->h_aliases[i]; ++i)
    _res_hconf_trim_domain (hp->h_aliases[i]);
}
