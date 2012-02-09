/* Error handling for runtime dynamic linker.
   Copyright (C) 1995-2002,2004,2005 Free Software Foundation, Inc.
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

#include <libintl.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ldsodefs.h>

/* This structure communicates state between _dl_catch_error and
   _dl_signal_error.  */
struct catch
  {
    const char *objname;	/* Object/File name.  */
    const char *errstring;	/* Error detail filled in here.  */
    bool malloced;		/* Nonzero if the string is malloced
				   by the libc malloc.  */
    jmp_buf env;		/* longjmp here on error.  */
  };

/* Multiple threads at once can use the `_dl_catch_error' function.  The
   calls can come from `_dl_map_object_deps', `_dlerror_run', or from
   any of the libc functionality which loads dynamic objects (NSS, iconv).
   Therefore we have to be prepared to save the state in thread-local
   memory.  The _dl_error_catch_tsd function pointer is reset by the thread
   library so that it returns the address of a thread-local variable.  */


/* This message we return as a last resort.  We define the string in a
   variable since we have to avoid freeing it and so have to enable
   a pointer comparison.  See below and in dlfcn/dlerror.c.  */
static const char _dl_out_of_memory[] = "out of memory";


/* This points to a function which is called when an continuable error is
   received.  Unlike the handling of `catch' this function may return.
   The arguments will be the `errstring' and `objname'.

   Since this functionality is not used in normal programs (only in ld.so)
   we do not care about multi-threaded programs here.  We keep this as a
   global variable.  */
static receiver_fct receiver;

#ifdef _LIBC_REENTRANT
# define CATCH_HOOK	(*(struct catch **) (*GL(dl_error_catch_tsd)) ())
#else
static struct catch *catch_hook;
# define CATCH_HOOK	catch_hook
#endif

void
internal_function
_dl_signal_error (int errcode, const char *objname, const char *occation,
		  const char *errstring)
{
  struct catch *lcatch;

  if (! errstring)
    errstring = N_("DYNAMIC LINKER BUG!!!");

  lcatch = CATCH_HOOK;
  if (objname == NULL)
    objname = "";
  if (lcatch != NULL)
    {
      /* We are inside _dl_catch_error.  Return to it.  We have to
	 duplicate the error string since it might be allocated on the
	 stack.  The object name is always a string constant.  */
      size_t len_objname = strlen (objname) + 1;
      size_t len_errstring = strlen (errstring) + 1;

      lcatch->errstring = (char *) malloc (len_objname + len_errstring);
      if (lcatch->errstring != NULL)
	{
	  /* Make a copy of the object file name and the error string.  */
	  lcatch->objname = memcpy (__mempcpy ((char *) lcatch->errstring,
					       errstring, len_errstring),
				    objname, len_objname);

	  /* If the main executable is relocated it means the libc's malloc
	     is used.  */
#ifdef SHARED
	  lcatch->malloced = (GL(dl_ns)[LM_ID_BASE]._ns_loaded != NULL
			      && (GL(dl_ns)[LM_ID_BASE]._ns_loaded->l_relocated
				  != 0));
#else
	  lcatch->malloced = true;
#endif
	}
      else
	{
	  /* This is better than nothing.  */
	  lcatch->objname = "";
	  lcatch->errstring = _dl_out_of_memory;
	  lcatch->malloced = false;
	}
      /* We do not restore the signal mask because none was saved.  */
      __longjmp (lcatch->env[0].__jmpbuf, errcode ?: -1);
    }
  else
    {
      /* Lossage while resolving the program's own symbols is always fatal.  */
      char buffer[1024];
      _dl_fatal_printf ("%s: %s: %s%s%s%s%s\n",
			rtld_progname ?: "<program name unknown>",
			occation ?: N_("error while loading shared libraries"),
			objname, *objname ? ": " : "",
			errstring, errcode ? ": " : "",
			(errcode
			 ? __strerror_r (errcode, buffer, sizeof buffer)
			 : ""));
    }
}


void
internal_function
_dl_signal_cerror (int errcode, const char *objname, const char *occation,
		   const char *errstring)
{
  if (__builtin_expect (GLRO(dl_debug_mask)
			& ~(DL_DEBUG_STATISTICS|DL_DEBUG_PRELINK), 0))
    _dl_debug_printf ("%s: error: %s: %s (%s)\n", objname, occation,
		      errstring, receiver ? "continued" : "fatal");

  if (receiver)
    {
      /* We are inside _dl_receive_error.  Call the user supplied
	 handler and resume the work.  The receiver will still be
	 installed.  */
      (*receiver) (errcode, objname, errstring);
    }
  else
    _dl_signal_error (errcode, objname, occation, errstring);
}


int
internal_function
_dl_catch_error (const char **objname, const char **errstring,
		 bool *mallocedp, void (*operate) (void *), void *args)
{
  int errcode;
  struct catch *volatile old;
  struct catch c;
  /* We need not handle `receiver' since setting a `catch' is handled
     before it.  */

  /* Some systems (e.g., SPARC) handle constructors to local variables
     inefficient.  So we initialize `c' by hand.  */
  c.errstring = NULL;

  struct catch **const catchp = &CATCH_HOOK;
  old = *catchp;
  /* Do not save the signal mask.  */
  errcode = __sigsetjmp (c.env, 0);
  if (__builtin_expect (errcode, 0) == 0)
    {
      *catchp = &c;
      (*operate) (args);
      *catchp = old;
      *objname = NULL;
      *errstring = NULL;
      *mallocedp = false;
      return 0;
    }

  /* We get here only if we longjmp'd out of OPERATE.  */
  *catchp = old;
  *objname = c.objname;
  *errstring = c.errstring;
  *mallocedp = c.malloced;
  return errcode == -1 ? 0 : errcode;
}


void
internal_function
_dl_receive_error (receiver_fct fct, void (*operate) (void *), void *args)
{
  struct catch **const catchp = &CATCH_HOOK;
  struct catch *old_catch;
  receiver_fct old_receiver;

  old_catch = *catchp;
  old_receiver = receiver;

  /* Set the new values.  */
  *catchp = NULL;
  receiver = fct;

  (*operate) (args);

  *catchp = old_catch;
  receiver = old_receiver;
}
