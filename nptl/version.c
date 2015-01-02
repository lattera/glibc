/* Entry point for libpthread DSO.
   Copyright (C) 2002-2015 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2002.

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

#include <unistd.h>
#include <sysdep.h>


static const char banner[] =
#include "banner.h"
"Copyright (C) 2015 Free Software Foundation, Inc.\n\
This is free software; see the source for copying conditions.\n\
There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A\n\
PARTICULAR PURPOSE.\n"
#ifdef HAVE_FORCED_UNWIND
"Forced unwind support included.\n"
#endif
;


/* This is made the e_entry of libpthread.so by LDFLAGS-pthread.so.  */
__attribute__ ((noreturn))
void
__nptl_main (void)
{
  __libc_write (STDOUT_FILENO, banner, sizeof banner - 1);
  _exit (0);
}
