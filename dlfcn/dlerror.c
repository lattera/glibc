/* Return error detail for failing <dlfcn.h> functions.
   Copyright (C) 1995, 96, 97, 98, 99, 2000 Free Software Foundation, Inc.
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

#include <dlfcn.h>
#include <libintl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bits/libc-lock.h>

/* Type for storing results of dynamic loading actions.  */
struct dl_action_result
  {
    int errcode;
    int returned;
    const char *objname;
    const char *errstring;
  };
static struct dl_action_result last_result;
static struct dl_action_result *static_buf;

/* This is the key for the thread specific memory.  */
static __libc_key_t key;

/* Destructor for the thread-specific data.  */
static void init (void);
static void free_key_mem (void *mem);


char *
dlerror (void)
{
  char *buf = NULL;
  struct dl_action_result *result;

  /* Get error string.  */
  result = (struct dl_action_result *) __libc_getspecific (key);
  if (result == NULL)
    result = &last_result;

  /* Test whether we already returned the string.  */
  if (result->returned != 0)
    {
      /* We can now free the string.  */
      if (result->errstring != NULL)
	{
	  if (strcmp (result->errstring, "out of memory") != 0)
	    free ((char *) result->errstring);
	  result->errstring = NULL;
	}
    }
  else if (result->errstring != NULL)
    {
      buf = (char *) result->errstring;
      if (__asprintf (&buf, result->errcode != 0 ? "%s: %s: %s" : "%s: %s",
		      result->objname, _(result->errstring),
		      strerror (result->errcode)) != -1)
	{
	  /* We don't need the error string anymore.  */
	  if (strcmp (result->errstring, "out of memory") != 0)
	    free ((char *) result->errstring);
	  result->errstring = buf;
	}

      /* Mark the error as returned.  */
      result->returned = 1;
    }

  return buf;
}

int
internal_function
_dlerror_run (void (*operate) (void *), void *args)
{
  __libc_once_define (static, once);
  struct dl_action_result *result;

  /* If we have not yet initialized the buffer do it now.  */
  __libc_once (once, init);

  /* Get error string and number.  */
  if (static_buf != NULL)
    result = static_buf;
  else
    {
      /* We don't use the static buffer and so we have a key.  Use it
	 to get the thread-specific buffer.  */
      result = __libc_getspecific (key);
      if (result == NULL)
	{
	  result = (struct dl_action_result *) calloc (1, sizeof (*result));
	  if (result == NULL)
	    /* We are out of memory.  Since this is no really critical
	       situation we carry on by using the global variable.
	       This might lead to conflicts between the threads but
	       they soon all will have memory problems.  */
	    result = &last_result;
	  else
	    /* Set the tsd.  */
	    __libc_setspecific (key, result);
	}
    }

  if (result->errstring != NULL)
    {
      /* Free the error string from the last failed command.  This can
	 happen if `dlerror' was not run after an error was found.  */
      if (strcmp (result->errstring, "out of memory") != 0)
	free ((char *) result->errstring);
      result->errstring = NULL;
    }

  result->errcode = _dl_catch_error (&result->objname, &result->errstring,
				     operate, args);

  /* If no error we mark that no error string is available.  */
  result->returned = result->errstring == NULL;

  return result->errstring != NULL;
}


/* Initialize buffers for results.  */
static void
init (void)
{
  if (__libc_key_create (&key, free_key_mem))
    /* Creating the key failed.  This means something really went
       wrong.  In any case use a static buffer which is better than
       nothing.  */
    static_buf = &last_result;
}


/* Free the thread specific data, this is done if a thread terminates.  */
static void
free_key_mem (void *mem)
{
  struct dl_action_result *result = (struct dl_action_result *) mem;

  if (result->errstring != NULL
      && strcmp (result->errstring, "out of memory") != 0)
    free ((char *) result->errstring);

  free (mem);
  __libc_setspecific (key, NULL);
}
