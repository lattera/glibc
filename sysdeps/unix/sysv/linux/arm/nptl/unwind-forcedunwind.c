/* Copyright (C) 2003, 2009 Free Software Foundation, Inc.
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

static void *libgcc_s_handle;
static void (*libgcc_s_resume) (struct _Unwind_Exception *exc);
static _Unwind_Reason_Code (*libgcc_s_personality)
  (int, _Unwind_Action, _Unwind_Exception_Class, struct _Unwind_Exception *,
   struct _Unwind_Context *);
static _Unwind_Reason_Code (*libgcc_s_forcedunwind)
  (struct _Unwind_Exception *, _Unwind_Stop_Fn, void *);
static _Unwind_Word (*libgcc_s_getcfa) (struct _Unwind_Context *);
static void (*libgcc_s_sjlj_register) (struct SjLj_Function_Context *);
static void (*libgcc_s_sjlj_unregister) (struct SjLj_Function_Context *);

void
__attribute_noinline__
pthread_cancel_init (void)
{
  void *resume, *personality, *forcedunwind, *getcfa;
  void *handle;
  void *sjlj_register, *sjlj_unregister;

  if (__builtin_expect (libgcc_s_handle != NULL, 1))
    {
      /* Force gcc to reload all values.  */
      asm volatile ("" ::: "memory");
      return;
    }

  handle = __libc_dlopen ("libgcc_s.so.1");

  if (handle == NULL
      || (sjlj_register = __libc_dlsym (handle, "_Unwind_SjLj_Register")) == NULL
      || (sjlj_unregister = __libc_dlsym (handle, "_Unwind_SjLj_Unregister")) == NULL
      || (resume = __libc_dlsym (handle, "_Unwind_SjLj_Resume")) == NULL
      || (personality = __libc_dlsym (handle, "__gcc_personality_sj0")) == NULL
      || (forcedunwind = __libc_dlsym (handle, "_Unwind_SjLj_ForcedUnwind"))
	 == NULL
      || (getcfa = __libc_dlsym (handle, "_Unwind_GetCFA")) == NULL
      )
    __libc_fatal ("libgcc_s.so.1 must be installed for pthread_cancel to work\n");

  libgcc_s_resume = resume;
  libgcc_s_personality = personality;
  libgcc_s_forcedunwind = forcedunwind;
  libgcc_s_sjlj_register = sjlj_register;
  libgcc_s_sjlj_unregister = sjlj_unregister;
  libgcc_s_getcfa = getcfa;
  /* Make sure libgcc_s_getcfa is written last.  Otherwise,
     pthread_cancel_init might return early even when the pointer the
     caller is interested in is not initialized yet.  */
  atomic_write_barrier ();
  libgcc_s_handle = handle;
}

void
__libc_freeres_fn_section
__unwind_freeres (void)
{
  void *handle = libgcc_s_handle;
  if (handle != NULL)
    {
      libgcc_s_handle = NULL;
      __libc_dlclose (handle);
    }
}

void
_Unwind_Resume (struct _Unwind_Exception *exc)
{
  if (__builtin_expect (libgcc_s_resume == NULL, 0))
    pthread_cancel_init ();

  libgcc_s_resume (exc);
}

_Unwind_Reason_Code
__gcc_personality_v0 (int version, _Unwind_Action actions,
		      _Unwind_Exception_Class exception_class,
                      struct _Unwind_Exception *ue_header,
                      struct _Unwind_Context *context)
{
  if (__builtin_expect (libgcc_s_personality == NULL, 0))
    pthread_cancel_init ();

  return libgcc_s_personality (version, actions, exception_class,
			       ue_header, context);
}

_Unwind_Reason_Code
_Unwind_ForcedUnwind (struct _Unwind_Exception *exc, _Unwind_Stop_Fn stop,
		      void *stop_argument)
{
  if (__builtin_expect (libgcc_s_forcedunwind == NULL, 0))
    pthread_cancel_init ();

  return libgcc_s_forcedunwind (exc, stop, stop_argument);
}

_Unwind_Word
_Unwind_GetCFA (struct _Unwind_Context *context)
{
  if (__builtin_expect (libgcc_s_getcfa == NULL, 0))
    pthread_cancel_init ();

  return libgcc_s_getcfa (context);
}

void
_Unwind_SjLj_Register (struct SjLj_Function_Context *fc)
{
  if (__builtin_expect (libgcc_s_sjlj_register == NULL, 0))
    pthread_cancel_init ();

  libgcc_s_sjlj_register (fc);
}

void
_Unwind_SjLj_Unregister (struct SjLj_Function_Context *fc)
{
  if (__builtin_expect (libgcc_s_sjlj_unregister == NULL, 0))
    pthread_cancel_init ();

  libgcc_s_sjlj_unregister (fc);
}
