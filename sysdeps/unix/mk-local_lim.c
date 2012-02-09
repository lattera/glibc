/* Copyright (C) 1993, 1996, 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <stdio.h>
#include <sys/types.h>

#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif

#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif

#ifdef HAVE_SYS_LIMITS_H
#include <sys/limits.h>
#endif

/* Generate bits/local_lim.h from the values defined in the system's
   headers.  */

struct param
  {
    char *name;
    int value;
  };

static struct param params[] =
  {

#if !defined (ARG_MAX) && defined (NCARGS)
#define ARG_MAX NCARGS
#endif
#ifdef ARG_MAX
    { "ARG_MAX", ARG_MAX },
#endif

#if !defined (CHILD_MAX) && defined (MAXUPRC)
#define CHILD_MAX MAXUPRC
#endif
#ifdef CHILD_MAX
    { "CHILD_MAX", CHILD_MAX },
#endif

#if !defined (LINK_MAX) && defined (MAXLINK)
#define LINK_MAX MAXLINK
#endif
#ifdef LINK_MAX
    { "LINK_MAX", LINK_MAX },
#endif

#if !defined (OPEN_MAX) && defined (NOFILE)
#define OPEN_MAX NOFILE
#endif
#ifdef OPEN_MAX
    { "OPEN_MAX", OPEN_MAX },
#endif

#if !defined (MAX_CANON) && defined (CANBSIZ)
#define MAX_CANON CANBSIZ
#endif
#ifdef MAX_CANON
    { "MAX_CANON", MAX_CANON },
#endif

#if !defined (NAME_MAX) && defined (MAXNAMLEN)
#define NAME_MAX MAXNAMLEN
#endif
#ifndef NAME_MAX
#define NAME_MAX	255	/* XXX ? */
#endif
    { "NAME_MAX", NAME_MAX },

#if !defined (PATH_MAX) && defined (MAXPATHLEN)
#define PATH_MAX MAXPATHLEN
#endif
#ifdef PATH_MAX
    { "PATH_MAX", PATH_MAX },
#endif

#if !defined (SYMLOOP_MAX) && defined (MAXSYMLINKS)
#define SYMLOOP_MAX MAXSYMLINKS
#endif
#ifdef SYMLOOP_MAX
    { "SYMLOOP_MAX", SYMLOOP_MAX },
#endif

    { NULL, 0 }
  };

int
main()
{
  extern char *ctime ();
  extern time_t time ();
  time_t now = time ((time_t *) NULL);
  register struct param *p;

  if (! params[0].name)
    /* We have no information to give, so let the caller know.  */
    exit (1);

  printf ("\
/* Implementation-specific limits.\n\
   Generated at %.24s.  */\n\n", ctime (&now));

  for (p = params; p->name != NULL; ++p)
    printf ("#define %s %d\n", p->name, p->value);

  exit (0);
}
