/* Determine directory for shm/sem files.  Linux version.
   Copyright (C) 2000-2018 Free Software Foundation, Inc.
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

#include "shm-directory.h"

#include <errno.h>
#include <mntent.h>
#include <paths.h>
#include <stdio.h>
#include <string.h>
#include <sys/statfs.h>
#include <libc-lock.h>
#include "linux_fsinfo.h"


/* Mount point of the shared memory filesystem.  */
static struct
{
  char *dir;
  size_t dirlen;
} mountpoint;

/* This is the default directory.  */
static const char defaultdir[] = "/dev/shm/";

/* Protect the `mountpoint' variable above.  */
__libc_once_define (static, once);


/* Determine where the shmfs is mounted (if at all).  */
static void
where_is_shmfs (void)
{
  char buf[512];
  struct statfs f;
  struct mntent resmem;
  struct mntent *mp;
  FILE *fp;

  /* The canonical place is /dev/shm.  This is at least what the
     documentation tells everybody to do.  */
  if (__statfs (defaultdir, &f) == 0 && (f.f_type == SHMFS_SUPER_MAGIC
                                         || f.f_type == RAMFS_MAGIC))
    {
      /* It is in the normal place.  */
      mountpoint.dir = (char *) defaultdir;
      mountpoint.dirlen = sizeof (defaultdir) - 1;

      return;
    }

  /* OK, do it the hard way.  Look through the /proc/mounts file and if
     this does not exist through /etc/fstab to find the mount point.  */
  fp = __setmntent ("/proc/mounts", "r");
  if (__glibc_unlikely (fp == NULL))
    {
      fp = __setmntent (_PATH_MNTTAB, "r");
      if (__glibc_unlikely (fp == NULL))
        /* There is nothing we can do.  Blind guesses are not helpful.  */
        return;
    }

  /* Now read the entries.  */
  while ((mp = __getmntent_r (fp, &resmem, buf, sizeof buf)) != NULL)
    /* The original name is "shm" but this got changed in early Linux
       2.4.x to "tmpfs".  */
    if (strcmp (mp->mnt_type, "tmpfs") == 0
        || strcmp (mp->mnt_type, "shm") == 0)
      {
        /* Found it.  There might be more than one place where the
           filesystem is mounted but one is enough for us.  */
        size_t namelen;

        /* First make sure this really is the correct entry.  At least
           some versions of the kernel give wrong information because
           of the implicit mount of the shmfs for SysV IPC.  */
        if (__statfs (mp->mnt_dir, &f) != 0 || (f.f_type != SHMFS_SUPER_MAGIC
                                                && f.f_type != RAMFS_MAGIC))
          continue;

        namelen = strlen (mp->mnt_dir);

        if (namelen == 0)
          /* Hum, maybe some crippled entry.  Keep on searching.  */
          continue;

        mountpoint.dir = (char *) malloc (namelen + 2);
        if (mountpoint.dir != NULL)
          {
            char *cp = __mempcpy (mountpoint.dir, mp->mnt_dir, namelen);
            if (cp[-1] != '/')
              *cp++ = '/';
            *cp = '\0';
            mountpoint.dirlen = cp - mountpoint.dir;
          }

        break;
      }

  /* Close the stream.  */
  __endmntent (fp);
}


const char *
__shm_directory (size_t *len)
{
  /* Determine where the shmfs is mounted.  */
  __libc_once (once, where_is_shmfs);

  /* If we don't know the mount points there is nothing we can do.  Ever.  */
  if (__glibc_unlikely (mountpoint.dir == NULL))
    {
      __set_errno (ENOSYS);
      return NULL;
    }

  *len = mountpoint.dirlen;
  return mountpoint.dir;
}
#if IS_IN (libpthread)
hidden_def (__shm_directory)
#endif


/* Make sure the table is freed if we want to free everything before
   exiting.  */
libc_freeres_fn (freeit)
{
  if (mountpoint.dir != defaultdir)
    free (mountpoint.dir);
}
