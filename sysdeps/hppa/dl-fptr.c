/* Make dynamic PLABELs for function pointers. HPPA version.
   Copyright (C) 1999, 2000 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <unistd.h>
#include <string.h>
#include <sys/param.h>
#include <sys/mman.h>
#include <link.h>
#include <errno.h>
#include <ldsodefs.h>
#include <elf/dynamic-link.h>
#include <dl-machine.h>
#ifdef _LIBC_REENTRANT
# include <pt-machine.h>

/* Remember, we use 0 to mean that a lock is taken on PA-RISC. */
static int __hppa_fptr_lock = 1;
#endif

/* Because ld.so is now versioned, these functions can be in their own
   file; no relocations need to be done to call them.  Of course, if
   ld.so is not versioned...  */
#if 0
#ifndef DO_VERSIONING
# error "This will not work with versioning turned off, sorry."
#endif
#endif

#ifdef MAP_ANON
/* The fd is not examined when using MAP_ANON.  */
#define ANONFD -1
#else
extern int _dl_zerofd;
#define ANONFD _dl_zerofd
#endif

struct hppa_fptr __boot_ldso_fptr[HPPA_BOOT_FPTR_SIZE];
struct hppa_fptr *__fptr_root = NULL;
struct hppa_fptr *__fptr_next = __boot_ldso_fptr;
static struct hppa_fptr *__fptr_free = NULL;
int __fptr_count = HPPA_BOOT_FPTR_SIZE;

Elf32_Addr
__hppa_make_fptr (const struct link_map *sym_map, Elf32_Addr value,
		  struct hppa_fptr **root, struct hppa_fptr *mem)
{
  struct hppa_fptr **loc;
  struct hppa_fptr *f;

#ifdef _LIBC_REENTRANT
  /* Make sure we are alone. We don't need a lock during bootstrap. */
  if (mem == NULL)
    while (testandset (&__hppa_fptr_lock));
#endif

  /* Search the sorted linked list for an existing entry for this
     symbol.  */
  loc = root;
  f = *loc;
  while (f != NULL && f->func <= value)
    {
      if (f->func == value)
	goto found;
      loc = &f->next;
      f = *loc;
    }

  /* Not found.  Create a new one.  */
  if (mem != NULL)
    f = mem;
  else if (__fptr_free != NULL)
    {
      f = __fptr_free;
      __fptr_free = f->next;
    }
  else
    {
      if (__fptr_count == 0)
	{
#ifndef MAP_ANON
# define MAP_ANON 0
	  if (_dl_zerofd == -1)
	    {
	      _dl_zerofd = _dl_sysdep_open_zero_fill ();
	      if (_dl_zerofd == -1)
		{
		  __close (fd);
		  _dl_signal_error (errno, NULL,
				    "cannot open zero fill device");
		}
	    }
#endif

	  __fptr_next = __mmap (0, _dl_pagesize, PROT_READ | PROT_WRITE,
				MAP_ANON | MAP_PRIVATE, ANONFD, 0);
	  if (__fptr_next == MAP_FAILED)
	    _dl_signal_error(errno, NULL, "cannot map page for fptr");
	  __fptr_count = _dl_pagesize / sizeof (struct hppa_fptr);
	}
      f = __fptr_next++;
      __fptr_count--;
    }

  f->func = value;
  /* GOT has already been relocated in elf_get_dynamic_info - don't
     try to relocate it again.  */
  f->gp = sym_map->l_info[DT_PLTGOT]->d_un.d_ptr;
  f->next = *loc;
  *loc = f;

found:
#ifdef _LIBC_REENTRANT
  /* Release the lock.  Again, remember, zero means the lock is taken!  */
  if (mem == NULL)
    __hppa_fptr_lock = 1;
#endif

  /* Set bit 30 to indicate to $$dyncall that this is a PLABEL. */
  return (Elf32_Addr) f | 2;
}

void
_dl_unmap (struct link_map *map)
{
  struct hppa_fptr **floc;
  struct hppa_fptr *f;
  struct hppa_fptr **lloc;
  struct hppa_fptr *l;

  __munmap ((void *) map->l_map_start, map->l_map_end - map->l_map_start);

#ifdef _LIBC_REENTRANT
  /* Make sure we are alone.  */
  while (testandset (&__hppa_fptr_lock));
#endif

  /* Search the sorted linked list for the first entry for this object.  */
  floc = &__fptr_root;
  f = *floc;
  while (f != NULL && f->func < map->l_map_start)
    {
      floc = &f->next;
      f = *floc;
    }

  /* We found one.  */
  if (f != NULL && f->func < map->l_map_end)
    {
      /* Get the last entry.  */
      lloc = floc;
      l = f;
      while (l && l->func < map->l_map_end)
	{
	  lloc = &l->next;
	  l = *lloc;
	}

      /* Updated FPTR.  */
      *floc = l;

      /* Prepend them to the free list.  */
      *lloc = __fptr_free;
      __fptr_free = f;
    }

#ifdef _LIBC_REENTRANT
  /* Release the lock. */
  __hppa_fptr_lock = 1;
#endif
}

Elf32_Addr
_dl_lookup_address (const void *address)
{
  Elf32_Addr addr = (Elf32_Addr) address;
  struct hppa_fptr *f;

#ifdef _LIBC_REENTRANT
  /* Make sure we are alone.  */
  while (testandset (&__hppa_fptr_lock));
#endif

  for (f = __fptr_root; f != NULL; f = f->next)
    if (f == address)
      {
	addr = f->func;
	break;
      }

#ifdef _LIBC_REENTRANT
  /* Release the lock.   */
  __hppa_fptr_lock = 1;
#endif

  return addr;
}
