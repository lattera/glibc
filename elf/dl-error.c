/* Error handling for runtime dynamic linker.
   Copyright (C) 1995, 1996, 1997, 1998 Free Software Foundation, Inc.
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

#include <setjmp.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <elf/ldsodefs.h>
#include <bits/libc-lock.h>

/* This structure communicates state between _dl_catch_error and
   _dl_signal_error.  */
struct catch
  {
    char *errstring;		/* Error detail filled in here.  */
    jmp_buf env;		/* longjmp here on error.  */
  };

/* Multiple threads at once can use the `_dl_catch_error' function.  The
   calls can come from `_dl_map_object_deps', `_dlerror_run', or from
   any of the libc functionality which loads dynamic objects (NSS, iconv).
   Therefore we have to be prepared to save the state in thread-local
   memory.  `catch' will only be used for the non-threaded case.

   Please note the horrible kludge we have to use to check for the
   thread functions to be defined.  The problem is that while running
   ld.so standalone (i.e., before the relocation with the available
   libc symbols) we do not have a real handling of undefined weak symbols.
   All symbols are relocated, regardless of the availability.  They are
   relocated relative to the load address of the dynamic linker.  Adding
   this start address to zero (the value in the GOT for undefined symbols)
   leads to an address which is the load address of ld.so.  Once we have
   relocated with the libc values the value is NULL if the function is
   not available.  Our "solution" is to regard NULL and the ld.so load
   address as indicators for unavailable weak symbols.   */
static struct catch *catch;

#ifdef PIC
# define tsd_setspecific(data) \
  if (__libc_internal_tsd_set != (void *) _dl_rtld_map.l_addr		      \
      && __libc_internal_tsd_set != NULL)				      \
    __libc_internal_tsd_set (_LIBC_TSD_KEY_DL_ERROR, data);		      \
  else									      \
    catch = (data)
# define tsd_getspecific() \
  (__libc_internal_tsd_set != (void *) _dl_rtld_map.l_addr		      \
   && __libc_internal_tsd_set != NULL					      \
   ? (struct catch *) __libc_internal_tsd_get (_LIBC_TSD_KEY_DL_ERROR)	      \
   : catch)
#else
# define tsd_setspecific(data) \
  if (__libc_internal_tsd_set != NULL)					      \
    __libc_internal_tsd_set (_LIBC_TSD_KEY_DL_ERROR, data);		      \
  else									      \
    catch = (data)
# define tsd_getspecific() \
  (__libc_internal_tsd_set != NULL					      \
   ? (struct catch *) __libc_internal_tsd_get (_LIBC_TSD_KEY_DL_ERROR)	      \
   : catch)
#endif


/* This points to a function which is called when an error is
   received.  Unlike the handling of `catch' this function may return.
   The arguments will be the `errstring' and `objname'.

   Since this functionality is not used in normal programs (only in ld.so)
   we do not care about multi-threaded programs here.  We keep this as a
   global variable.  */
static receiver_fct receiver;


void
internal_function
_dl_signal_error (int errcode,
		  const char *objname,
		  const char *errstring)
{
  struct catch *lcatch;

  if (! errstring)
    errstring = "DYNAMIC LINKER BUG!!!";

  lcatch = tsd_getspecific ();
  if (lcatch != NULL)
    {
      /* We are inside _dl_catch_error.  Return to it.  We have to
	 duplicate the error string since it might be allocated on the
	 stack.  */
      size_t objname_len = objname ? strlen (objname) + 2 : 0;
      size_t errstring_len = strlen (errstring) + 1;
      lcatch->errstring = malloc (objname_len + errstring_len);
      if (lcatch->errstring != NULL)
	{
	  if (objname_len > 0)
	    {
	      memcpy (lcatch->errstring, objname, objname_len - 2);
	      memcpy (lcatch->errstring + objname_len - 2, ": ", 2);
	    }
	  memcpy (lcatch->errstring + objname_len, errstring, errstring_len);
	}
      longjmp (lcatch->env, errcode ?: -1);
    }
  else if (receiver)
    {
      /* We are inside _dl_receive_error.  Call the user supplied
	 handler and resume the work.  The receiver will still be
	 installed.  */
      (*receiver) (errcode, objname, errstring);
    }
  else
    {
      /* Lossage while resolving the program's own symbols is always fatal.  */
      char buffer[1024];
      _dl_sysdep_fatal (_dl_argv[0] ?: "<program name unknown>",
			": error in loading shared libraries: ",
			objname ?: "", objname && *objname ? ": " : "",
			errstring, errcode ? ": " : "",
			(errcode
			 ? __strerror_r (errcode, buffer, sizeof buffer)
			 : ""), "\n", NULL);
    }
}

int
internal_function
_dl_catch_error (char **errstring,
		 void (*operate) (void *),
		 void *args)
{
  int errcode;
  struct catch *old, c;
  /* We need not handle `receiver' since setting a `catch' is handled
     before it.  */

  /* Some systems (e.g., SPARC) handle constructors to local variables
     inefficient.  So we initialize `c' by hand.  */
  c.errstring = NULL;

  old = tsd_getspecific ();
  errcode = setjmp (c.env);
  if (errcode == 0)
    {
      tsd_setspecific (&c);
      (*operate) (args);
      tsd_setspecific (old);
      *errstring = NULL;
      return 0;
    }

  /* We get here only if we longjmp'd out of OPERATE.  */
  tsd_setspecific (old);
  *errstring = c.errstring;
  return errcode == -1 ? 0 : errcode;
}

void
internal_function
_dl_receive_error (receiver_fct fct, void (*operate) (void *), void *args)
{
  struct catch *old_catch;
  receiver_fct old_receiver;

  old_catch = tsd_getspecific ();
  old_receiver = receiver;

  /* Set the new values.  */
  tsd_setspecific (NULL);
  receiver = fct;

  (*operate) (args);

  tsd_setspecific (old_catch);
  receiver = old_receiver;
}
