/* Return error detail for failing <dlfcn.h> functions.
   Copyright (C) 1995-2018 Free Software Foundation, Inc.
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

#include <dlfcn.h>
#include <libintl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libc-lock.h>
#include <ldsodefs.h>
#include <libc-symbols.h>

#if !defined SHARED && IS_IN (libdl)

char *
dlerror (void)
{
  return __dlerror ();
}

#else

/* Type for storing results of dynamic loading actions.  */
struct dl_action_result
  {
    int errcode;
    int returned;
    bool malloced;
    const char *objname;
    const char *errstring;
  };
static struct dl_action_result last_result;
static struct dl_action_result *static_buf;

/* This is the key for the thread specific memory.  */
static __libc_key_t key;
__libc_once_define (static, once);

/* Destructor for the thread-specific data.  */
static void init (void);
static void free_key_mem (void *mem);


char *
__dlerror (void)
{
  char *buf = NULL;
  struct dl_action_result *result;

# ifdef SHARED
  if (!rtld_active ())
    return _dlfcn_hook->dlerror ();
# endif

  /* If we have not yet initialized the buffer do it now.  */
  __libc_once (once, init);

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
      int n;
      if (result->errcode == 0)
	n = __asprintf (&buf, "%s%s%s",
			result->objname,
			result->objname[0] == '\0' ? "" : ": ",
			_(result->errstring));
      else
	n = __asprintf (&buf, "%s%s%s: %s",
			result->objname,
			result->objname[0] == '\0' ? "" : ": ",
			_(result->errstring),
			strerror (result->errcode));
      if (n != -1)
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
# ifdef SHARED
strong_alias (__dlerror, dlerror)
# endif

int
_dlerror_run (void (*operate) (void *), void *args)
{
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
      if (result->malloced)
	free ((char *) result->errstring);
      result->errstring = NULL;
    }

  result->errcode = _dl_catch_error (&result->objname, &result->errstring,
				     &result->malloced, operate, args);

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


static void
check_free (struct dl_action_result *rec)
{
  if (rec->errstring != NULL
      && strcmp (rec->errstring, "out of memory") != 0)
    {
      /* We can free the string only if the allocation happened in the
	 C library used by the dynamic linker.  This means, it is
	 always the C library in the base namespace.  When we're statically
         linked, the dynamic linker is part of the program and so always
	 uses the same C library we use here.  */
#ifdef SHARED
      struct link_map *map = NULL;
      Dl_info info;
      if (_dl_addr (check_free, &info, &map, NULL) != 0 && map->l_ns == 0)
#endif
	free ((char *) rec->errstring);
    }
}


static void
__attribute__ ((destructor))
fini (void)
{
  check_free (&last_result);
}


/* Free the thread specific data, this is done if a thread terminates.  */
static void
free_key_mem (void *mem)
{
  check_free ((struct dl_action_result *) mem);

  free (mem);
  __libc_setspecific (key, NULL);
}

# ifdef SHARED

/* Free the dlerror-related resources.  */
void
__dlerror_main_freeres (void)
{
  void *mem;
  /* Free the global memory if used.  */
  check_free (&last_result);
  /* Free the TSD memory if used.  */
  mem = __libc_getspecific (key);
  if (mem != NULL)
    free_key_mem (mem);
}

struct dlfcn_hook *_dlfcn_hook __attribute__((nocommon));
libdl_hidden_data_def (_dlfcn_hook)

# else

static struct dlfcn_hook _dlfcn_hooks =
  {
    .dlopen = __dlopen,
    .dlclose = __dlclose,
    .dlsym = __dlsym,
    .dlvsym = __dlvsym,
    .dlerror = __dlerror,
    .dladdr = __dladdr,
    .dladdr1 = __dladdr1,
    .dlinfo = __dlinfo,
    .dlmopen = __dlmopen
  };

void
__libc_register_dlfcn_hook (struct link_map *map)
{
  struct dlfcn_hook **hook;

  hook = (struct dlfcn_hook **) __libc_dlsym_private (map, "_dlfcn_hook");
  if (hook != NULL)
    *hook = &_dlfcn_hooks;
}
# endif
#endif
