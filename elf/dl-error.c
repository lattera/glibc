/* Error handling for runtime dynamic linker.
Copyright (C) 1995 Free Software Foundation, Inc.
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
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <stddef.h>
#include <link.h>
#include <setjmp.h>

static jmp_buf catch_env;
static const char *signalled_errstring;

void
_dl_signal_error (int errcode, const char *errstring)
{
  signalled_errstring = errstring ?: "DYNAMIC LINKER BUG!!!";
  longjmp (catch_env, errcode ?: -1);
}

int
_dl_catch_error (const char **errstring, void (*operate) (void))
{
  int errcode;

  signalled_errstring = NULL;
  errcode = setjmp (catch_env);
  (*operate) ();
  *errstring = signalled_errstring;
  return *errstring ? errcode : 0;
}
