/* Copyright (C) 2002, 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2002.

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

#include <errno.h>
#include <fcntl.h>
#include <mntent.h>
#include <paths.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <linux_fsinfo.h>
#include "semaphoreP.h"



/* Information about the mount point.  */
struct mountpoint_info mountpoint attribute_hidden;

/* This is the default mount point.  */
static const char defaultmount[] = "/dev/shm";
/* This is the default directory.  */
static const char defaultdir[] = "/dev/shm/sem.";

/* Protect the `mountpoint' variable above.  */
pthread_once_t __namedsem_once attribute_hidden = PTHREAD_ONCE_INIT;


/* Determine where the shmfs is mounted (if at all).  */
void
attribute_hidden
__where_is_shmfs (void)
{
  char buf[512];
  struct statfs f;
  struct mntent resmem;
  struct mntent *mp;
  FILE *fp;

  /* The canonical place is /dev/shm.  This is at least what the
     documentation tells everybody to do.  */
  if (__statfs (defaultmount, &f) == 0 && f.f_type == SHMFS_SUPER_MAGIC)
    {
      /* It is in the normal place.  */
      mountpoint.dir = (char *) defaultdir;
      mountpoint.dirlen = sizeof (defaultdir) - 1;

      return;
    }

  /* OK, do it the hard way.  Look through the /proc/mounts file and if
     this does not exist through /etc/fstab to find the mount point.  */
  fp = __setmntent ("/proc/mounts", "r");
  if (__builtin_expect (fp == NULL, 0))
    {
      fp = __setmntent (_PATH_MNTTAB, "r");
      if (__builtin_expect (fp == NULL, 0))
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
	if (__statfs (mp->mnt_dir, &f) != 0 || f.f_type != SHMFS_SUPER_MAGIC)
	  continue;

	namelen = strlen (mp->mnt_dir);

	if (namelen == 0)
	  /* Hum, maybe some crippled entry.  Keep on searching.  */
	  continue;

	mountpoint.dir = (char *) malloc (namelen + 4 + 2);
	if (mountpoint.dir != NULL)
	  {
	    char *cp = __mempcpy (mountpoint.dir, mp->mnt_dir, namelen);
	    if (cp[-1] != '/')
	      *cp++ = '/';
	    cp = stpcpy (cp, "sem.");
	    mountpoint.dirlen = cp - mountpoint.dir;
	  }

	break;
      }

  /* Close the stream.  */
  __endmntent (fp);
}


sem_t *
sem_open (const char *name, int oflag, ...)
{
  char *finalname;
  size_t namelen = SEM_FAILED;
  sem_t *result;
  int fd;

  /* Determine where the shmfs is mounted.  */
  INTUSE(__pthread_once) (&__namedsem_once, __where_is_shmfs);

  /* If we don't know the mount points there is nothing we can do.  Ever.  */
  if (mountpoint.dir == NULL)
    {
      __set_errno (ENOSYS);
      return SEM_FAILED;
    }

  /* Construct the filename.  */
  while (name[0] == '/')
    ++name;

  if (name[0] == '\0')
    {
      /* The name "/" is not supported.  */
      __set_errno (EINVAL);
      return SEM_FAILED;
    }
  namelen = strlen (name);

  /* Create the name of the final file.  */
  finalname = (char *) alloca (mountpoint.dirlen + namelen + 1);
  __mempcpy (__mempcpy (finalname, mountpoint.dir, mountpoint.dirlen),
	     name, namelen + 1);

  /* If the semaphore object has to exist simply open it.  */
  if ((oflag & O_CREAT) == 0 || (oflag & O_EXCL) == 0)
    {
    try_again:
      fd = __libc_open (finalname,
			(oflag & ~(O_CREAT|O_ACCMODE)) | O_NOFOLLOW | O_RDWR);

      if (fd == -1)
	{
	  /* If we are supposed to create the file try this next.  */
	  if ((oflag & O_CREAT) != 0)
	    goto try_create;

	  /* Return.  errno is already set.  */
	}
      else
	/* Map the sem_t structure from the file.  */
	result = (sem_t *) mmap (NULL, sizeof (sem_t), PROT_READ | PROT_WRITE,
				 MAP_SHARED, fd, 0);
    }
  else
    {
      /* We have to open a temporary file first since it must have the
	 correct form before we can start using it.  */
      char *tmpfname;
      mode_t mode;
      unsigned int value;
      va_list ap;

    try_create:
      va_start (ap, oflag);

      mode = va_arg (ap, mode_t);
      value = va_arg (ap, unsigned int);

      va_end (ap);

      if (value > SEM_VALUE_MAX)
	{
	  __set_errno (EINVAL);
	  return SEM_FAILED;
	}

      tmpfname = (char *) alloca (mountpoint.dirlen + 6 + 1);
      strcpy (__mempcpy (tmpfname, mountpoint.dir, mountpoint.dirlen),
	      "XXXXXX");

      fd = mkstemp (tmpfname);
      if (fd == -1)
	return SEM_FAILED;

      /* Create the initial file content.  */
      sem_t initsem;

      struct sem *iinitsem = (struct sem *) &initsem;
      iinitsem->count = value;

      /* Initialize the remaining bytes as well.  */
      memset ((char *) &initsem + sizeof (struct sem), '\0',
	      sizeof (sem_t) - sizeof (struct sem));

      if (TEMP_FAILURE_RETRY (__libc_write (fd, &initsem, sizeof (sem_t)))
	  == sizeof (sem_t)
	  /* Adjust the permission.  */
	  && fchmod (fd, mode) == 0
	  /* Map the sem_t structure from the file.  */
	  && (result = (sem_t *) mmap (NULL, sizeof (sem_t),
				       PROT_READ | PROT_WRITE, MAP_SHARED,
				       fd, 0)) != MAP_FAILED)
	{
	  /* Create the file.  Don't overwrite an existing file.  */
	  if (link (tmpfname, finalname) != 0)
	    {
	      /* Undo the mapping.  */
	      (void) munmap (result, sizeof (sem_t));

	      /* Reinitialize 'result'.  */
	      result = SEM_FAILED;

	      /* This failed.  If O_EXCL is not set and the problem was
		 that the file exists, try again.  */
	      if ((oflag & O_EXCL) == 0 && errno == EEXIST)
		{
		  /* Remove the file.  */
		  (void) unlink (tmpfname);

		  /* Close the file.  */
		  (void) __libc_close (fd);

		  goto try_again;
		}
	    }
	}

      /* Now remove the temporary name.  This should never fail.  If
	 it fails we leak a file name.  Better fix the kernel.  */
      (void) unlink (tmpfname);
    }

  /* Map the mmap error to the error we need.  */
  if (MAP_FAILED != (void *) SEM_FAILED && result == MAP_FAILED)
    result = SEM_FAILED;

  /* We don't need the file descriptor anymore.  */
  if (fd != -1)
    (void) __libc_close (fd);

  return result;
}
