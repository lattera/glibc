/* Error handling for runtime dynamic linker.
   Copyright (C) 1995, 1996, 1997 Free Software Foundation, Inc.
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

#include <stddef.h>
#include <link.h>
#include <setjmp.h>
#include <stdlib.h>
#include <string.h>

/* This structure communicates state between _dl_catch_error and
   _dl_signal_error.  */
struct catch
  {
    char *errstring;		/* Error detail filled in here.  */
    const char *objname;
    jmp_buf env;		/* longjmp here on error.  */
  };

/* This points to such a structure during a call to _dl_catch_error.
   During implicit startup and run-time work for needed shared libraries,
   this is null.  */
static struct catch *catch;

/* This points to a function which is called when an error is
   received.  Unlike the handling of `catch' this function may return.
   The arguments will be the `errstring' and `objname'.  */
static receiver_fct receiver;


void
_dl_signal_error (int errcode,
		  const char *objname,
		  const char *errstring)
{
  if (! errstring)
    errstring = "DYNAMIC LINKER BUG!!!";

  if (catch)
    {
      /* We are inside _dl_catch_error.  Return to it.  We have to
	 duplicate the error string since it might be allocated on the
	 stack.  */
      size_t len = strlen (errstring) + 1;
      catch->errstring = malloc (len);
      if (catch->errstring != NULL)
	memcpy (catch->errstring, errstring, len);
      catch->objname = objname;
      longjmp (catch->env, errcode ?: -1);
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
      extern char **_dl_argv;	/* Set in rtld.c at startup.  */
      _dl_sysdep_fatal (_dl_argv[0] ?: "<program name unknown>",
			": error in loading shared libraries\n",
			objname ?: "", objname ? ": " : "",
			errstring, errcode ? ": " : "",
			errcode ? strerror (errcode) : "", "\n", NULL);
    }
}

int
_dl_catch_error (char **errstring,
		 const char **objname,
		 void (*operate) (void *),
		 void *args)
{
  int errcode;
  struct catch *old, c;
  /* We need not handle `receiver' since setting a `catch' is handled
     before it.  */

  /* Some systems (.e.g, SPARC) handle constructors to local variables
     inefficient.  So we initialize `c' by hand.  */
  c.errstring = NULL;
  c.objname   = NULL;

  old = catch;
  errcode = setjmp (c.env);
  if (errcode == 0)
    {
      catch = &c;
      (*operate) (args);
      catch = old;
      *errstring = NULL;
      *objname = NULL;
      return 0;
    }

  /* We get here only if we longjmp'd out of OPERATE.  */
  catch = old;
  *errstring = c.errstring;
  *objname = c.objname;
  return errcode == -1 ? 0 : errcode;
}

void
_dl_receive_error (receiver_fct fct, void (*operate) (void *), void *args)
{
  struct catch *old_catch;
  receiver_fct old_receiver;

  old_catch = catch;
  old_receiver = receiver;

  /* Set the new values.  */
  catch = NULL;
  receiver = fct;

  (*operate) (args);

  catch = old_catch;
  receiver = old_receiver;
}
