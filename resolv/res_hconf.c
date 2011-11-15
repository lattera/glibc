/* Copyright (C) 1993,1995-2006,2007,2009,2011
	Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by David Mosberger (davidm@azstarnet.com).

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

/* This file provides a Linux /etc/host.conf compatible front end to
   the various name resolvers (/etc/hosts, named, NIS server, etc.).
   Though mostly compatibly, the following differences exist compared
   to the original implementation:

	- new command "spoof" takes an arguments like RESOLV_SPOOF_CHECK
	  environment variable (i.e., `off', `nowarn', or `warn').

	- line comments can appear anywhere (not just at the beginning of
	  a line)
*/

#include <assert.h>
#include <errno.h>
#include <ctype.h>
#include <libintl.h>
#include <memory.h>
#include <stdio.h>
#include <stdio_ext.h>
#include <stdlib.h>
#include <string.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <bits/libc-lock.h>
#include "ifreq.h"
#include "res_hconf.h"
#include <wchar.h>

#define _PATH_HOSTCONF	"/etc/host.conf"

/* Environment vars that all user to override default behavior:  */
#define ENV_HOSTCONF	"RESOLV_HOST_CONF"
#define ENV_SPOOF	"RESOLV_SPOOF_CHECK"
#define ENV_TRIM_OVERR	"RESOLV_OVERRIDE_TRIM_DOMAINS"
#define ENV_TRIM_ADD	"RESOLV_ADD_TRIM_DOMAINS"
#define ENV_MULTI	"RESOLV_MULTI"
#define ENV_REORDER	"RESOLV_REORDER"

enum parse_cbs
  {
    CB_none,
    CB_arg_trimdomain_list,
    CB_arg_spoof,
    CB_arg_bool
  };

static const struct cmd
{
  const char name[11];
  uint8_t cb;
  unsigned int arg;
} cmd[] =
{
  {"order",		CB_none,		0},
  {"trim",		CB_arg_trimdomain_list,	0},
  {"spoof",		CB_arg_spoof,		0},
  {"multi",		CB_arg_bool,		HCONF_FLAG_MULTI},
  {"nospoof",		CB_arg_bool,		HCONF_FLAG_SPOOF},
  {"spoofalert",	CB_arg_bool,		HCONF_FLAG_SPOOFALERT},
  {"reorder",		CB_arg_bool,		HCONF_FLAG_REORDER}
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
arg_trimdomain_list (const char *fname, int line_num, const char *args)
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
	  char *buf;

	  if (__asprintf (&buf, _("\
%s: line %d: cannot specify more than %d trim domains"),
			  fname, line_num, TRIMDOMAINS_MAX) < 0)
	    return 0;

	  __fxprintf (NULL, "%s", buf);

	  free (buf);
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
	      char *buf;

	      if (__asprintf (&buf, _("\
%s: line %d: list delimiter not followed by domain"),
			      fname, line_num) < 0)
		return 0;

	      __fxprintf (NULL, "%s", buf);

	      free (buf);
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
arg_spoof (const char *fname, int line_num, const char *args)
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
      char *buf;

      if (__asprintf (&buf,
		      _("%s: line %d: expected `on' or `off', found `%s'\n"),
		      fname, line_num, args) < 0)
	return 0;

      __fxprintf (NULL, "%s", buf);

      free (buf);
      return 0;
    }
  return args;
}


static void
parse_line (const char *fname, int line_num, const char *str)
{
  const char *start;
  const struct cmd *c = 0;
  size_t len;
  size_t i;

  str = skip_ws (str);

  /* skip line comment and empty lines: */
  if (*str == '\0' || *str == '#') return;

  start = str;
  str = skip_string (str);
  len = str - start;

  for (i = 0; i < sizeof (cmd) / sizeof (cmd[0]); ++i)
    {
      if (__strncasecmp (start, cmd[i].name, len) == 0
	  && strlen (cmd[i].name) == len)
	{
	  c = &cmd[i];
	  break;
	}
    }
  if (c == NULL)
    {
      char *buf;

      if (__asprintf (&buf, _("%s: line %d: bad command `%s'\n"),
		      fname, line_num, start) < 0)
	return;

      __fxprintf (NULL, "%s", buf);

      free (buf);
      return;
    }

  /* process args: */
  str = skip_ws (str);

  if (c->cb == CB_arg_trimdomain_list)
    str = arg_trimdomain_list (fname, line_num, str);
  else if (c->cb == CB_arg_spoof)
    str = arg_spoof (fname, line_num, str);
  else if (c->cb == CB_arg_bool)
    str = arg_bool (fname, line_num, str, c->arg);
  else
    /* Ignore the line.  */
    return;

  if (!str)
    return;

  /* rest of line must contain white space or comment only: */
  while (*str)
    {
      if (!isspace (*str)) {
	if (*str != '#')
	  {
	    char *buf;

	    if (__asprintf (&buf,
			    _("%s: line %d: ignoring trailing garbage `%s'\n"),
			    fname, line_num, str) < 0)
	      break;

	    __fxprintf (NULL, "%s", buf);

	    free (buf);
	  }
	break;
      }
      ++str;
    }
}


static void
do_init (void)
{
  const char *hconf_name;
  int line_num = 0;
  char buf[256], *envval;
  FILE *fp;

  memset (&_res_hconf, '\0', sizeof (_res_hconf));

  hconf_name = getenv (ENV_HOSTCONF);
  if (hconf_name == NULL)
    hconf_name = _PATH_HOSTCONF;

  fp = fopen (hconf_name, "rce");
  if (fp)
    {
      /* No threads using this stream.  */
      __fsetlocking (fp, FSETLOCKING_BYCALLER);

      while (fgets_unlocked (buf, sizeof (buf), fp))
	{
	  ++line_num;
	  *__strchrnul (buf, '\n') = '\0';
	  parse_line (hconf_name, line_num, buf);
	}
      fclose (fp);
    }

  envval = getenv (ENV_SPOOF);
  if (envval)
    arg_spoof (ENV_SPOOF, 1, envval);

  envval = getenv (ENV_MULTI);
  if (envval)
    arg_bool (ENV_MULTI, 1, envval, HCONF_FLAG_MULTI);

  envval = getenv (ENV_REORDER);
  if (envval)
    arg_bool (ENV_REORDER, 1, envval, HCONF_FLAG_REORDER);

  envval = getenv (ENV_TRIM_ADD);
  if (envval)
    arg_trimdomain_list (ENV_TRIM_ADD, 1, envval);

  envval = getenv (ENV_TRIM_OVERR);
  if (envval)
    {
      _res_hconf.num_trimdomains = 0;
      arg_trimdomain_list (ENV_TRIM_OVERR, 1, envval);
    }

  _res_hconf.initialized = 1;
}


/* Initialize hconf datastructure by reading host.conf file and
   environment variables.  */
void
_res_hconf_init (void)
{
  __libc_once_define (static, once);

  __libc_once (once, do_init);
}


#ifndef NOT_IN_libc
/* List of known interfaces.  */
libc_freeres_ptr (
static struct netaddr
{
  int addrtype;
  union
  {
    struct
    {
      u_int32_t	addr;
      u_int32_t	mask;
    } ipv4;
  } u;
} *ifaddrs);

/* Reorder addresses returned in a hostent such that the first address
   is an address on the local subnet, if there is such an address.
   Otherwise, nothing is changed.

   Note that this function currently only handles IPv4 addresses.  */

void
_res_hconf_reorder_addrs (struct hostent *hp)
{
#if defined SIOCGIFCONF && defined SIOCGIFNETMASK
  int i, j;
  /* Number of interfaces.  */
  static int num_ifs = -1;
  /* We need to protect the dynamic buffer handling.  */
  __libc_lock_define_initialized (static, lock);

  /* Only reorder if we're supposed to.  */
  if ((_res_hconf.flags & HCONF_FLAG_REORDER) == 0)
    return;

  /* Can't deal with anything but IPv4 for now...  */
  if (hp->h_addrtype != AF_INET)
    return;

  if (num_ifs <= 0)
    {
      struct ifreq *ifr, *cur_ifr;
      int sd, num, i;
      /* Save errno.  */
      int save = errno;

      /* Initialize interface table.  */

      /* The SIOCGIFNETMASK ioctl will only work on an AF_INET socket.  */
      sd = __socket (AF_INET, SOCK_DGRAM, 0);
      if (sd < 0)
	return;

      /* Get lock.  */
      __libc_lock_lock (lock);

      /* Recheck, somebody else might have done the work by done.  */
      if (num_ifs <= 0)
	{
	  int new_num_ifs = 0;

	  /* Get a list of interfaces.  */
	  __ifreq (&ifr, &num, sd);
	  if (!ifr)
	    goto cleanup;

	  ifaddrs = malloc (num * sizeof (ifaddrs[0]));
	  if (!ifaddrs)
	    goto cleanup1;

	  /* Copy usable interfaces in ifaddrs structure.  */
	  for (cur_ifr = ifr, i = 0; i < num;
	       cur_ifr = __if_nextreq (cur_ifr), ++i)
	    {
	      if (cur_ifr->ifr_addr.sa_family != AF_INET)
		continue;

	      ifaddrs[new_num_ifs].addrtype = AF_INET;
	      ifaddrs[new_num_ifs].u.ipv4.addr =
		((struct sockaddr_in *) &cur_ifr->ifr_addr)->sin_addr.s_addr;

	      if (__ioctl (sd, SIOCGIFNETMASK, cur_ifr) < 0)
		continue;

	      ifaddrs[new_num_ifs].u.ipv4.mask =
		((struct sockaddr_in *) &cur_ifr->ifr_netmask)->sin_addr.s_addr;

	      /* Now we're committed to this entry.  */
	      ++new_num_ifs;
	    }
	  /* Just keep enough memory to hold all the interfaces we want.  */
	  ifaddrs = realloc (ifaddrs, new_num_ifs * sizeof (ifaddrs[0]));
	  assert (ifaddrs != NULL);

	cleanup1:
	  __if_freereq (ifr, num);

	cleanup:
	  /* Release lock, preserve error value, and close socket.  */
	  errno = save;

	  num_ifs = new_num_ifs;

	  __libc_lock_unlock (lock);
	}

      __close (sd);
    }

  if (num_ifs == 0)
    return;

  /* Find an address for which we have a direct connection.  */
  for (i = 0; hp->h_addr_list[i]; ++i)
    {
      struct in_addr *haddr = (struct in_addr *) hp->h_addr_list[i];

      for (j = 0; j < num_ifs; ++j)
	{
	  u_int32_t if_addr    = ifaddrs[j].u.ipv4.addr;
	  u_int32_t if_netmask = ifaddrs[j].u.ipv4.mask;

	  if (((haddr->s_addr ^ if_addr) & if_netmask) == 0)
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
#endif
