/* Copyright (C) 1991,1994-1996,1998,2001,2002 Free Software Foundation, Inc.
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

#include <assert.h>
#include <libintl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysdep.h>
#include <unistd.h>


extern const char *__progname;

#ifdef USE_IN_LIBIO
# include <wchar.h>
# include <libio/iolibio.h>
# define fflush(s) INTUSE(_IO_fflush) (s)
#endif

/* This function, when passed a string containing an asserted
   expression, a filename, and a line number, prints a message
   on the standard error stream of the form:
   	a.c:10: foobar: Assertion `a == b' failed.
   It then aborts program execution via a call to `abort'.  */

#ifdef FATAL_PREPARE_INCLUDE
# include FATAL_PREPARE_INCLUDE
#endif

#undef __assert_fail
void
__assert_fail (const char *assertion, const char *file, unsigned int line,
	       const char *function)
{
  char *buf;

#ifdef FATAL_PREPARE
  FATAL_PREPARE;
#endif

  if (__asprintf (&buf, _("%s%s%s:%u: %s%sAssertion `%s' failed.\n"),
		  __progname, __progname[0] ? ": " : "",
		  file, line,
		  function ? function : "", function ? ": " : "",
		  assertion) >= 0)
    {
      /* Print the message.  */
#ifdef USE_IN_LIBIO
      if (_IO_fwide (stderr, 0) > 0)
	(void) __fwprintf (stderr, L"%s", buf);
      else
#endif
	(void) fputs (buf, stderr);

      (void) fflush (stderr);

      /* We have to free the buffer since the application might catch the
	 SIGABRT.  */
      free (buf);
    }
  else
    {
      /* At least print a minimal message.  */
      static const char errstr[] = "Unexpected error.\n";
      __libc_write (STDERR_FILENO, errstr, sizeof (errstr) - 1);
    }

  abort ();
}
hidden_def(__assert_fail)
