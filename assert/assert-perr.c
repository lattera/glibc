/* Copyright (C) 1994, 1995, 1996, 1997, 1998 Free Software Foundation, Inc.
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

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sysdep.h>


extern const char *__assert_program_name; /* In assert.c.  */

#ifdef USE_IN_LIBIO
# include <libo/iolibio.h>
# define fflush(s) _IO_fflush (s)
#endif

/* This function, when passed an error number, a filename, and a line
   number, prints a message on the standard error stream of the form:
   	a.c:10: foobar: Unexpected error: Computer bought the farm
   It then aborts program execution via a call to `abort'.  */

#ifdef FATAL_PREPARE_INCLUDE
# include FATAL_PREPARE_INCLUDE
#endif

void
__assert_perror_fail (int errnum,
		      const char *file, unsigned int line,
		      const char *function)
{
  char errbuf[1024];
#ifdef FATAL_PREPARE
  FATAL_PREPARE;
#endif

  /* Print the message.  */
  (void) fprintf (stderr, _("%s%s%s:%u: %s%sUnexpected error: %s.\n"),
		  __assert_program_name ? __assert_program_name : "",
		  __assert_program_name ? ": " : "",
		  file, line,
		  function ? function : "", function ? ": " : "",
		  __strerror_r (errnum, errbuf, sizeof errbuf));
  (void) fflush (stderr);

  abort ();
}
