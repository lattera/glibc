/* Copyright (C) 1995, 1996, 1997 Free Software Foundation, Inc.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <fstab.h>
#include <mntent.h>
#include <stdio.h>

static FILE *fstab;
static struct mntent mntres;
static char buffer[8192];


static FILE *
fstab_stream (void)
{
  if (! fstab)
    fstab = setmntent (_PATH_FSTAB, "r");
  return fstab;
}

int
setfsent (void)
{
  if (fstab)
    {
      rewind (fstab);
      return 1;
    }
  else
    fstab = setmntent (_PATH_FSTAB, "r");
  return fstab ? 0 : 1;
}

static struct fstab *
mnt2fs (struct mntent *m)
{
  static struct fstab f;

  if (m == NULL)
    return NULL;

  f.fs_spec = m->mnt_fsname;
  f.fs_file = m->mnt_dir;
  f.fs_vfstype = m->mnt_type;
  f.fs_mntops = m->mnt_opts;
  f.fs_type = (hasmntopt (m, FSTAB_RW) ? (char *) FSTAB_RW :
	       hasmntopt (m, FSTAB_RQ) ? (char *) FSTAB_RQ :
	       hasmntopt (m, FSTAB_RO) ? (char *) FSTAB_RO :
	       hasmntopt (m, FSTAB_SW) ? (char *) FSTAB_SW :
	       hasmntopt (m, FSTAB_XX) ? (char *) FSTAB_XX :
	       (char *) "??");
  f.fs_freq = m->mnt_freq;
  f.fs_passno = m->mnt_passno;
  return &f;
}

struct fstab *
getfsent (void)
{
  FILE *s = fstab_stream ();

  if (! s)
    return NULL;

  return mnt2fs (__getmntent_r (s, &mntres, buffer, sizeof buffer));
}

struct fstab *
getfsspec (name)
     register const char *name;
{
  struct mntent *m;
  if (setfsent ())
    while (m = __getmntent_r (fstab, &mntres, buffer, sizeof buffer))
      if (!strcmp (m->mnt_fsname, name))
	return mnt2fs (m);
  return NULL;
}

struct fstab *
getfsfile (name)
     register const char *name;
{
  struct mntent *m;
  if (setfsent ())
    while (m = __getmntent_r (fstab, &mntres, buffer, sizeof buffer))
      if (!strcmp (m->mnt_dir, name))
	return mnt2fs (m);
  return NULL;
}

void
endfsent ()
{
  if (fstab)
    {
      (void) endmntent (fstab);
      fstab = NULL;
    }
}
