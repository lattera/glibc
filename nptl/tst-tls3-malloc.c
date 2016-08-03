/* Test TLS allocation with an interposed malloc.
   Copyright (C) 2016 Free Software Foundation, Inc.
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

/* Reuse the test.  */
#include "tst-tls3.c"

#include <sys/mman.h>

/* Interpose a minimal malloc implementation.  This implementation
   deliberately interposes just a restricted set of symbols, to detect
   if the TLS code bypasses the interposed malloc.  */

/* Lock to guard malloc internals.  */
static pthread_mutex_t malloc_lock = PTHREAD_MUTEX_INITIALIZER;

/* Information about an allocation chunk.  */
struct malloc_chunk
{
  /* Start of the allocation.  */
  void *start;
  /* Size of the allocation.  */
  size_t size;
};

enum { malloc_chunk_count = 1000 };
static struct malloc_chunk chunks[malloc_chunk_count];

/* Lock the malloc lock.  */
static void
xlock (void)
{
  int ret = pthread_mutex_lock (&malloc_lock);
  if (ret != 0)
    {
      errno = ret;
      printf ("error: pthread_mutex_lock: %m\n");
      _exit (1);
    }
}

/* Unlock the malloc lock.  */
static void
xunlock (void)
{
  int ret = pthread_mutex_unlock (&malloc_lock);
  if (ret != 0)
    {
      errno = ret;
      printf ("error: pthread_mutex_unlock: %m\n");
      _exit (1);
    }
}

/* Internal malloc without locking and registration.  */
static void *
malloc_internal (size_t size)
{
  void *result = mmap (NULL, size, PROT_READ | PROT_WRITE,
                       MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  if (result == MAP_FAILED)
    {
      printf ("error: mmap: %m\n");
      _exit (1);
    }
  return result;
}

void *
malloc (size_t size)
{
  if (size == 0)
    size = 1;
  xlock ();
  void *result = malloc_internal (size);
  for (int i = 0; i < malloc_chunk_count; ++i)
    if (chunks[i].start == NULL)
      {
        chunks[i].start = result;
        chunks[i].size = size;
        xunlock ();
        return result;
      }
  xunlock ();
  printf ("error: no place to store chunk pointer\n");
  _exit (1);
}

void *
calloc (size_t a, size_t b)
{
  if (b != 0 && a > SIZE_MAX / b)
    return NULL;
  /* malloc uses mmap, which provides zeroed memory.  */
  return malloc (a * b);
}

static void
xunmap (void *ptr, size_t size)
{
  int ret = munmap (ptr, size);
  if (ret < 0)
    {
      printf ("error: munmap (%p, %zu) failed: %m\n", ptr, size);
      _exit (1);
    }
}

void
free (void *ptr)
{
  if (ptr == NULL)
    return;

  xlock ();
  for (int i = 0; i < malloc_chunk_count; ++i)
    if (chunks[i].start == ptr)
      {
        xunmap (ptr, chunks[i].size);
        chunks[i] = (struct malloc_chunk) {};
        xunlock ();
        return;
      }
  xunlock ();
  printf ("error: tried to free non-allocated pointer %p\n", ptr);
  _exit (1);
}

void *
realloc (void *old, size_t size)
{
  if (old != NULL)
    {
      xlock ();
      for (int i = 0; i < malloc_chunk_count; ++i)
        if (chunks[i].start == old)
          {
            size_t old_size = chunks[i].size;
            void *result;
            if (old_size < size)
              {
                result = malloc_internal (size);
                /* Reuse the slot for the new allocation.  */
                memcpy (result, old, old_size);
                xunmap (old, old_size);
                chunks[i].start = result;
                chunks[i].size = size;
              }
            else
              /* Old size is not smaller, so reuse the old
                 allocation.  */
              result = old;
            xunlock ();
            return result;
          }
      xunlock ();
      printf ("error: tried to realloc non-allocated pointer %p\n", old);
      _exit (1);
    }
  else
    return malloc (size);
}
