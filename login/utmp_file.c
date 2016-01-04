/* Copyright (C) 1996-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>
   and Paul Janzen <pcj@primenet.com>, 1996.

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

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <utmp.h>
#include <not-cancel.h>
#include <kernel-features.h>

#include "utmp-private.h"
#include "utmp-equal.h"


/* Descriptor for the file and position.  */
static int file_fd = -1;
static bool file_writable;
static off64_t file_offset;

/* Cache for the last read entry.  */
static struct utmp last_entry;


/* Locking timeout.  */
#ifndef TIMEOUT
# define TIMEOUT 10
#endif

/* Do-nothing handler for locking timeout.  */
static void timeout_handler (int signum) {};

/* LOCK_FILE(fd, type) failure_statement
     attempts to get a lock on the utmp file referenced by FD.  If it fails,
     the failure_statement is executed, otherwise it is skipped.
   LOCKING_FAILED()
     jumps into the UNLOCK_FILE macro and ensures cleanup of LOCK_FILE.
   UNLOCK_FILE(fd)
     unlocks the utmp file referenced by FD and performs the cleanup of
     LOCK_FILE.
 */
#define LOCK_FILE(fd, type) \
{									      \
  struct flock fl;							      \
  struct sigaction action, old_action;					      \
  unsigned int old_timeout;						      \
									      \
  /* Cancel any existing alarm.  */					      \
  old_timeout = alarm (0);						      \
									      \
  /* Establish signal handler.  */					      \
  action.sa_handler = timeout_handler;					      \
  __sigemptyset (&action.sa_mask);					      \
  action.sa_flags = 0;							      \
  __sigaction (SIGALRM, &action, &old_action);				      \
									      \
  alarm (TIMEOUT);							      \
									      \
  /* Try to get the lock.  */						      \
  memset (&fl, '\0', sizeof (struct flock));				      \
  fl.l_type = (type);							      \
  fl.l_whence = SEEK_SET;						      \
  if (fcntl_not_cancel ((fd), F_SETLKW, &fl) < 0)

#define LOCKING_FAILED() \
  goto unalarm_return

#define UNLOCK_FILE(fd) \
  /* Unlock the file.  */						      \
  fl.l_type = F_UNLCK;							      \
  fcntl_not_cancel ((fd), F_SETLKW, &fl);				      \
									      \
 unalarm_return:							      \
  /* Reset the signal handler and alarm.  We must reset the alarm	      \
     before resetting the handler so our alarm does not generate a	      \
     spurious SIGALRM seen by the user.  However, we cannot just set	      \
     the user's old alarm before restoring the handler, because then	      \
     it's possible our handler could catch the user alarm's SIGARLM	      \
     and then the user would never see the signal he expected.  */	      \
  alarm (0);								      \
  __sigaction (SIGALRM, &old_action, NULL);				      \
  if (old_timeout != 0)							      \
    alarm (old_timeout);						      \
} while (0)


/* Functions defined here.  */
static int setutent_file (void);
static int getutent_r_file (struct utmp *buffer, struct utmp **result);
static int getutid_r_file (const struct utmp *key, struct utmp *buffer,
			   struct utmp **result);
static int getutline_r_file (const struct utmp *key, struct utmp *buffer,
			     struct utmp **result);
static struct utmp *pututline_file (const struct utmp *data);
static void endutent_file (void);
static int updwtmp_file (const char *file, const struct utmp *utmp);

/* Jump table for file functions.  */
const struct utfuncs __libc_utmp_file_functions =
{
  setutent_file,
  getutent_r_file,
  getutid_r_file,
  getutline_r_file,
  pututline_file,
  endutent_file,
  updwtmp_file
};


#ifndef TRANSFORM_UTMP_FILE_NAME
# define TRANSFORM_UTMP_FILE_NAME(file_name) (file_name)
#endif

static int
setutent_file (void)
{
  if (file_fd < 0)
    {
      const char *file_name;

      file_name = TRANSFORM_UTMP_FILE_NAME (__libc_utmp_file_name);

#ifdef O_CLOEXEC
# define O_flags O_LARGEFILE | O_CLOEXEC
#else
# define O_flags O_LARGEFILE
#endif
      file_writable = false;
      file_fd = open_not_cancel_2 (file_name, O_RDONLY | O_flags);
      if (file_fd == -1)
	return 0;

#ifndef __ASSUME_O_CLOEXEC
# ifdef O_CLOEXEC
      if (__have_o_cloexec <= 0)
# endif
	{
	  /* We have to make sure the file is `closed on exec'.  */
	  int result = fcntl_not_cancel (file_fd, F_GETFD, 0);
	  if (result >= 0)
	    {
# ifdef O_CLOEXEC
	      if (__have_o_cloexec == 0)
		__have_o_cloexec = (result & FD_CLOEXEC) ? 1 : -1;

	      if (__have_o_cloexec < 0)
# endif
		result = fcntl_not_cancel (file_fd, F_SETFD,
					   result | FD_CLOEXEC);
	    }

	  if (result == -1)
	    {
	      close_not_cancel_no_status (file_fd);
	      return 0;
	    }
	}
#endif
    }

  __lseek64 (file_fd, 0, SEEK_SET);
  file_offset = 0;

  /* Make sure the entry won't match.  */
#if _HAVE_UT_TYPE - 0
  last_entry.ut_type = -1;
#else
  last_entry.ut_line[0] = '\177';
# if _HAVE_UT_ID - 0
  last_entry.ut_id[0] = '\0';
# endif
#endif

  return 1;
}


static int
getutent_r_file (struct utmp *buffer, struct utmp **result)
{
  ssize_t nbytes;

  assert (file_fd >= 0);

  if (file_offset == -1l)
    {
      /* Not available.  */
      *result = NULL;
      return -1;
    }

  LOCK_FILE (file_fd, F_RDLCK)
    {
      nbytes = 0;
      LOCKING_FAILED ();
    }

  /* Read the next entry.  */
  nbytes = read_not_cancel (file_fd, &last_entry, sizeof (struct utmp));

  UNLOCK_FILE (file_fd);

  if (nbytes != sizeof (struct utmp))
    {
      if (nbytes != 0)
	file_offset = -1l;
      *result = NULL;
      return -1;
    }

  /* Update position pointer.  */
  file_offset += sizeof (struct utmp);

  memcpy (buffer, &last_entry, sizeof (struct utmp));
  *result = buffer;

  return 0;
}


static int
internal_getut_r (const struct utmp *id, struct utmp *buffer,
		  bool *lock_failed)
{
  int result = -1;

  LOCK_FILE (file_fd, F_RDLCK)
    {
      *lock_failed = true;
      LOCKING_FAILED ();
    }

#if _HAVE_UT_TYPE - 0
  if (id->ut_type == RUN_LVL || id->ut_type == BOOT_TIME
      || id->ut_type == OLD_TIME || id->ut_type == NEW_TIME)
    {
      /* Search for next entry with type RUN_LVL, BOOT_TIME,
	 OLD_TIME, or NEW_TIME.  */

      while (1)
	{
	  /* Read the next entry.  */
	  if (read_not_cancel (file_fd, buffer, sizeof (struct utmp))
	      != sizeof (struct utmp))
	    {
	      __set_errno (ESRCH);
	      file_offset = -1l;
	      goto unlock_return;
	    }
	  file_offset += sizeof (struct utmp);

	  if (id->ut_type == buffer->ut_type)
	    break;
	}
    }
  else
#endif /* _HAVE_UT_TYPE */
    {
      /* Search for the next entry with the specified ID and with type
	 INIT_PROCESS, LOGIN_PROCESS, USER_PROCESS, or DEAD_PROCESS.  */

      while (1)
	{
	  /* Read the next entry.  */
	  if (read_not_cancel (file_fd, buffer, sizeof (struct utmp))
	      != sizeof (struct utmp))
	    {
	      __set_errno (ESRCH);
	      file_offset = -1l;
	      goto unlock_return;
	    }
	  file_offset += sizeof (struct utmp);

	  if (__utmp_equal (buffer, id))
	    break;
	}
    }

  result = 0;

unlock_return:
  UNLOCK_FILE (file_fd);

  return result;
}


/* For implementing this function we don't use the getutent_r function
   because we can avoid the reposition on every new entry this way.  */
static int
getutid_r_file (const struct utmp *id, struct utmp *buffer,
		struct utmp **result)
{
  assert (file_fd >= 0);

  if (file_offset == -1l)
    {
      *result = NULL;
      return -1;
    }

  /* We don't have to distinguish whether we can lock the file or
     whether there is no entry.  */
  bool lock_failed = false;
  if (internal_getut_r (id, &last_entry, &lock_failed) < 0)
    {
      *result = NULL;
      return -1;
    }

  memcpy (buffer, &last_entry, sizeof (struct utmp));
  *result = buffer;

  return 0;
}


/* For implementing this function we don't use the getutent_r function
   because we can avoid the reposition on every new entry this way.  */
static int
getutline_r_file (const struct utmp *line, struct utmp *buffer,
		  struct utmp **result)
{
  assert (file_fd >= 0);

  if (file_offset == -1l)
    {
      *result = NULL;
      return -1;
    }

  LOCK_FILE (file_fd, F_RDLCK)
    {
      *result = NULL;
      LOCKING_FAILED ();
    }

  while (1)
    {
      /* Read the next entry.  */
      if (read_not_cancel (file_fd, &last_entry, sizeof (struct utmp))
	  != sizeof (struct utmp))
	{
	  __set_errno (ESRCH);
	  file_offset = -1l;
	  *result = NULL;
	  goto unlock_return;
	}
      file_offset += sizeof (struct utmp);

      /* Stop if we found a user or login entry.  */
      if (
#if _HAVE_UT_TYPE - 0
	  (last_entry.ut_type == USER_PROCESS
	   || last_entry.ut_type == LOGIN_PROCESS)
	  &&
#endif
	  !strncmp (line->ut_line, last_entry.ut_line, sizeof line->ut_line))
	break;
    }

  memcpy (buffer, &last_entry, sizeof (struct utmp));
  *result = buffer;

unlock_return:
  UNLOCK_FILE (file_fd);

  return ((*result == NULL) ? -1 : 0);
}


static struct utmp *
pututline_file (const struct utmp *data)
{
  struct utmp buffer;
  struct utmp *pbuf;
  int found;

  assert (file_fd >= 0);

  if (! file_writable)
    {
      /* We must make the file descriptor writable before going on.  */
      const char *file_name = TRANSFORM_UTMP_FILE_NAME (__libc_utmp_file_name);

      int new_fd = open_not_cancel_2 (file_name, O_RDWR | O_flags);
      if (new_fd == -1)
	return NULL;

#ifndef __ASSUME_O_CLOEXEC
# ifdef O_CLOEXEC
      if (__have_o_cloexec <= 0)
# endif
	{
	  /* We have to make sure the file is `closed on exec'.  */
	  int result = fcntl_not_cancel (file_fd, F_GETFD, 0);
	  if (result >= 0)
	    {
# ifdef O_CLOEXEC
	      if (__have_o_cloexec == 0)
		__have_o_cloexec = (result & FD_CLOEXEC) ? 1 : -1;

	      if (__have_o_cloexec < 0)
# endif
		result = fcntl_not_cancel (file_fd, F_SETFD,
					   result | FD_CLOEXEC);
	    }

	  if (result == -1)
	    {
	      close_not_cancel_no_status (file_fd);
	      return NULL;
	    }
	}
#endif

      if (__lseek64 (new_fd, __lseek64 (file_fd, 0, SEEK_CUR), SEEK_SET) == -1
	  || __dup2 (new_fd, file_fd) < 0)
	{
	  close_not_cancel_no_status (new_fd);
	  return NULL;
	}
      close_not_cancel_no_status (new_fd);
      file_writable = true;
    }

  /* Find the correct place to insert the data.  */
  if (file_offset > 0
      && (
#if _HAVE_UT_TYPE - 0
	  (last_entry.ut_type == data->ut_type
	   && (last_entry.ut_type == RUN_LVL
	       || last_entry.ut_type == BOOT_TIME
	       || last_entry.ut_type == OLD_TIME
	       || last_entry.ut_type == NEW_TIME))
	  ||
#endif
	  __utmp_equal (&last_entry, data)))
    found = 1;
  else
    {
      bool lock_failed = false;
      found = internal_getut_r (data, &buffer, &lock_failed);

      if (__builtin_expect (lock_failed, false))
	{
	  __set_errno (EAGAIN);
	  return NULL;
	}
    }

  LOCK_FILE (file_fd, F_WRLCK)
    {
      pbuf = NULL;
      LOCKING_FAILED ();
    }

  if (found < 0)
    {
      /* We append the next entry.  */
      file_offset = __lseek64 (file_fd, 0, SEEK_END);
      if (file_offset % sizeof (struct utmp) != 0)
	{
	  file_offset -= file_offset % sizeof (struct utmp);
	  __ftruncate64 (file_fd, file_offset);

	  if (__lseek64 (file_fd, 0, SEEK_END) < 0)
	    {
	      pbuf = NULL;
	      goto unlock_return;
	    }
	}
    }
  else
    {
      /* We replace the just read entry.  */
      file_offset -= sizeof (struct utmp);
      __lseek64 (file_fd, file_offset, SEEK_SET);
    }

  /* Write the new data.  */
  if (write_not_cancel (file_fd, data, sizeof (struct utmp))
      != sizeof (struct utmp))
    {
      /* If we appended a new record this is only partially written.
	 Remove it.  */
      if (found < 0)
	(void) __ftruncate64 (file_fd, file_offset);
      pbuf = NULL;
    }
  else
    {
      file_offset += sizeof (struct utmp);
      pbuf = (struct utmp *) data;
    }

 unlock_return:
  UNLOCK_FILE (file_fd);

  return pbuf;
}


static void
endutent_file (void)
{
  assert (file_fd >= 0);

  close_not_cancel_no_status (file_fd);
  file_fd = -1;
}


static int
updwtmp_file (const char *file, const struct utmp *utmp)
{
  int result = -1;
  off64_t offset;
  int fd;

  /* Open WTMP file.  */
  fd = open_not_cancel_2 (file, O_WRONLY | O_LARGEFILE);
  if (fd < 0)
    return -1;

  LOCK_FILE (fd, F_WRLCK)
    LOCKING_FAILED ();

  /* Remember original size of log file.  */
  offset = __lseek64 (fd, 0, SEEK_END);
  if (offset % sizeof (struct utmp) != 0)
    {
      offset -= offset % sizeof (struct utmp);
      __ftruncate64 (fd, offset);

      if (__lseek64 (fd, 0, SEEK_END) < 0)
	goto unlock_return;
    }

  /* Write the entry.  If we can't write all the bytes, reset the file
     size back to the original size.  That way, no partial entries
     will remain.  */
  if (write_not_cancel (fd, utmp, sizeof (struct utmp))
      != sizeof (struct utmp))
    {
      __ftruncate64 (fd, offset);
      goto unlock_return;
    }

  result = 0;

unlock_return:
  UNLOCK_FILE (fd);

  /* Close WTMP file.  */
  close_not_cancel_no_status (fd);

  return result;
}
