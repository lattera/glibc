/* Copyright (c) 1996 by Internet Software Consortium.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND INTERNET SOFTWARE CONSORTIUM DISCLAIMS
 * ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL INTERNET SOFTWARE
 * CONSORTIUM BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

/* Copyright 1996 by the Massachusetts Institute of Technology.
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 * M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 */

/* This file is part of the hesiod library.  It implements the core
 * portion of the hesiod resolver.
 *
 * This file is loosely based on an interim version of hesiod.c from
 * the BIND IRS library, which was in turn based on an earlier version
 * of this file.  Extensive changes have been made on each step of the
 * path.
 *
 * This implementation is not truly thread-safe at the moment because
 * it uses res_send() and accesses _res.
 */

static const char rcsid[] = "$Id$";

#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <errno.h>
#include <netdb.h>
#include <resolv.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "hesiod.h"
#include "hesiod_p.h"

/* A few operating systems don't define these. */
#ifndef C_HS
# define C_HS	4
#endif
#ifndef T_TXT
# define T_TXT	16
#endif

static int read_config_file (struct hesiod_p *ctx, const char *filename);
static char **get_txt_records (struct hesiod_p *ctx, int class,
			       const char *name);
#ifdef _LIBC
# define cistrcmp(s1, s2) strcasecmp (s1, s2)
#else
static int cistrcmp (const char *s1, const char *s2);
#endif

/* This function is called to initialize a hesiod_p. */
int
hesiod_init (void **context)
{
  struct hesiod_p *ctx;
  const char *p, *configname;

  ctx = malloc (sizeof (struct hesiod_p));
  if (ctx)
    {
      *context = ctx;
      configname = __secure_getenv ("HESIOD_CONFIG");
      if (!configname)
	configname = SYSCONFDIR "/hesiod.conf";
      if (read_config_file (ctx, configname) >= 0)
	{
	  /* The default rhs can be overridden by an environment variable. */
	  p = __secure_getenv ("HES_DOMAIN");
	  if (p)
	    {
	      if (ctx->rhs)
		free (ctx->rhs);
	      ctx->rhs = malloc (strlen (p) + 2);
	      if (ctx->rhs)
		{
		  *ctx->rhs = '.';
		  strcpy (ctx->rhs + 1, (*p == '.') ? p + 1 : p);
		  return 0;
		}
	      else
		__set_errno (ENOMEM);
	    }
	  else
	    return 0;
	}
    }
  else
    __set_errno (ENOMEM);

  if (ctx->lhs)
    free (ctx->lhs);
  if (ctx->rhs)
    free (ctx->rhs);
  if (ctx)
    free (ctx);
  return -1;
}

/* This function deallocates the hesiod_p. */
void
hesiod_end (void *context)
{
  struct hesiod_p *ctx = (struct hesiod_p *) context;

  free (ctx->rhs);
  if (ctx->lhs)
    free (ctx->lhs);
  free (ctx);
}

/* This function takes a hesiod (name, type) and returns a DNS
 * name which is to be resolved.
 */
char *
hesiod_to_bind (void *context, const char *name, const char *type)
{
  struct hesiod_p *ctx = (struct hesiod_p *) context;
  char bindname[MAXDNAME], *p, *endp, *ret, **rhs_list = NULL;
  const char *rhs;
  size_t len;

  endp = stpcpy (bindname, name);

  /* Find the right right hand side to use, possibly truncating bindname. */
  p = strchr (bindname, '@');
  if (p)
    {
      *p++ = 0;
      if (strchr (p, '.'))
	rhs = name + (p - bindname);
      else
	{
	  rhs_list = hesiod_resolve (context, p, "rhs-extension");
	  if (rhs_list)
	    rhs = *rhs_list;
	  else
	    {
	      __set_errno (ENOENT);
	      return NULL;
	    }
	}
    }
  else
    rhs = ctx->rhs;

  /* See if we have enough room. */
  len = (endp - bindname) + 1 + strlen (type);
  if (ctx->lhs)
    len += strlen (ctx->lhs) + ((ctx->lhs[0] != '.') ? 1 : 0);
  len += strlen (rhs) + ((rhs[0] != '.') ? 1 : 0);
  if (len > sizeof (bindname) - 1)
    {
      if (rhs_list)
	hesiod_free_list (context, rhs_list);
      __set_errno (EMSGSIZE);
      return NULL;
    }

  /* Put together the rest of the domain. */
  endp = stpcpy (stpcpy (endp, "."), type);
  if (ctx->lhs)
    {
      if (ctx->lhs[0] != '.')
	endp = stpcpy (endp, ".");
      endp = stpcpy (endp, ctx->lhs);
    }
  if (rhs[0] != '.')
    endp = stpcpy (endp, ".");
  endp = stpcpy (endp, rhs);

  /* rhs_list is no longer needed, since we're done with rhs. */
  if (rhs_list)
    hesiod_free_list (context, rhs_list);

  /* Make a copy of the result and return it to the caller. */
  ret = malloc ((endp - bindname) + 1);
  if (!ret)
    {
      __set_errno (ENOMEM);
      return NULL;
    }
  return strcpy (ret, bindname);
}

/* This is the core function.  Given a hesiod name and type, it
 * returns an array of strings returned by the resolver.
 */
char **
hesiod_resolve (void *context, const char *name, const char *type)
{
  struct hesiod_p *ctx = (struct hesiod_p *) context;
  char *bindname, **retvec;

  bindname = hesiod_to_bind (context, name, type);
  if (bindname == NULL)
    return NULL;

  retvec = get_txt_records(ctx, ctx->classes[0], bindname);
  if (retvec == NULL && errno == ENOENT && ctx->classes[1])
    retvec = get_txt_records (ctx, ctx->classes[1], bindname);

  free (bindname);
  return retvec;
}

void
hesiod_free_list (void *context, char **list)
{
  char **p;

  for (p = list; *p; p++)
    free (*p);
  free (list);
}

/* This function parses the /etc/hesiod.conf file.  Returns 0 on success,
 * -1 on failure.  On failure, it might leave values in ctx->lhs or
 * ctx->rhs which need to be freed by the caller. */
static int
read_config_file (struct hesiod_p *ctx, const char *filename)
{
  char *key, *data, *p, **which;
  char buf[MAXDNAME + 7];
  int n;
  FILE *fp;

  /* Set default query classes. */
  ctx->classes[0] = C_IN;
  ctx->classes[1] = C_HS;

  /* Try to open the configuration file. */
  fp = fopen (filename, "r");
  if (fp == NULL)
    {
      /* Use compiled in default domain names. */
      ctx->lhs = malloc (strlen (DEF_LHS) + 1);
      ctx->rhs = malloc (strlen (DEF_RHS) + 1);
      if (ctx->lhs && ctx->rhs)
	{
	  strcpy (ctx->lhs, DEF_LHS);
	  strcpy (ctx->rhs, DEF_RHS);
	  return 0;
	}
      else
	{
	  __set_errno (ENOMEM);
	  return -1;
	}
    }

  ctx->lhs = NULL;
  ctx->rhs = NULL;
  while (fgets (buf, sizeof (buf), fp) != NULL)
    {
      p = buf;
      if (*p == '#' || *p == '\n' || *p == '\r')
	continue;
      while (*p == ' ' || *p == '\t')
	++p;
      key = p;
      while(*p != ' ' && *p != '\t' && *p != '=')
	++p;
      *p++ = 0;

      while (isspace (*p) || *p == '=')
	++p;
      data = p;
      while (!isspace (*p))
	++p;
      *p = 0;

      if (cistrcmp (key, "lhs") == 0 || cistrcmp (key, "rhs") == 0)
	{
	  which = (strcmp (key, "lhs") == 0) ? &ctx->lhs : &ctx->rhs;
	  *which = strdup (data);
	  if (!*which)
	    {
	      __set_errno (ENOMEM);
	      return -1;
	    }
	}
      else if (cistrcmp (key, "classes") == 0)
	{
	  n = 0;
	  while (*data && n < 2)
	    {
	      p = data;
	      while (*p && *p != ',')
		++p;
	      if (*p)
		*p++ = 0;
	      if (cistrcmp (data, "IN") == 0)
		ctx->classes[n++] = C_IN;
	      else if (cistrcmp (data, "HS") == 0)
		ctx->classes[n++] = C_HS;
	      data = p;
	    }
	  while (n < 2)
	    ctx->classes[n++] = 0;
	}
    }
  fclose (fp);

  if (!ctx->rhs || ctx->classes[0] == 0 || ctx->classes[0] == ctx->classes[1])
    {
      __set_errno (ENOEXEC);
      return -1;
    }

  return 0;
}

/* Given a DNS class and a DNS name, do a lookup for TXT records, and
 * return a list of them.
 */
static char **
get_txt_records (struct hesiod_p *ctx, int qclass, const char *name)
{
  HEADER *hp;
  unsigned char qbuf[PACKETSZ], abuf[MAX_HESRESP], *p, *eom, *eor;
  char *dst, **list;
  int ancount, qdcount, i, j, n, skip, type, class, len;

  /* Make sure the resolver is initialized. */
  if ((_res.options & RES_INIT) == 0 && res_init () == -1)
    return NULL;

  /* Construct the query. */
  n = res_mkquery (QUERY, name, qclass, T_TXT, NULL, 0,
		   NULL, qbuf, PACKETSZ);
  if (n < 0)
      return NULL;

  /* Send the query. */
  n = res_send (qbuf, n, abuf, MAX_HESRESP);
  if (n < 0)
    {
      __set_errno (ECONNREFUSED);
      return NULL;
    }

  /* Parse the header of the result. */
  hp = (HEADER *) abuf;
  ancount = ntohs (hp->ancount);
  qdcount = ntohs (hp->qdcount);
  p = abuf + sizeof (HEADER);
  eom = abuf + n;

  /* Skip questions, trying to get to the answer section which follows. */
  for (i = 0; i < qdcount; ++i)
    {
      skip = dn_skipname (p, eom);
      if (skip < 0 || p + skip + QFIXEDSZ > eom)
	{
	  __set_errno (EMSGSIZE);
	  return NULL;
	}
      p += skip + QFIXEDSZ;
    }

  /* Allocate space for the text record answers. */
  list = malloc ((ancount + 1) * sizeof(char *));
  if (list == NULL)
    {
      __set_errno (ENOMEM);
      return NULL;
    }

  /* Parse the answers. */
  j = 0;
  for (i = 0; i < ancount; i++)
    {
      /* Parse the header of this answer. */
      skip = dn_skipname (p, eom);
      if (skip < 0 || p + skip + 10 > eom)
	break;
      type = p[skip + 0] << 8 | p[skip + 1];
      class = p[skip + 2] << 8 | p[skip + 3];
      len = p[skip + 8] << 8 | p[skip + 9];
      p += skip + 10;
      if (p + len > eom)
	{
	  __set_errno (EMSGSIZE);
	  break;
	}

      /* Skip entries of the wrong class and type. */
      if (class != qclass || type != T_TXT)
	{
	  p += len;
	  continue;
	}

      /* Allocate space for this answer. */
      list[j] = malloc (len);
      if (!list[j])
	{
	  __set_errno (ENOMEM);
	  break;
	}
      dst = list[j++];

      /* Copy answer data into the allocated area. */
      eor = p + len;
      while (p < eor)
	{
	  n = (unsigned char) *p++;
	  if (p + n > eor)
	    {
	      __set_errno (EMSGSIZE);
	      break;
	    }
	  dst = mempcpy (dst, p, n);
	  p += n;
	}
      if (p < eor)
	{
	  __set_errno (EMSGSIZE);
	  break;
	}
      *dst = 0;
    }

  /* If we didn't terminate the loop normally, something went wrong. */
  if (i < ancount)
    {
      for (i = 0; i < j; i++)
	free (list[i]);
      free (list);
      return NULL;
    }

  if (j == 0)
    {
      __set_errno (ENOENT);
      free (list);
      return NULL;
    }

  list[j] = NULL;
  return list;
}

#ifndef _LIBC
static int
cistrcmp (const char *s1, const char *s2)
{
  while (*s1 && tolower(*s1) == tolower(*s2))
    {
      s1++;
      s2++;
    }
  return tolower(*s1) - tolower(*s2);
}
#endif
