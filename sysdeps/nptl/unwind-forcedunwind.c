/* Copyright (C) 2003-2018 Free Software Foundation, Inc.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, see <http://www.gnu.org/licenses/>.  */

#include <dlfcn.h>
#include <stdio.h>
#include <unwind.h>
#include <pthreadP.h>
#include <sysdep.h>
#include <gnu/lib-names.h>
#include <unwind-resume.h>

static void *libgcc_s_handle;
void (*__libgcc_s_resume) (struct _Unwind_Exception *exc)
  attribute_hidden __attribute__ ((noreturn));
static _Unwind_Reason_Code (*libgcc_s_personality) PERSONALITY_PROTO;
static _Unwind_Reason_Code (*libgcc_s_forcedunwind)
  (struct _Unwind_Exception *, _Unwind_Stop_Fn, void *);
static _Unwind_Word (*libgcc_s_getcfa) (struct _Unwind_Context *);

void
__attribute_noinline__
pthread_cancel_init (void)
{
  void *resume;
  void *personality;
  void *forcedunwind;
  void *getcfa;
  void *handle;

  if (__glibc_likely (libgcc_s_handle != NULL))
    {
      /* Force gcc to reload all values.  */
      asm volatile ("" ::: "memory");
      return;
    }

  /* See include/dlfcn.h. Use of __libc_dlopen requires RTLD_NOW.  */
  handle = __libc_dlopen (LIBGCC_S_SO);

  if (handle == NULL
      || (resume = __libc_dlsym (handle, "_Unwind_Resume")) == NULL
      || (personality = __libc_dlsym (handle, "__gcc_personality_v0")) == NULL
      || (forcedunwind = __libc_dlsym (handle, "_Unwind_ForcedUnwind"))
	 == NULL
      || (getcfa = __libc_dlsym (handle, "_Unwind_GetCFA")) == NULL
#ifdef ARCH_CANCEL_INIT
      || ARCH_CANCEL_INIT (handle)
#endif
      )
    __libc_fatal (LIBGCC_S_SO " must be installed for pthread_cancel to work\n");

  PTR_MANGLE (resume);
  __libgcc_s_resume = resume;
  PTR_MANGLE (personality);
  libgcc_s_personality = personality;
  PTR_MANGLE (forcedunwind);
  libgcc_s_forcedunwind = forcedunwind;
  PTR_MANGLE (getcfa);
  libgcc_s_getcfa = getcfa;
  /* Make sure libgcc_s_handle is written last.  Otherwise,
     pthread_cancel_init might return early even when the pointer the
     caller is interested in is not initialized yet.  */
  atomic_write_barrier ();
  libgcc_s_handle = handle;
}

/* Register for cleanup in libpthread.so.  */
void
__nptl_unwind_freeres (void)
{
  void *handle = libgcc_s_handle;
  if (handle != NULL)
    {
      libgcc_s_handle = NULL;
      __libc_dlclose (handle);
    }
}

#if !HAVE_ARCH_UNWIND_RESUME
void
_Unwind_Resume (struct _Unwind_Exception *exc)
{
  if (__glibc_unlikely (libgcc_s_handle == NULL))
    pthread_cancel_init ();
  else
    atomic_read_barrier ();

  void (*resume) (struct _Unwind_Exception *exc) = __libgcc_s_resume;
  PTR_DEMANGLE (resume);
  resume (exc);
}
#endif

_Unwind_Reason_Code
__gcc_personality_v0 PERSONALITY_PROTO
{
  if (__glibc_unlikely (libgcc_s_handle == NULL))
    pthread_cancel_init ();
  else
    atomic_read_barrier ();

  __typeof (libgcc_s_personality) personality = libgcc_s_personality;
  PTR_DEMANGLE (personality);
  return (*personality) PERSONALITY_ARGS;
}

_Unwind_Reason_Code
_Unwind_ForcedUnwind (struct _Unwind_Exception *exc, _Unwind_Stop_Fn stop,
		      void *stop_argument)
{
  if (__glibc_unlikely (libgcc_s_handle == NULL))
    pthread_cancel_init ();
  else
    atomic_read_barrier ();

  _Unwind_Reason_Code (*forcedunwind)
    (struct _Unwind_Exception *, _Unwind_Stop_Fn, void *)
    = libgcc_s_forcedunwind;
  PTR_DEMANGLE (forcedunwind);
  return forcedunwind (exc, stop, stop_argument);
}

_Unwind_Word
_Unwind_GetCFA (struct _Unwind_Context *context)
{
  if (__glibc_unlikely (libgcc_s_handle == NULL))
    pthread_cancel_init ();
  else
    atomic_read_barrier ();

  _Unwind_Word (*getcfa) (struct _Unwind_Context *) = libgcc_s_getcfa;
  PTR_DEMANGLE (getcfa);
  return getcfa (context);
}
