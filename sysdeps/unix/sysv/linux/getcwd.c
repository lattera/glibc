/* Determine current working directory.  Linux version.
   Copyright (C) 1997-2012 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1997.

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
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/param.h>

#include <sysdep.h>
#include <sys/syscall.h>
#include <bp-checks.h>

#include <kernel-features.h>


/* If we compile the file for use in ld.so we don't need the feature
   that getcwd() allocates the buffers itself.  */
#ifdef IS_IN_rtld
# define NO_ALLOCATION	1
#endif


#if __ASSUME_GETCWD_SYSCALL > 0
/* Kernel 2.1.92 introduced a third way to get the current working
   directory: a syscall.  We've got to be careful that even when
   compiling under 2.1.92+ the libc still runs under older kernels. */
# define no_syscall_getcwd 0
# define have_new_dcache 1
#else
# if __NR_getcwd
/* Kernel 2.1.92 introduced a third way to get the current working
   directory: a syscall.  We've got to be careful that even when
   compiling under 2.1.92+ the libc still runs under older kernels.
   An additional problem is that the system call does not return
   the path of directories longer than one page.  */
static int no_syscall_getcwd;
static int have_new_dcache;
# else
#  define no_syscall_getcwd 1
static int have_new_dcache = 1;
# endif
#endif

/* The "proc" filesystem provides an easy method to retrieve the value.
   For each process, the corresponding directory contains a symbolic link
   named `cwd'.  Reading the content of this link immediate gives us the
   information.  But we have to take care for systems which do not have
   the proc filesystem mounted.  Use the POSIX implementation in this case.  */
static char *generic_getcwd (char *buf, size_t size) internal_function;

char *
__getcwd (char *buf, size_t size)
{
  char *path;
  int n;
  char *result;

  if (no_syscall_getcwd && !have_new_dcache)
    return generic_getcwd (buf, size);

#ifndef NO_ALLOCATION
  size_t alloc_size = size;
  if (size == 0)
    {
      if (buf != NULL)
	{
	  __set_errno (EINVAL);
	  return NULL;
	}

      alloc_size = MAX (PATH_MAX, __getpagesize ());
    }

  if (buf == NULL)
    {
      path = malloc (alloc_size);
      if (path == NULL)
	return NULL;
    }
  else
#else
# define alloc_size size
#endif
    path = buf;

#if defined __NR_getcwd || __LINUX_GETCWD_SYSCALL > 0
  if (!no_syscall_getcwd)
    {
      int retval;

      retval = INLINE_SYSCALL (getcwd, 2, CHECK_STRING (path), alloc_size);
      if (retval >= 0)
	{
# ifndef NO_ALLOCATION
	  if (buf == NULL && size == 0)
	    /* Ensure that the buffer is only as large as necessary.  */
	    buf = realloc (path, (size_t) retval);

	  if (buf == NULL)
	    /* Either buf was NULL all along, or `realloc' failed but
	       we still have the original string.  */
	    buf = path;
# endif

	  return buf;
	}

      /* The system call cannot handle paths longer than a page.
	 Neither can the magic symlink in /proc/self.  Just use the
	 generic implementation right away.  */
      if (errno == ENAMETOOLONG)
	{
# ifndef NO_ALLOCATION
	  if (buf == NULL && size == 0)
	    {
	      free (path);
	      path = NULL;
	    }
# endif

	  result = generic_getcwd (path, size);

# ifndef NO_ALLOCATION
	  if (result == NULL && buf == NULL && size != 0)
	    free (path);
# endif

	  return result;
	}

# if __ASSUME_GETCWD_SYSCALL
      /* It should never happen that the `getcwd' syscall failed because
	 the buffer is too small if we allocated the buffer ourselves
	 large enough.  */
      assert (errno != ERANGE || buf != NULL || size != 0);

#  ifndef NO_ALLOCATION
      if (buf == NULL)
	free (path);
#  endif

      return NULL;
# else
      if (errno == ENOSYS)
	{
	   no_syscall_getcwd = 1;
	   have_new_dcache = 1;	/* Now we will try the /proc method.  */
	}
      else if (errno != ERANGE || buf != NULL)
	{
#  ifndef NO_ALLOCATION
	  if (buf == NULL)
	    free (path);
#  endif
	  return NULL;
	}
# endif
    }
#endif

  n = __readlink ("/proc/self/cwd", path, alloc_size - 1);
  if (n != -1)
    {
      if (path[0] == '/')
	{
	  if ((size_t) n >= alloc_size - 1)
	    {
#ifndef NO_ALLOCATION
	      if (buf == NULL)
		free (path);
#endif
	      return NULL;
	    }

	  path[n] = '\0';
#ifndef NO_ALLOCATION
	  if (buf == NULL && size == 0)
	    /* Ensure that the buffer is only as large as necessary.  */
	    buf = realloc (path, (size_t) n + 1);
	  if (buf == NULL)
	    /* Either buf was NULL all along, or `realloc' failed but
	       we still have the original string.  */
	    buf = path;
#endif

	  return buf;
	}
#ifndef have_new_dcache
      else
	have_new_dcache = 0;
#endif
    }

#if __ASSUME_GETCWD_SYSCALL == 0
  /* Set to have_new_dcache only if error indicates that proc doesn't
     exist.  */
  if (errno != EACCES && errno != ENAMETOOLONG)
    have_new_dcache = 0;
#endif

#ifndef NO_ALLOCATION
  /* Don't put restrictions on the length of the path unless the user does.  */
  if (buf == NULL && size == 0)
    {
      free (path);
      path = NULL;
    }
#endif

  result = generic_getcwd (path, size);

#ifndef NO_ALLOCATION
  if (result == NULL && buf == NULL && size != 0)
    free (path);
#endif

  return result;
}
weak_alias (__getcwd, getcwd)

/* Get the code for the generic version.  */
#define GETCWD_RETURN_TYPE	static char * internal_function
#define __getcwd		generic_getcwd
#include <sysdeps/posix/getcwd.c>
