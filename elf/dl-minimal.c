/* Minimal replacements for basic facilities used in the dynamic linker.
   Copyright (C) 1995, 1996 Free Software Foundation, Inc.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

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

void * weak_function
malloc (size_t n)
{
#ifdef MAP_ANON
#define	_dl_zerofd (-1)
#else
  extern int _dl_zerofd;

  if (_dl_zerofd == -1)
    _dl_zerofd = _dl_sysdep_open_zero_fill ();
#define MAP_ANON 0
#endif

  if (_dl_pagesize == 0)
    _dl_pagesize = __getpagesize ();

  if (alloc_end == 0)
    {
      /* Consume any unused space in the last page of our data segment.  */
      extern int _end;
      alloc_ptr = &_end;
      alloc_end = (void *) 0 + (((alloc_ptr - (void *) 0) + _dl_pagesize - 1)
				& ~(_dl_pagesize - 1));
    }

  /* Make sure the allocation pointer is ideally aligned.  */
  alloc_ptr = (void *) 0 + (((alloc_ptr - (void *) 0) + sizeof (double) - 1)
			    & ~(sizeof (double) - 1));

  if (alloc_ptr + n >= alloc_end)
    {
      /* Insufficient space left; allocate another page.  */
      caddr_t page;
      assert (n <= _dl_pagesize);
      page = __mmap (0, _dl_pagesize, PROT_READ|PROT_WRITE,
		     MAP_ANON|MAP_PRIVATE, _dl_zerofd, 0);
      assert (page != (caddr_t) -1);
      if (page != alloc_end)
	alloc_ptr = page;
      alloc_end = page + _dl_pagesize;
    }

  alloc_last_block = (void *) alloc_ptr;
  alloc_ptr += n;
  return alloc_last_block;
}

/* This will rarely be called.  */
void weak_function
free (void *ptr)
{
  /* We can free only the last block allocated.  */
  if (ptr == alloc_last_block)
    alloc_ptr = alloc_last_block;
}

/* This is only called with the most recent block returned by malloc.  */
void * weak_function
realloc (void *ptr, size_t n)
{
  void *new;
  assert (ptr == alloc_last_block);
  alloc_ptr = alloc_last_block;
  new = malloc (n);
  assert (new == ptr);
  return new;
}

/* Avoid signal frobnication in setjmp/longjmp.  Keeps things smaller.  */

#include <setjmp.h>

int weak_function
__sigjmp_save (sigjmp_buf env, int savemask)
{ env[0].__mask_was_saved = savemask; return 0; }

void weak_function
longjmp (jmp_buf env, int val) { __longjmp (env[0].__jmpbuf, val); }

/* Define our own stub for the localization function used by strerror.
   English-only in the dynamic linker keeps it smaller.  */

char * weak_function
__dcgettext (const char *domainname, const char *msgid, int category)
{
  assert (domainname == _libc_intl_domainname);
  return (char *) msgid;
}
weak_alias (__dcgettext, dcgettext)

#ifndef NDEBUG

/* Define (weakly) our own assert failure function which doesn't use stdio.
   If we are linked into the user program (-ldl), the normal __assert_fail
   defn can override this one.  */

void weak_function
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

void weak_function
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

#endif
