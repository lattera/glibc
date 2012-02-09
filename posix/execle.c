/* Copyright (C) 1991,97,98,99,2002,2005 Free Software Foundation, Inc.
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

#include <unistd.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <stackinfo.h>

/* Execute PATH with all arguments after PATH until a NULL pointer,
   and the argument after that for environment.  */
int
execle (const char *path, const char *arg, ...)
{
#define INITIAL_ARGV_MAX 1024
  size_t argv_max = INITIAL_ARGV_MAX;
  const char *initial_argv[INITIAL_ARGV_MAX];
  const char **argv = initial_argv;
  va_list args;
  argv[0] = arg;

  va_start (args, arg);
  unsigned int i = 0;
  while (argv[i++] != NULL)
    {
      if (i == argv_max)
	{
	  argv_max *= 2;
	  const char **nptr = realloc (argv == initial_argv ? NULL : argv,
				       argv_max * sizeof (const char *));
	  if (nptr == NULL)
	    {
	      if (argv != initial_argv)
		free (argv);
	      return -1;
	    }
	  if (argv == initial_argv)
	    /* We have to copy the already filled-in data ourselves.  */
	    memcpy (nptr, argv, i * sizeof (const char *));

	  argv = nptr;
	}

      argv[i] = va_arg (args, const char *);
    }

  const char *const *envp = va_arg (args, const char *const *);
  va_end (args);

  int ret = __execve (path, (char *const *) argv, (char *const *) envp);
  if (argv != initial_argv)
    free (argv);

  return ret;
}
libc_hidden_def (execle)
