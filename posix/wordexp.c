/* Copyright (C) 1992, 1997 Free Software Foundation, Inc.
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

#include <sys/types.h>
#include <wordexp.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>


/* We do word expansion with a pipe to the shell.
   The shell command `sh [-P] [-u] -w "words ..."' expands words.
   If -P, command substitution is an error.
   If -u, reference to an undefined variable is an error.
   The shell writes on its stdout:
   	%u\0	Number of words.
	%u\0	Number of bytes in all words together (not counting \0s).
	word1\0
	word2\0
	...
	wordN\0
   */

#define	SHELL_PATH	"/bin/sh"
#define	SHELL_NAME	"sh"


int
wordexp (string, pwordexp, flags)
     const char *string;
     wordexp_t *pwordexp;
     int flags;
{
  int error;
  pid_t pid;
  int d[2];
  int status;

  FILE *f;
  size_t wordc, start, buflen;
  char *buf;

  /* Create the pipe through which we will communicate to the shell.  */
  if (pipe (d) < 0)
    return -1;

  pid = fork ();
  if (pid < 0)
    return -1;

  if (pid == 0)
    {
      /* Child.  Run the shell.  */

      const char *argv[5];

      close (d[STDIN_FILENO]);
      dup2 (d[STDOUT_FILENO], STDOUT_FILENO);
      if (!(flags & WRDE_SHOWERR))
	close (STDERR_FILENO);

      i = 0;
      argv[i++] = SHELL_NAME;
      if (flags & WRDE_NOCMD)
	argv[i++] = "-P";
      if (flags & WRDE_UNDEF)
	argv[i++] = "-u";
      argv[i++] = "-w";
      argv[i++] = string;
      argv[i++] = NULL;

      execv (SHELL_PATH, argv);
      _exit (WRDE_NOSPACE);
    }

  /* Parent.  */

  buf = NULL;
  error = WRDE_NOSPACE;

  close (d[STDOUT_FILENO]);
  f = fdopen (d[STDIN_FILENO]);
  if (f == NULL)
    goto lose;

  /* Read the number of words and number of bytes from the shell.  */
  if (fscanf (f, "%u", &wordc) != 1 || getc (f) != '\0' ||
      fscanf (f, "%u", &buflen) != 1 || getc (f) != '\0')
    goto lose;

  /* Read the words from the shell, and wait for it to return.  */
  buflen += wordc;
  buf = malloc (buflen);
  if (buf == NULL ||
      fread (buf, buflen, 1, f) != 1 ||
      waitpid (pid, &status, 0) != pid)
    goto lose;

  if (WIFEXITED (status))
    {
      if (WEXITSTATUS (status) != 0)
	{
	  error = WEXITSTATUS (status);
	  goto lose;
	}
    }
  else
    goto lose;

  /* Pack the structure.  */

  start = 0;
  if (flags & WRDE_DOOFFS)
    start += pwordexp->we_offs;
  if (flags & WRDE_APPEND)
    start += pwordexp->we_wordc;
  wordc = start + wordc + 1;

  if (flags & WRDE_APPEND)
    wordv = (char **) realloc ((void *) pwordexp->we_wordv,
			       wordc * sizeof (char *));
  else
    wordv = (char **) malloc (wordc * sizeof (char *));
  if (wordv == NULL)
    goto lose;

  if (flags & WRDE_DOOFFS)
    for (i = 0; i < pwordexp->we_offs; ++i)
      wordv[i] = NULL;

  for (i = start; i < wordc; ++i)
    {
      pwordexp->we_wordv[i] = buf;
      buf = strchr (buf, '\0') + 1;
    }
  wordv[i] = NULL;

  if (flags & WRDE_REUSE)
    {
      free (pwordexp->we_wordv[0]);
      if (!(flags & WRDE_APPEND))
	free (pwordexp->we_wordv);
    }

  pwordexp->we_wordc = wordc;
  pwordexp->we_wordv = wordv;

  return 0;

 lose:
  {
    int save;
    save = errno;
    (void) kill (pid, SIGKILL);
    free (buf);
    (void) waitpid (pid, (int *) NULL, 0);
    errno = save;
    return error;
  }
}


void
DEFUN(wordexp, (pwordexp), wordexp_t *pwordexp)
{
  /* All the other elts point into the first.  */
  free (pwordexp->we_wordv[0]);
  free (pwordexp->we_wordv);
}
