/* Copyright (C) 1991, 1997 Free Software Foundation, Inc.
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

#include <alloca.h>
#include <unistd.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>

/* Execute PATH with all arguments after PATH until a NULL pointer,
   and the argument after that for environment.  */
int
execle (const char *path, const char *arg, ...)
{
  size_t argv_max = 1024;
  const char **argv = alloca (argv_max * sizeof (const char *));
  const char *const *envp;
  unsigned int i;
  va_list args;
  argv[0] = arg;

  va_start(args, arg);
  i = 1;
  do
    {
      if (i == argv_max)
	{
	  const char **nptr = alloca ((argv_max *= 2) * sizeof (const char *));

	  if ((char *) nptr + argv_max == (char *) argv)
	    {
	      /* Stack grows down.  */
	      argv = (const char **) memcpy (nptr, argv, i);
	      argv_max += i;
	    }
	  else if ((char *) argv + i == (char *) nptr)
	    /* Stack grows up.  */
	    argv_max += i;
	  else
	    /* We have a hole in the stack.  */
	    argv = (const char **) memcpy (nptr, argv, i);
	}

      argv[i] = va_arg (args, const char *);
    }
  while (argv[i++] != NULL);

  envp = va_arg(args, const char *const *);
  va_end(args);

  return __execve(path, (char *const *) argv, (char *const *) envp);
}
