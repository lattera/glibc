/* Environment handling for dynamic loader.
   Copyright (C) 1995, 1996, 1997, 1998, 2000 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <string.h>
#include <ldsodefs.h>

extern char **_environ;
extern void unsetenv (const char *name);

/* Walk through the environment of the process and return all entries
   starting with `LD_'.  */
char *
internal_function
_dl_next_ld_env_entry (char ***position)
{
  char **current = *position;
  char *result = NULL;

  if (current == NULL)
    /* We start over.  */
    current = _environ;

  while (result == NULL && *current != NULL)
    {
      if ((*current)[0] == 'L' && (*current)[1] == 'D' && (*current)[2] == '_')
	result = *current;

      ++current;
    }

  /* Save current position for next visit.  */
  *position = current;

  return result;
}

void
unsetenv (const char *name)
{
  const size_t len = strlen (name);
  char **ep;

  ep = _environ;
  while (*ep != NULL)
    if (!strncmp (*ep, name, len) && (*ep)[len] == '=')
      {
	/* Found it.  Remove this pointer by moving later ones back.  */
	char **dp = ep;

	do
	  dp[0] = dp[1];
	while (*dp++);
	/* Continue the loop in case NAME appears again.  */
      }
    else
      ++ep;
}
