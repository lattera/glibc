/* Minimal replacements for basic facilities used in the dynamic linker.
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

#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <link.h>
#include "../stdio-common/_itoa.h"

/* Minimal `malloc' allocator for use while loading shared libraries.
   Only small blocks are allocated, and none are ever freed.  */

static void *alloc_ptr, *alloc_end, *alloc_last_block;

void *
malloc (size_t n)
{
  extern int _dl_zerofd;
  static size_t pagesize;

  if (pagesize == 0)
    pagesize = __getpagesize ();

  if (_dl_zerofd == -1)
    _dl_zerofd = _dl_sysdep_open_zero_fill ();

  if (alloc_end == 0)
    {
      /* Consume any unused space in the last page of our data segment.  */
      extern int _end;
      alloc_ptr = &_end;
      alloc_end = (void *) 0 + (((alloc_ptr - (void *) 0) + pagesize - 1)
				& ~(pagesize - 1));
    }

  /* Make sure the allocation pointer is ideally aligned.  */
  alloc_ptr = (void *) 0 + (((alloc_ptr - (void *) 0) + sizeof (double) - 1)
			    & ~(sizeof (double) - 1));

  if (alloc_ptr + n >= alloc_end)
    {
      /* Insufficient space left; allocate another page.  */
      caddr_t page;
      assert (n <= pagesize);
      page = mmap (0, pagesize, PROT_READ|PROT_WRITE,
		   MAP_ANON|MAP_PRIVATE, _dl_zerofd, 0);
      assert (page != (caddr_t) -1);
      if (page != alloc_end)
	alloc_ptr = page;
      alloc_end = page + pagesize;
    }

  alloc_last_block = (void *) alloc_ptr;
  alloc_ptr += n;
  return alloc_last_block;
}
weak_symbol (malloc)

/* This will rarely be called.  */
void
free (void *ptr)
{
  /* We can free only the last block allocated.  */
  if (ptr == alloc_last_block)
    alloc_ptr = alloc_last_block;
}
weak_symbol (free)

/* This is never called.  */
void *
realloc (void *ptr, size_t n)
{ ptr += n; abort (); }
weak_symbol (realloc)

/* Avoid signal frobnication in setjmp/longjmp.  Keeps things smaller.  */

#include <setjmp.h>

int __sigjmp_save (sigjmp_buf env, int savemask)
{ env[0].__mask_was_saved = savemask; return 0; }
weak_symbol (__sigjmp_save)

void
longjmp (jmp_buf env, int val) { __longjmp (env[0].__jmpbuf, val); }
weak_symbol (longjmp)


/* Define our own stub for the localization function used by strerror.
   English-only in the dynamic linker keeps it smaller.  */

char *
__dgettext (const char *domainname, const char *msgid)
{
  assert (domainname == _libc_intl_domainname);
  return (char *) msgid;
}
weak_symbol (__dgettext)
weak_alias (__dgettext, dgettext)

#ifndef NDEBUG

/* Define (weakly) our own assert failure function which doesn't use stdio.
   If we are linked into the user program (-ldl), the normal __assert_fail
   defn can override this one.  */

void
__assert_fail (const char *assertion,
	       const char *file, unsigned int line, const char *function)
{
  char buf[64];
  buf[sizeof buf - 1] = '\0';
  _dl_sysdep_fatal ("BUG IN DYNAMIC LINKER ld.so: ",
		    file, ": ", _itoa (line, buf + sizeof buf - 1, 10, 0),
		    ": ", function ?: "", function ? ": " : "",
		    "Assertion `", assertion, "' failed!\n",
		    NULL);

}
weak_symbol (__assert_fail)

void
__assert_perror_fail (int errnum,
		      const char *file, unsigned int line,
		      const char *function)
{
  char buf[64];
  buf[sizeof buf - 1] = '\0';
  _dl_sysdep_fatal ("BUG IN DYNAMIC LINKER ld.so: ",
		    file, ": ", _itoa (line, buf + sizeof buf - 1, 10, 0),
		    ": ", function ?: "", function ? ": " : "",
		    "Unexpected error: ", strerror (errnum), "\n", NULL);

}
weak_symbol (__assert_perror_fail)

#endif
