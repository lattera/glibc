/* Determine current working directory.  Linux version.
   Copyright (C) 1997, 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1997.

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

#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>

#include <sysdep.h>
#include <sys/syscall.h>


/* The "proc" filesystem provides an easy method to retrieve the value.
   For each process, the corresponding directory contains a symbolic link
   named `cwd'.  Reading the content of this link immediate gives us the
   information.  But we have to take care for systems which do not have
   the proc filesystem mounted.  Use the POSIX implementation in this case.  */
static char *generic_getcwd (char *buf, size_t size) internal_function;

#ifdef __NR_getcwd
/* Kernel 2.1.92 introduced a third way to get the current working
   directory: a syscall.  We've got to be careful that even when
   compiling under 2.1.92+ the libc still runs under older kernels. */
extern int __syscall_getcwd (char *buf, unsigned long size);
static int no_syscall_getcwd;
static int no_new_dcache = 1;
#else
# define no_syscall_getcwd 1
static int no_new_dcache;
#endif

char *
__getcwd (char *buf, size_t size)
{
  int save_errno;
  char *path;
  int n;
  char *result;
  size_t alloc_size = size;

  if (no_syscall_getcwd && no_new_dcache)
    return generic_getcwd (buf, size);

  if (size == 0)
    {
      if (buf != NULL)
	{
	  __set_errno (EINVAL);
	  return NULL;
	}

      alloc_size = PATH_MAX;
    }

  if (buf != NULL)
    path = buf;
  else
    {
      path = malloc (alloc_size);
      if (path == NULL)
	return NULL;
    }

  save_errno = errno;

#ifdef __NR_getcwd
  if (!no_syscall_getcwd)
    {
      int retval;

      retval = INLINE_SYSCALL (getcwd, 2, path, alloc_size);
      if (retval >= 0)
	{
	  if (buf == NULL)
	    {
	      buf = realloc (path, (size_t) retval);
	      if (buf == NULL)
		/* `realloc' failed but we still have the original string.  */
		buf = path;
	    }
	  return buf;
	}

      if (errno == ENOSYS)
	{
	   no_syscall_getcwd = 1;
	   no_new_dcache = 0;	/* Now we will try the /proc method.  */
	}
      else if (errno != ERANGE || buf != NULL)
	{
	  if (buf == NULL)
	    free (path);
	  return NULL;
	}

      __set_errno (save_errno);
    }
#endif

  n = __readlink ("/proc/self/cwd", path, alloc_size - 1);
  if (n != -1)
    {
      if (path[0] == '/')
	{
	  if (n >= alloc_size - 1)
	    {
	      if (buf == NULL)
		free (path);
	      return NULL;
	    }

	  path[n] = '\0';
	  if (buf == NULL)
	    {
	      buf = realloc (path, (size_t) n + 1);
	      if (buf == NULL)
		/* `relloc' failed but we still have the original string.  */
		buf = path;
	    }
	  return buf;
	}
      else
	no_new_dcache = 1;
    }

  /* Set to no_new_dcache only if error indicates that proc doesn't exist.  */
  if (errno != EACCES && errno != ENAMETOOLONG)
    no_new_dcache = 1;

  /* Something went wrong.  Restore the error number and use the generic
     version.  */
  __set_errno (save_errno);

  /* Don't put restrictions on the length of the path unless the user does.  */
  if (size == 0)
    {
      free (path);
      path = NULL;
    }

  result = generic_getcwd (path, size);

  if (result == NULL && buf == NULL && size != 0)
    free (path);

  return result;
}
weak_alias (__getcwd, getcwd)

/* Get the code for the generic version.  */
#define GETCWD_RETURN_TYPE	static char * internal_function
#define __getcwd		generic_getcwd
#include <sysdeps/posix/getcwd.c>
