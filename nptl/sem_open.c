/* Copyright (C) 2002-2018 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <search.h>
#include <semaphore.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "semaphoreP.h"
#include <shm-directory.h>
#include <futex-internal.h>
#include <libc-lock.h>

/* Comparison function for search of existing mapping.  */
int
attribute_hidden
__sem_search (const void *a, const void *b)
{
  const struct inuse_sem *as = (const struct inuse_sem *) a;
  const struct inuse_sem *bs = (const struct inuse_sem *) b;

  if (as->ino != bs->ino)
    /* Cannot return the difference the type is larger than int.  */
    return as->ino < bs->ino ? -1 : (as->ino == bs->ino ? 0 : 1);

  if (as->dev != bs->dev)
    /* Cannot return the difference the type is larger than int.  */
    return as->dev < bs->dev ? -1 : (as->dev == bs->dev ? 0 : 1);

  return strcmp (as->name, bs->name);
}


/* The search tree for existing mappings.  */
void *__sem_mappings attribute_hidden;

/* Lock to protect the search tree.  */
int __sem_mappings_lock attribute_hidden = LLL_LOCK_INITIALIZER;


/* Search for existing mapping and if possible add the one provided.  */
static sem_t *
check_add_mapping (const char *name, size_t namelen, int fd, sem_t *existing)
{
  sem_t *result = SEM_FAILED;

  /* Get the information about the file.  */
  struct stat64 st;
  if (__fxstat64 (_STAT_VER, fd, &st) == 0)
    {
      /* Get the lock.  */
      lll_lock (__sem_mappings_lock, LLL_PRIVATE);

      /* Search for an existing mapping given the information we have.  */
      struct inuse_sem *fake;
      fake = (struct inuse_sem *) alloca (sizeof (*fake) + namelen);
      memcpy (fake->name, name, namelen);
      fake->dev = st.st_dev;
      fake->ino = st.st_ino;

      struct inuse_sem **foundp = __tfind (fake, &__sem_mappings,
					   __sem_search);
      if (foundp != NULL)
	{
	  /* There is already a mapping.  Use it.  */
	  result = (*foundp)->sem;
	  ++(*foundp)->refcnt;
	}
      else
	{
	  /* We haven't found a mapping.  Install ione.  */
	  struct inuse_sem *newp;

	  newp = (struct inuse_sem *) malloc (sizeof (*newp) + namelen);
	  if (newp != NULL)
	    {
	      /* If the caller hasn't provided any map it now.  */
	      if (existing == SEM_FAILED)
		existing = (sem_t *) mmap (NULL, sizeof (sem_t),
					   PROT_READ | PROT_WRITE, MAP_SHARED,
					   fd, 0);

	      newp->dev = st.st_dev;
	      newp->ino = st.st_ino;
	      newp->refcnt = 1;
	      newp->sem = existing;
	      memcpy (newp->name, name, namelen);

	      /* Insert the new value.  */
	      if (existing != MAP_FAILED
		  && __tsearch (newp, &__sem_mappings, __sem_search) != NULL)
		/* Successful.  */
		result = existing;
	      else
		/* Something went wrong while inserting the new
		   value.  We fail completely.  */
		free (newp);
	    }
	}

      /* Release the lock.  */
      lll_unlock (__sem_mappings_lock, LLL_PRIVATE);
    }

  if (result != existing && existing != SEM_FAILED && existing != MAP_FAILED)
    {
      /* Do not disturb errno.  */
      int save = errno;
      munmap (existing, sizeof (sem_t));
      errno = save;
    }

  return result;
}


sem_t *
sem_open (const char *name, int oflag, ...)
{
  int fd;
  sem_t *result;

  /* Check that shared futexes are supported.  */
  int err = futex_supports_pshared (PTHREAD_PROCESS_SHARED);
  if (err != 0)
    {
      __set_errno (err);
      return SEM_FAILED;
    }

  /* Create the name of the final file in local variable SHM_NAME.  */
  SHM_GET_NAME (EINVAL, SEM_FAILED, SEM_SHM_PREFIX);

  /* Disable asynchronous cancellation.  */
#ifdef __libc_ptf_call
  int state;
  __libc_ptf_call (__pthread_setcancelstate,
                   (PTHREAD_CANCEL_DISABLE, &state), 0);
#endif

  /* If the semaphore object has to exist simply open it.  */
  if ((oflag & O_CREAT) == 0 || (oflag & O_EXCL) == 0)
    {
    try_again:
      fd = __libc_open (shm_name,
			(oflag & ~(O_CREAT|O_ACCMODE)) | O_NOFOLLOW | O_RDWR);

      if (fd == -1)
	{
	  /* If we are supposed to create the file try this next.  */
	  if ((oflag & O_CREAT) != 0 && errno == ENOENT)
	    goto try_create;

	  /* Return.  errno is already set.  */
	}
      else
	/* Check whether we already have this semaphore mapped and
	   create one if necessary.  */
	result = check_add_mapping (name, namelen, fd, SEM_FAILED);
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
	  result = SEM_FAILED;
	  goto out;
	}

      /* Create the initial file content.  */
      union
      {
	sem_t initsem;
	struct new_sem newsem;
      } sem;

#if __HAVE_64B_ATOMICS
      sem.newsem.data = value;
#else
      sem.newsem.value = value << SEM_VALUE_SHIFT;
      /* pad is used as a mutex on pre-v9 sparc and ignored otherwise.  */
      sem.newsem.pad = 0;
      sem.newsem.nwaiters = 0;
#endif
      /* This always is a shared semaphore.  */
      sem.newsem.private = FUTEX_SHARED;

      /* Initialize the remaining bytes as well.  */
      memset ((char *) &sem.initsem + sizeof (struct new_sem), '\0',
	      sizeof (sem_t) - sizeof (struct new_sem));

      tmpfname = __alloca (shm_dirlen + sizeof SEM_SHM_PREFIX + 6);
      char *xxxxxx = __mempcpy (tmpfname, shm_dir, shm_dirlen);

      int retries = 0;
#define NRETRIES 50
      while (1)
	{
	  /* Add the suffix for mktemp.  */
	  strcpy (xxxxxx, "XXXXXX");

	  /* We really want to use mktemp here.  We cannot use mkstemp
	     since the file must be opened with a specific mode.  The
	     mode cannot later be set since then we cannot apply the
	     file create mask.  */
	  if (__mktemp (tmpfname) == NULL)
	    {
	      result = SEM_FAILED;
	      goto out;
	    }

	  /* Open the file.  Make sure we do not overwrite anything.  */
	  fd = __libc_open (tmpfname, O_RDWR | O_CREAT | O_EXCL, mode);
	  if (fd == -1)
	    {
	      if (errno == EEXIST)
		{
		  if (++retries < NRETRIES)
		    continue;

		  __set_errno (EAGAIN);
		}

	      result = SEM_FAILED;
	      goto out;
	    }

	  /* We got a file.  */
	  break;
	}

      if (TEMP_FAILURE_RETRY (__libc_write (fd, &sem.initsem, sizeof (sem_t)))
	  == sizeof (sem_t)
	  /* Map the sem_t structure from the file.  */
	  && (result = (sem_t *) mmap (NULL, sizeof (sem_t),
				       PROT_READ | PROT_WRITE, MAP_SHARED,
				       fd, 0)) != MAP_FAILED)
	{
	  /* Create the file.  Don't overwrite an existing file.  */
	  if (link (tmpfname, shm_name) != 0)
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
	  else
	    /* Insert the mapping into the search tree.  This also
	       determines whether another thread sneaked by and already
	       added such a mapping despite the fact that we created it.  */
	    result = check_add_mapping (name, namelen, fd, result);
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
    {
      /* Do not disturb errno.  */
      int save = errno;
      __libc_close (fd);
      errno = save;
    }

out:
#ifdef __libc_ptf_call
  __libc_ptf_call (__pthread_setcancelstate, (state, NULL), 0);
#endif

  return result;
}
