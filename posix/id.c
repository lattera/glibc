/* Copyright (C) 1991, 1995 Free Software Foundation, Inc.
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

#include <ansidecl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <grp.h>
#include <pwd.h>
#include <limits.h>
#include <sys/types.h>


static void
DEFUN(print_grpname, (id, parens),
      gid_t id AND int parens)
{
  CONST struct group *CONST g = getgrgid(id);
  if (g == NULL)
    {
      if (parens)
	return;
      else
	{
	  fprintf(stderr, _("Couldn't find name for group %d\n"), id);
	  exit(EXIT_FAILURE);
	}
    }

  if (parens)
    printf("(%s)", g->gr_name);
  else
    puts(g->gr_name);
}

static void
DEFUN(print_pwdname, (id, parens),
      uid_t id AND int parens)
{
  CONST struct passwd *CONST p = getpwuid(id);
  if (p == NULL)
    {
      if (parens)
	return;
      else
	{
	  fprintf(stderr, _("Couldn't find name for user %d\n"), (int) id);
	  exit(EXIT_FAILURE);
	}
    }

  if (parens)
    printf("(%s)", p->pw_name);
  else
    puts(p->pw_name);
}

int
DEFUN(main, (argc, argv), int argc AND char **argv)
{
  int print_gid = 1, print_uid = 1;
  int real = 0, name = 0;
  int error = 0;
  register int c;

  uid_t ruid = getuid(), euid = geteuid();
  gid_t rgid = getgid(), egid = getegid();

  while ((c = getopt(argc, argv, "gurn")) != -1)
    switch (c)
      {
      default:
	error = 1;
	break;

      case 'g':
	print_gid = 1;
	print_uid = 0;
	break;

      case 'u':
	print_uid = 1;
	print_gid = 0;
	break;

      case 'r':
	real = 1;
	break;

      case 'n':
	name = 1;
	break;
      }

  if (error || argc != optind)
    {
      fputs(_("Usage: id [-gurn]\n"), stderr);
      exit(EXIT_FAILURE);
    }

  if (print_uid && !print_gid)
    {
      CONST uid_t uid = real ? ruid : euid;
      if (name)
	print_pwdname(uid, 0);
      else
	printf("%d\n", (int) uid);
    }
  else if (print_gid && !print_uid)
    {
      CONST gid_t gid = real ? rgid : egid;
      if (name)
	print_grpname(gid, 0);
      else
	printf("%d\n", (int) gid);
    }
  else
    {
#if	NGROUPS_MAX > 0
      gid_t groups[NGROUPS_MAX];
      int ngroups;
      ngroups = getgroups(NGROUPS_MAX, groups);
#endif

      printf("uid=%d", (int) ruid);
      print_pwdname(ruid, 1);
      printf(" gid=%d", (int) rgid);
      print_grpname(rgid, 1);
      if (euid != ruid)
	{
	  printf(" euid=%d", (int) euid);
	  print_pwdname(euid, 1);
	}
      if (egid != rgid)
	{
	  printf(" egid=%d", (int) egid);
	  print_grpname(egid, 1);
	}

#if	NGROUPS > 0
      if (ngroups > 0)
	{
	  register size_t i;
	  printf(" groups=%d", (int) groups[0]);
	  print_grpname(groups[0], 1);
	  for (i = 1; i < ngroups; ++i)
	    {
	      printf(", %d", (int) groups[i]);
	      print_grpname(groups[i], 1);
	    }
	}
#endif

      putchar('\n');
    }

  exit(EXIT_SUCCESS);
}
