/* You don't really want to know what this hack is for.
   Copyright (C) 1996, 1997 Free Software Foundation, Inc.
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

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>

static void *funcall (char **stringp);
static void *eval (char **stringp);

static void *
funcall (char **stringp)
{
  void *args[strlen (*stringp)], **ap = args;
  void *argcookie = &args[1];

  do
    {
      /* Evaluate the next token.  */
      *ap++ = eval (stringp);

      /* Whitespace is irrelevant.  */
      while (isspace (**stringp))
	++*stringp;

      /* Terminate at closing paren or end of line.  */
    } while (**stringp != '\0' && **stringp != ')');
  if (**stringp != '\0')
    /* Swallow closing paren.  */
    ++*stringp;

  if (args[0] == NULL)
    {
      static const char unknown[] = "Unknown function\n";
      write (1, unknown, sizeof unknown - 1);
      return NULL;
    }

  /* Do it to it.  */
  __builtin_return (__builtin_apply (args[0],
				     &argcookie,
				     (char *) ap - (char *) &args[1]));
}

static void *
eval (char **stringp)
{
  void *value;
  char *p = *stringp, c;

  /* Whitespace is irrelevant.  */
  while (isspace (*p))
    ++p;

  switch (*p)
    {
    case '"':
      /* String constant.  */
      value = ++p;
      do
	if (*p == '\\')
	  {
	    switch (*strcpy (p, p + 1))
	      {
	      case 't':
		*p = '\t';
		break;
	      case 'n':
		*p = '\n';
		break;
	      }
	    ++p;
	  }
      while (*p != '\0' && *p++ != '"');
      if (p[-1] == '"')
	p[-1] = '\0';
      break;

    case '(':
      *stringp = ++p;
      return funcall (stringp);

    default:
      /* Try to parse it as a number.  */
      value = (void *) strtol (p, stringp, 0);
      if (*stringp != p)
	return value;

      /* Anything else is a symbol that produces its address.  */
      value = p;
      do
	++p;
      while (*p != '\0' && !isspace (*p) && (!ispunct (*p) || *p == '_'));
      c = *p;
      *p = '\0';
      value = dlsym (NULL, value);
      *p = c;
      break;
    }

  *stringp = p;
  return value;
}


void
_start (void)
{
  char *buf = NULL;
  size_t bufsz = 0;

  while (__getdelim (&buf, &bufsz, '\n', stdin) > 0)
    {
      char *p = buf;
      eval (&p);
    }

  exit (0);
}
