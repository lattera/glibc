/* More debugging hooks for `malloc'.
   Copyright (C) 1991, 1992, 1993, 1994, 1996 Free Software Foundation, Inc.
		 Written April 2, 1991 by John Gilmore of Cygnus Support.
		 Based on mcheck.c by Mike Haertel.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   The author may be reached (Email) at the address mike@ai.mit.edu,
   or (US mail) as Mike Haertel c/o Free Software Foundation.  */

#ifndef	_MALLOC_INTERNAL
#define	_MALLOC_INTERNAL
#include <malloc.h>
#include <mcheck.h>
#include <bits/libc-lock.h>
#endif

#include <stdio.h>

#ifndef	__GNU_LIBRARY__
extern char *getenv ();
#else
#include <stdlib.h>
#endif

static FILE *mallstream;
static char mallenv[]= "MALLOC_TRACE";
static char mallbuf[BUFSIZ];	/* Buffer for the output.  */

__libc_lock_define_initialized (static, lock);

/* Address to breakpoint on accesses to... */
__ptr_t mallwatch;

/* File name and line number information, for callers that had
   the foresight to call through a macro.  */
char *_mtrace_file;
int _mtrace_line;

/* Old hook values.  */
static void (*tr_old_free_hook) __P ((__ptr_t ptr));
static __ptr_t (*tr_old_malloc_hook) __P ((__malloc_size_t size));
static __ptr_t (*tr_old_realloc_hook) __P ((__ptr_t ptr, __malloc_size_t size));

/* This function is called when the block being alloc'd, realloc'd, or
   freed has an address matching the variable "mallwatch".  In a debugger,
   set "mallwatch" to the address of interest, then put a breakpoint on
   tr_break.  */

void tr_break __P ((void));
void
tr_break ()
{
}

static void tr_where __P ((void));
static void
tr_where ()
{
  if (_mtrace_file)
    {
      fprintf (mallstream, "@ %s:%d ", _mtrace_file, _mtrace_line);
      _mtrace_file = NULL;
    }
}

static void tr_freehook __P ((__ptr_t));
static void
tr_freehook (ptr)
     __ptr_t ptr;
{
  tr_where ();
  fprintf (mallstream, "- %p\n", ptr);	/* Be sure to print it first.  */
  if (ptr == mallwatch)
    tr_break ();
  __libc_lock_lock (lock);
  __free_hook = tr_old_free_hook;
  free (ptr);
  __free_hook = tr_freehook;
  __libc_lock_unlock (lock);
}

static __ptr_t tr_mallochook __P ((__malloc_size_t));
static __ptr_t
tr_mallochook (size)
     __malloc_size_t size;
{
  __ptr_t hdr;

  __libc_lock_lock (lock);

  __malloc_hook = tr_old_malloc_hook;
  hdr = (__ptr_t) malloc (size);
  __malloc_hook = tr_mallochook;

  __libc_lock_unlock (lock);

  tr_where ();
  /* We could be printing a NULL here; that's OK.  */
  fprintf (mallstream, "+ %p %lx\n", hdr, (unsigned long)size);

  if (hdr == mallwatch)
    tr_break ();

  return hdr;
}

static __ptr_t tr_reallochook __P ((__ptr_t, __malloc_size_t));
static __ptr_t
tr_reallochook (ptr, size)
     __ptr_t ptr;
     __malloc_size_t size;
{
  __ptr_t hdr;

  if (ptr == mallwatch)
    tr_break ();

  __libc_lock_lock (lock);

  __free_hook = tr_old_free_hook;
  __malloc_hook = tr_old_malloc_hook;
  __realloc_hook = tr_old_realloc_hook;
  hdr = (__ptr_t) realloc (ptr, size);
  __free_hook = tr_freehook;
  __malloc_hook = tr_mallochook;
  __realloc_hook = tr_reallochook;

  __libc_lock_unlock (lock);

  tr_where ();
  if (hdr == NULL)
    /* Failed realloc.  */
    fprintf (mallstream, "! %p %lx\n", ptr, (unsigned long)size);
  else if (ptr == NULL)
    fprintf (mallstream, "+ %p %lx\n", hdr, (unsigned long)size);
  else
    fprintf (mallstream, "< %p\n> %p %lx\n", ptr, hdr, (unsigned long)size);

  if (hdr == mallwatch)
    tr_break ();

  return hdr;
}

/* We enable tracing if either the environment variable MALLOC_TRACE
   is set, or if the variable mallwatch has been patched to an address
   that the debugging user wants us to stop on.  When patching mallwatch,
   don't forget to set a breakpoint on tr_break!  */

void
mtrace ()
{
  char *mallfile;

  /* Don't panic if we're called more than once.  */
  if (mallstream != NULL)
    return;

#ifdef _LIBC
  /* When compiling the GNU libc we use the secure getenv function
     which prevents the misuse in case of SUID or SGID enabled
     programs.  */
  mallfile = __secure_getenv (mallenv);
#else
  mallfile = getenv (mallenv);
#endif
  if (mallfile != NULL || mallwatch != NULL)
    {
      mallstream = fopen (mallfile != NULL ? mallfile : "/dev/null", "w");
      if (mallstream != NULL)
	{
	  /* Be sure it doesn't malloc its buffer!  */
	  setbuf (mallstream, mallbuf);
	  fprintf (mallstream, "= Start\n");
	  tr_old_free_hook = __free_hook;
	  __free_hook = tr_freehook;
	  tr_old_malloc_hook = __malloc_hook;
	  __malloc_hook = tr_mallochook;
	  tr_old_realloc_hook = __realloc_hook;
	  __realloc_hook = tr_reallochook;
	}
    }
}

void
muntrace ()
{
  if (mallstream == NULL)
    return;

  fprintf (mallstream, "= End\n");
  fclose (mallstream);
  mallstream = NULL;
  __free_hook = tr_old_free_hook;
  __malloc_hook = tr_old_malloc_hook;
  __realloc_hook = tr_old_realloc_hook;
}
