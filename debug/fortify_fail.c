/* Copyright (C) 2007-2018 Free Software Foundation, Inc.
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

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


extern char **__libc_argv attribute_hidden;

void
__attribute__ ((noreturn))
__fortify_fail_abort (_Bool need_backtrace, const char *msg)
{
  /* The loop is added only to keep gcc happy.  Don't pass down
     __libc_argv[0] if we aren't doing backtrace since __libc_argv[0]
     may point to the corrupted stack.  */
  while (1)
    __libc_message (need_backtrace ? (do_abort | do_backtrace) : do_abort,
		    "*** %s ***: %s terminated\n",
		    msg,
		    (need_backtrace && __libc_argv[0] != NULL
		     ? __libc_argv[0] : "<unknown>"));
}

void
__attribute__ ((noreturn))
__fortify_fail (const char *msg)
{
  __fortify_fail_abort (true, msg);
}

libc_hidden_def (__fortify_fail)
libc_hidden_def (__fortify_fail_abort)
