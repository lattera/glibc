/* Copyright (C) 1991, 1992 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <sys/types.h>
#include <sys/param.h>

/* Generate local_limits.h from <sys/param.h> values for BSD.  */

struct param
  {
    char *name;
    int value;
  };

static struct param params[] =
  {
    { "NGROUPS_MAX", NGROUPS },

    { "ARG_MAX", NCARGS },

#ifdef	MAXUPRC
    { "CHILD_MAX", MAXUPRC },
#endif

    { "OPEN_MAX", NOFILE },

    /* Apparently, Ultrix's <sys/param.h> defines LINK_MAX and not MAXLINK.  */
#if defined (LINK_MAX) && !defined (MAXLINK)
#define MAXLINK	LINK_MAX
#endif
#ifdef	MAXLINK
    { "LINK_MAX", MAXLINK },
#endif

    { "MAX_CANON", CANBSIZ },

#ifndef	MAXNAMLEN
#define MAXNAMLEN 255
#endif
    { "NAME_MAX", MAXNAMLEN },

    { "PATH_MAX", MAXPATHLEN },

    { NULL, 0 }
  };

int
main()
{
  extern char *ctime();
  extern time_t time();
  time_t now = time((time_t *) NULL);
  register struct param *p;

  printf("\
/* Implementation-specific limits.\n\
   Generated at %.25s.  */\n\n", ctime(&now));

  for (p = params; p->name != NULL; ++p)
    printf("#define %s %d\n", p->name, p->value);

  exit(0);
}
