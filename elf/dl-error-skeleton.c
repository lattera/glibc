/* Template for error handling for runtime dynamic linker.
   Copyright (C) 1995-2016 Free Software Foundation, Inc.
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

/* The following macro needs to be defined before including this
   skeleton file:

   DL_ERROR_BOOTSTRAP

     If 1, do not use TLS and implement _dl_signal_cerror and
     _dl_receive_error.  If 0, TLS is used, and the variants with
     error callbacks are not provided.  */


#include <libintl.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ldsodefs.h>
#include <stdio.h>

/* This structure communicates state between _dl_catch_error and
   _dl_signal_error.  */
struct catch
  {
    const char **objname;	/* Object/File name.  */
    const char **errstring;	/* Error detail filled in here.  */
    bool *malloced;		/* Nonzero if the string is malloced
				   by the libc malloc.  */
    volatile int *errcode;	/* Return value of _dl_signal_error.  */
    jmp_buf env;		/* longjmp here on error.  */
  };

/* Multiple threads at once can use the `_dl_catch_error' function.  The
   calls can come from `_dl_map_object_deps', `_dlerror_run', or from
   any of the libc functionality which loads dynamic objects (NSS, iconv).
   Therefore we have to be prepared to save the state in thread-local
   memory.  */
#if !DL_ERROR_BOOTSTRAP
static __thread struct catch *catch_hook attribute_tls_model_ie;
#else
/* The version of this code in ld.so cannot use thread-local variables
   and is used during bootstrap only.  */
static struct catch *catch_hook;
#endif

/* This message we return as a last resort.  We define the string in a
   variable since we have to avoid freeing it and so have to enable
   a pointer comparison.  See below and in dlfcn/dlerror.c.  */
static const char _dl_out_of_memory[] = "out of memory";

#if DL_ERROR_BOOTSTRAP
/* This points to a function which is called when an continuable error is
   received.  Unlike the handling of `catch' this function may return.
   The arguments will be the `errstring' and `objname'.

   Since this functionality is not used in normal programs (only in ld.so)
   we do not care about multi-threaded programs here.  We keep this as a
   global variable.  */
static receiver_fct receiver;
#endif /* DL_ERROR_BOOTSTRAP */

void
internal_function
_dl_signal_error (int errcode, const char *objname, const char *occation,
		  const char *errstring)
{
  struct catch *lcatch = catch_hook;

  if (! errstring)
    errstring = N_("DYNAMIC LINKER BUG!!!");

  if (objname == NULL)
    objname = "";
  if (lcatch != NULL)
    {
      /* We are inside _dl_catch_error.  Return to it.  We have to
	 duplicate the error string since it might be allocated on the
	 stack.  The object name is always a string constant.  */
      size_t len_objname = strlen (objname) + 1;
      size_t len_errstring = strlen (errstring) + 1;

      char *errstring_copy = malloc (len_objname + len_errstring);
      if (errstring_copy != NULL)
	{
	  /* Make a copy of the object file name and the error string.  */
	  *lcatch->objname = memcpy (__mempcpy (errstring_copy,
						errstring, len_errstring),
				     objname, len_objname);
	  *lcatch->errstring = errstring_copy;

	  /* If the main executable is relocated it means the libc's malloc
	     is used.  */
          bool malloced = true;
#ifdef SHARED
	  malloced = (GL(dl_ns)[LM_ID_BASE]._ns_loaded != NULL
                      && (GL(dl_ns)[LM_ID_BASE]._ns_loaded->l_relocated != 0));
#endif
	  *lcatch->malloced = malloced;
	}
      else
	{
	  /* This is better than nothing.  */
	  *lcatch->objname = "";
	  *lcatch->errstring = _dl_out_of_memory;
	  *lcatch->malloced = false;
	}

      *lcatch->errcode = errcode;

      /* We do not restore the signal mask because none was saved.  */
      __longjmp (lcatch->env[0].__jmpbuf, 1);
    }
  else
    {
      /* Lossage while resolving the program's own symbols is always fatal.  */
      char buffer[1024];
      _dl_fatal_printf ("%s: %s: %s%s%s%s%s\n",
			RTLD_PROGNAME,
			occation ?: N_("error while loading shared libraries"),
			objname, *objname ? ": " : "",
			errstring, errcode ? ": " : "",
			(errcode
			 ? __strerror_r (errcode, buffer, sizeof buffer)
			 : ""));
    }
}
libc_hidden_def (_dl_signal_error)


#if DL_ERROR_BOOTSTRAP
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
#endif /* DL_ERROR_BOOTSTRAP */


int
internal_function
_dl_catch_error (const char **objname, const char **errstring,
		 bool *mallocedp, void (*operate) (void *), void *args)
{
  /* We need not handle `receiver' since setting a `catch' is handled
     before it.  */

  /* Only this needs to be marked volatile, because it is the only local
     variable that gets changed between the setjmp invocation and the
     longjmp call.  All others are just set here (before setjmp) and read
     in _dl_signal_error (before longjmp).  */
  volatile int errcode;

  struct catch c;
  /* Don't use an initializer since we don't need to clear C.env.  */
  c.objname = objname;
  c.errstring = errstring;
  c.malloced = mallocedp;
  c.errcode = &errcode;

  struct catch *const old = catch_hook;
  catch_hook = &c;

  /* Do not save the signal mask.  */
  if (__builtin_expect (__sigsetjmp (c.env, 0), 0) == 0)
    {
      (*operate) (args);
      catch_hook = old;
      *objname = NULL;
      *errstring = NULL;
      *mallocedp = false;
      return 0;
    }

  /* We get here only if we longjmp'd out of OPERATE.  _dl_signal_error has
     already stored values into *OBJNAME, *ERRSTRING, and *MALLOCEDP.  */
  catch_hook = old;
  return errcode;
}
libc_hidden_def (_dl_catch_error)

#if DL_ERROR_BOOTSTRAP
void
internal_function
_dl_receive_error (receiver_fct fct, void (*operate) (void *), void *args)
{
  struct catch *old_catch = catch_hook;
  receiver_fct old_receiver = receiver;

  /* Set the new values.  */
  catch_hook = NULL;
  receiver = fct;

  (*operate) (args);

  catch_hook = old_catch;
  receiver = old_receiver;
}
#endif /* DL_ERROR_BOOTSTRAP */
