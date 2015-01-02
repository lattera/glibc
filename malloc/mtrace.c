/* More debugging hooks for `malloc'.
   Copyright (C) 1991-2015 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
                 Written April 2, 1991 by John Gilmore of Cygnus Support.
                 Based on mcheck.c by Mike Haertel.

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

#ifndef _MALLOC_INTERNAL
# define _MALLOC_INTERNAL
# include <malloc.h>
# include <mcheck.h>
# include <bits/libc-lock.h>
#endif

#include <dlfcn.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <_itoa.h>

#include <libc-internal.h>

#include <libio/iolibio.h>
#define setvbuf(s, b, f, l) _IO_setvbuf (s, b, f, l)
#define fwrite(buf, size, count, fp) _IO_fwrite (buf, size, count, fp)

#include <kernel-features.h>

#define TRACE_BUFFER_SIZE 512

static FILE *mallstream;
static const char mallenv[] = "MALLOC_TRACE";
static char *malloc_trace_buffer;

__libc_lock_define_initialized (static, lock);

/* Address to breakpoint on accesses to... */
__ptr_t mallwatch;

/* Old hook values.  */
static void (*tr_old_free_hook) (__ptr_t ptr, const __ptr_t);
static __ptr_t (*tr_old_malloc_hook) (size_t size, const __ptr_t);
static __ptr_t (*tr_old_realloc_hook) (__ptr_t ptr, size_t size,
                                       const __ptr_t);
static __ptr_t (*tr_old_memalign_hook) (size_t __alignment, size_t __size,
                                        const __ptr_t);

/* This function is called when the block being alloc'd, realloc'd, or
   freed has an address matching the variable "mallwatch".  In a debugger,
   set "mallwatch" to the address of interest, then put a breakpoint on
   tr_break.  */

extern void tr_break (void) __THROW;
libc_hidden_proto (tr_break)
void
tr_break (void)
{
}
libc_hidden_def (tr_break)

static void internal_function
tr_where (const __ptr_t caller, Dl_info *info)
{
  if (caller != NULL)
    {
      if (info != NULL)
        {
          char *buf = (char *) "";
          if (info->dli_sname != NULL)
            {
              size_t len = strlen (info->dli_sname);
              buf = alloca (len + 6 + 2 * sizeof (void *));

              buf[0] = '(';
              __stpcpy (_fitoa (caller >= (const __ptr_t) info->dli_saddr
                                ? caller - (const __ptr_t) info->dli_saddr
                                : (const __ptr_t) info->dli_saddr - caller,
                                __stpcpy (__mempcpy (buf + 1, info->dli_sname,
                                                     len),
                                          caller >= (__ptr_t) info->dli_saddr
                                          ? "+0x" : "-0x"),
                                16, 0),
                        ")");
            }

          fprintf (mallstream, "@ %s%s%s[%p] ",
                   info->dli_fname ? : "", info->dli_fname ? ":" : "",
                   buf, caller);
        }
      else
        fprintf (mallstream, "@ [%p] ", caller);
    }
}

static Dl_info *
lock_and_info (const __ptr_t caller, Dl_info *mem)
{
  if (caller == NULL)
    return NULL;

  Dl_info *res = _dl_addr (caller, mem, NULL, NULL) ? mem : NULL;

  __libc_lock_lock (lock);

  return res;
}

static void
tr_freehook (__ptr_t ptr, const __ptr_t caller)
{
  if (ptr == NULL)
    return;

  Dl_info mem;
  Dl_info *info = lock_and_info (caller, &mem);
  tr_where (caller, info);
  /* Be sure to print it first.  */
  fprintf (mallstream, "- %p\n", ptr);
  if (ptr == mallwatch)
    {
      __libc_lock_unlock (lock);
      tr_break ();
      __libc_lock_lock (lock);
    }
  __free_hook = tr_old_free_hook;
  if (tr_old_free_hook != NULL)
    (*tr_old_free_hook)(ptr, caller);
  else
    free (ptr);
  __free_hook = tr_freehook;
  __libc_lock_unlock (lock);
}

static __ptr_t
tr_mallochook (size_t size, const __ptr_t caller)
{
  __ptr_t hdr;

  Dl_info mem;
  Dl_info *info = lock_and_info (caller, &mem);

  __malloc_hook = tr_old_malloc_hook;
  if (tr_old_malloc_hook != NULL)
    hdr = (__ptr_t) (*tr_old_malloc_hook)(size, caller);
  else
    hdr = (__ptr_t) malloc (size);
  __malloc_hook = tr_mallochook;

  tr_where (caller, info);
  /* We could be printing a NULL here; that's OK.  */
  fprintf (mallstream, "+ %p %#lx\n", hdr, (unsigned long int) size);

  __libc_lock_unlock (lock);

  if (hdr == mallwatch)
    tr_break ();

  return hdr;
}

static __ptr_t
tr_reallochook (__ptr_t ptr, size_t size, const __ptr_t caller)
{
  __ptr_t hdr;

  if (ptr == mallwatch)
    tr_break ();

  Dl_info mem;
  Dl_info *info = lock_and_info (caller, &mem);

  __free_hook = tr_old_free_hook;
  __malloc_hook = tr_old_malloc_hook;
  __realloc_hook = tr_old_realloc_hook;
  if (tr_old_realloc_hook != NULL)
    hdr = (__ptr_t) (*tr_old_realloc_hook)(ptr, size, caller);
  else
    hdr = (__ptr_t) realloc (ptr, size);
  __free_hook = tr_freehook;
  __malloc_hook = tr_mallochook;
  __realloc_hook = tr_reallochook;

  tr_where (caller, info);
  if (hdr == NULL)
    {
      if (size != 0)
        /* Failed realloc.  */
        fprintf (mallstream, "! %p %#lx\n", ptr, (unsigned long int) size);
      else
        fprintf (mallstream, "- %p\n", ptr);
    }
  else if (ptr == NULL)
    fprintf (mallstream, "+ %p %#lx\n", hdr, (unsigned long int) size);
  else
    {
      fprintf (mallstream, "< %p\n", ptr);
      tr_where (caller, info);
      fprintf (mallstream, "> %p %#lx\n", hdr, (unsigned long int) size);
    }

  __libc_lock_unlock (lock);

  if (hdr == mallwatch)
    tr_break ();

  return hdr;
}

static __ptr_t
tr_memalignhook (size_t alignment, size_t size, const __ptr_t caller)
{
  __ptr_t hdr;

  Dl_info mem;
  Dl_info *info = lock_and_info (caller, &mem);

  __memalign_hook = tr_old_memalign_hook;
  __malloc_hook = tr_old_malloc_hook;
  if (tr_old_memalign_hook != NULL)
    hdr = (__ptr_t) (*tr_old_memalign_hook)(alignment, size, caller);
  else
    hdr = (__ptr_t) memalign (alignment, size);
  __memalign_hook = tr_memalignhook;
  __malloc_hook = tr_mallochook;

  tr_where (caller, info);
  /* We could be printing a NULL here; that's OK.  */
  fprintf (mallstream, "+ %p %#lx\n", hdr, (unsigned long int) size);

  __libc_lock_unlock (lock);

  if (hdr == mallwatch)
    tr_break ();

  return hdr;
}


#ifdef _LIBC

/* This function gets called to make sure all memory the library
   allocates get freed and so does not irritate the user when studying
   the mtrace output.  */
static void __libc_freeres_fn_section
release_libc_mem (void)
{
  /* Only call the free function if we still are running in mtrace mode.  */
  if (mallstream != NULL)
    __libc_freeres ();
}
#endif


/* We enable tracing if either the environment variable MALLOC_TRACE
   is set, or if the variable mallwatch has been patched to an address
   that the debugging user wants us to stop on.  When patching mallwatch,
   don't forget to set a breakpoint on tr_break!  */

void
mtrace (void)
{
#ifdef _LIBC
  static int added_atexit_handler;
#endif
  char *mallfile;

  /* Don't panic if we're called more than once.  */
  if (mallstream != NULL)
    return;

#ifdef _LIBC
  /* When compiling the GNU libc we use the secure getenv function
     which prevents the misuse in case of SUID or SGID enabled
     programs.  */
  mallfile = __libc_secure_getenv (mallenv);
#else
  mallfile = getenv (mallenv);
#endif
  if (mallfile != NULL || mallwatch != NULL)
    {
      char *mtb = malloc (TRACE_BUFFER_SIZE);
      if (mtb == NULL)
        return;

      mallstream = fopen (mallfile != NULL ? mallfile : "/dev/null", "wce");
      if (mallstream != NULL)
        {
#ifndef __ASSUME_O_CLOEXEC
          /* Make sure we close the file descriptor on exec.  */
          int flags = __fcntl (fileno (mallstream), F_GETFD, 0);
          if (flags >= 0)
            {
              flags |= FD_CLOEXEC;
              __fcntl (fileno (mallstream), F_SETFD, flags);
            }
#endif
          /* Be sure it doesn't malloc its buffer!  */
          malloc_trace_buffer = mtb;
          setvbuf (mallstream, malloc_trace_buffer, _IOFBF, TRACE_BUFFER_SIZE);
          fprintf (mallstream, "= Start\n");
          tr_old_free_hook = __free_hook;
          __free_hook = tr_freehook;
          tr_old_malloc_hook = __malloc_hook;
          __malloc_hook = tr_mallochook;
          tr_old_realloc_hook = __realloc_hook;
          __realloc_hook = tr_reallochook;
          tr_old_memalign_hook = __memalign_hook;
          __memalign_hook = tr_memalignhook;
#ifdef _LIBC
          if (!added_atexit_handler)
            {
              extern void *__dso_handle __attribute__ ((__weak__));
              added_atexit_handler = 1;
              __cxa_atexit ((void (*)(void *))release_libc_mem, NULL,
                            &__dso_handle ? __dso_handle : NULL);
            }
#endif
        }
      else
        free (mtb);
    }
}

void
muntrace (void)
{
  if (mallstream == NULL)
    return;

  /* Do the reverse of what done in mtrace: first reset the hooks and
     MALLSTREAM, and only after that write the trailer and close the
     file.  */
  FILE *f = mallstream;
  mallstream = NULL;
  __free_hook = tr_old_free_hook;
  __malloc_hook = tr_old_malloc_hook;
  __realloc_hook = tr_old_realloc_hook;
  __memalign_hook = tr_old_memalign_hook;

  fprintf (f, "= End\n");
  fclose (f);
}
