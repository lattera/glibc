/* Copyright (C) 2003-2015 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jakub Jelinek <jakub@redhat.com>.

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
#include <stdio.h>
#include <unwind.h>
#include <pthreadP.h>

static _Unwind_Word (*libgcc_s_getbsp) (struct _Unwind_Context *);

#define ARCH_CANCEL_INIT(handle) \
  ((libgcc_s_getbsp = __libc_dlsym (handle, "_Unwind_GetBSP")) == NULL)

#include <sysdeps/nptl/unwind-forcedunwind.c>

_Unwind_Word
_Unwind_GetBSP (struct _Unwind_Context *context)
{
  if (__builtin_expect (libgcc_s_getbsp == NULL, 0))
    pthread_cancel_init ();

  return libgcc_s_getbsp (context);
}
