/* Copyright (C) 2003, 2006 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jakub Jelinek <jakub@redhat.com>.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <dlfcn.h>
#include <stdio.h>
#include <unwind.h>
#include <pthreadP.h>

static _Unwind_Word (*libgcc_s_getbsp) (struct _Unwind_Context *);

#define ARCH_CANCEL_INIT(handle) \
  ((libgcc_s_getbsp = __libc_dlsym (handle, "_Unwind_GetBSP")) == NULL)

#include <sysdeps/pthread/unwind-forcedunwind.c>

_Unwind_Word
_Unwind_GetBSP (struct _Unwind_Context *context)
{
  if (__builtin_expect (libgcc_s_getbsp == NULL, 0))
    pthread_cancel_init ();

  return libgcc_s_getbsp (context);
}
