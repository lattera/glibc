/* Manage function descriptors.  IA-64 version.
   Copyright (C) 1999, 2000, 2001 Free Software Foundation, Inc.
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

#include <ia64intrin.h>
#include <unistd.h>
#include <string.h>
#include <sys/param.h>
#include <sys/mman.h>
#include <link.h>
#include <ldsodefs.h>
#include <elf/dynamic-link.h>
#include <dl-machine.h>
#ifdef _LIBC_REENTRANT
# include <pt-machine.h>
# include <signal.h>
# include <time.h>
#endif

Elf64_Addr __ia64_boot_fptr_table[IA64_BOOT_FPTR_TABLE_LEN];

static struct local
  {
    struct ia64_fdesc_table *root;
    struct ia64_fdesc *free_list;
    unsigned int npages;		/* # of pages to allocate */
#ifdef _LIBC_REENTRANT
    volatile int lock;
    sigset_t full_sigset;
#endif
    /* the next to members MUST be consecutive! */
    struct ia64_fdesc_table boot_table;
    struct ia64_fdesc boot_fdescs[1024];
  }
local =
  {
    root: &local.boot_table,
    npages: 2,
    boot_table:
      {
	len: sizeof (local.boot_fdescs) / sizeof (local.boot_fdescs[0]),
	first_unused: 0
      }
  };

/* Locking is tricky: we may get a signal while holding the lock and
   the signal handler may end up calling into the dynamic loader
   again.  Also, if a real-time process spins on the lock, a
   non-realtime process may never get the chance to release it's lock,
   unless the realtime process relinquishes the CPU from time to time.
   Hence we (a) block signals before acquiring the lock and (b) do a
   nanosleep() when we detect prolongued contention.  */
#ifdef _LIBC_REENTRANT
# define lock(l)						\
{								\
  sigset_t _saved_set;						\
  int i = 10000;						\
  if (!__sigismember (&(l)->full_sigset, SIGINT))		\
    __sigfillset (&(l)->full_sigset);				\
								\
  while (testandset ((int *) &(l)->lock))			\
    {								\
      struct timespec ts;					\
      if (i > 0)						\
	{							\
	  --i;							\
	  continue;						\
	}							\
      ts.tv_sec = 0;						\
      ts.tv_nsec = 1*1000*1000;					\
      __nanosleep (&ts, NULL);					\
    }								\
  __sigprocmask (SIG_BLOCK, &(l)->full_sigset, &_saved_set);
# define unlock(l)						\
  __sigprocmask (SIG_SETMASK, &_saved_set, NULL);		\
  (l)->lock = 0;						\
}
#else
# define lock(l)
# define unlock(l)
#endif

/* Create a new fdesc table and return a pointer to the first fdesc
   entry.  The fdesc lock must have been acquired already.  */

static struct ia64_fdesc *
new_fdesc_table (struct local *l)
{
  size_t size = l->npages * _dl_pagesize;
  struct ia64_fdesc_table *new_table;
  struct ia64_fdesc *fdesc;

  l->npages += l->npages;
  new_table = __mmap (0, size, PROT_READ | PROT_WRITE,
		      MAP_ANON | MAP_PRIVATE, -1, 0);
  if (new_table == MAP_FAILED)
    _dl_signal_error (errno, NULL, "cannot map pages for fdesc table");

  new_table->len = (size - sizeof (*new_table)) / sizeof (struct ia64_fdesc);
  fdesc = &new_table->fdesc[0];
  new_table->first_unused = 1;
  new_table->next = l->root;
  l->root = new_table;
  return fdesc;
}

static Elf64_Addr
make_fdesc (Elf64_Addr ip, Elf64_Addr gp)
{
  struct ia64_fdesc *fdesc = NULL;
  struct ia64_fdesc_table *t;
  unsigned int old;
  struct local *l;

  asm ("addl %0 = @gprel (local), gp" : "=r" (l));

  t = l->root;
  while (1)
    {
      old = t->first_unused;
      if (old >= t->len)
	break;
      else if (__sync_bool_compare_and_swap (&t->first_unused, old, old + 1))
	{
	  fdesc = &t->fdesc[old];
	  goto install;
	}
    }

  lock (l);
  {
    if (l->free_list)
      {
	fdesc = l->free_list;		/* get it from free-list */
	l->free_list = (struct ia64_fdesc *) fdesc->ip;
      }
    else
      fdesc = new_fdesc_table (l);	/* create new fdesc table */
  }
  unlock (l);

 install:
  fdesc->ip = ip;
  fdesc->gp = gp;

  return (Elf64_Addr) fdesc;
}

static inline Elf64_Addr *
make_fptr_table (struct link_map *map)
{
  const Elf64_Sym *symtab = (const void *) D_PTR (map, l_info[DT_SYMTAB]);
  const char *strtab = (const void *) D_PTR (map, l_info[DT_STRTAB]);
  Elf64_Addr *fptr_table;
  size_t size;
  size_t len;

  /* XXX Apparently the only way to find out the size of the dynamic
     symbol section is to assume that the string table follows right
     afterwards...  */
  len = ((strtab - (char *) symtab) / map->l_info[DT_SYMENT]->d_un.d_val);
  size = ((len * sizeof (fptr_table[0]) + _dl_pagesize - 1) & -_dl_pagesize);
  /* XXX We don't support here in the moment systems without MAP_ANON.
     There probably are none for IA-64.  In case this is proven wrong
     we will have to open /dev/null here and use the file descriptor
     instead of the hard-coded -1.  */
  fptr_table = __mmap (NULL, size, PROT_READ | PROT_WRITE,
		       MAP_ANON | MAP_PRIVATE, -1, 0);
  if (fptr_table == MAP_FAILED)
    _dl_signal_error (errno, NULL, "cannot map pages for fptr table");

  map->l_mach.fptr_table_len = len;
  map->l_mach.fptr_table = fptr_table;
  return fptr_table;
}

Elf64_Addr
__ia64_make_fptr (struct link_map *map, const Elf64_Sym *sym, Elf64_Addr ip)
{
  Elf64_Addr *ftab = map->l_mach.fptr_table;
  const Elf64_Sym *symtab;
  Elf_Symndx symidx;

  if (__builtin_expect (!map->l_mach.fptr_table, 0))
    ftab = make_fptr_table (map);

  symtab = (const void *) D_PTR (map, l_info[DT_SYMTAB]);
  symidx = sym - symtab;

  if (symidx >= map->l_mach.fptr_table_len)
    _dl_signal_error (0, NULL,
		      "internal error: symidx out of range of fptr table");

  if (!ftab[symidx])
    {
      /* GOT has already been relocated in elf_get_dynamic_info -
	 don't try to relocate it again.  */
      ftab[symidx] = make_fdesc (ip, map->l_info[DT_PLTGOT]->d_un.d_ptr);
#if 0
      {
	const char *strtab = (const void *) D_PTR (map, l_info[DT_STRTAB]);
	struct local *l;

	asm ("addl %0 = @gprel (local), gp" : "=r" (l));
	if (l->root != &l->boot_table || l->boot_table.first_unused > 20)
	  _dl_debug_printf ("created fdesc symbol `%s' at %lx\n",
			    strtab + sym->st_name, ftab[symidx]);
      }
#endif
    }

  return ftab[symidx];
}

void
_dl_unmap (struct link_map *map)
{
  Elf64_Addr *ftab = map->l_mach.fptr_table;
  struct ia64_fdesc *head = NULL, *tail = NULL;
  size_t i;

  __munmap ((void *) map->l_map_start, map->l_map_end - map->l_map_start);

  if (!ftab)
    return;

  /* String together the fdesc structures that are being freed.  */
  for (i = 0; i < map->l_mach.fptr_table_len; ++i)
    {
      if (ftab[i])
	{
	  *(struct ia64_fdesc **) ftab[i] = head;
	  head = (struct ia64_fdesc *) ftab[i];
	  if (!tail)
	    tail = head;
	}
    }

  /* Prepend the new list to the free_list: */
  if (tail)
    {
      lock (&local);
      {
	*(struct ia64_fdesc **) tail = local.free_list;
	local.free_list = head;
      }
      unlock (&local);
    }

  __munmap (ftab,
	    map->l_mach.fptr_table_len * sizeof (map->l_mach.fptr_table[0]));
  map->l_mach.fptr_table = NULL;
}

Elf64_Addr
_dl_lookup_address (const void *address)
{
  Elf64_Addr addr = (Elf64_Addr) address;
  struct ia64_fdesc_table *t;
  unsigned long int i;

  for (t = local.root; t != NULL; t = t->next)
    {
      i = (struct ia64_fdesc *) addr - &t->fdesc[0];
      if (i < t->first_unused && addr == (Elf64_Addr) &t->fdesc[i])
	{
	  addr = t->fdesc[i].ip;
	  break;
	}
    }
  return addr;
}
