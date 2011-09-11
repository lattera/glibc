/* Malloc implementation for multiple threads without lock contention.
   Copyright (C) 2001-2006, 2007, 2008, 2009, 2011 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Wolfram Gloger <wg@malloc.de>, 2001.

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

/* What to do if the standard debugging hooks are in place and a
   corrupt pointer is detected: do nothing (0), print an error message
   (1), or call abort() (2). */

/* Hooks for debugging versions.  The initial hooks just call the
   initialization routine, then do the normal work. */

static void*
malloc_hook_ini(size_t sz, const __malloc_ptr_t caller)
{
  __malloc_hook = NULL;
  ptmalloc_init();
  return public_mALLOc(sz);
}

static void*
realloc_hook_ini(void* ptr, size_t sz, const __malloc_ptr_t caller)
{
  __malloc_hook = NULL;
  __realloc_hook = NULL;
  ptmalloc_init();
  return public_rEALLOc(ptr, sz);
}

static void*
memalign_hook_ini(size_t alignment, size_t sz, const __malloc_ptr_t caller)
{
  __memalign_hook = NULL;
  ptmalloc_init();
  return public_mEMALIGn(alignment, sz);
}

/* Whether we are using malloc checking.  */
static int using_malloc_checking;

/* A flag that is set by malloc_set_state, to signal that malloc checking
   must not be enabled on the request from the user (via the MALLOC_CHECK_
   environment variable).  It is reset by __malloc_check_init to tell
   malloc_set_state that the user has requested malloc checking.

   The purpose of this flag is to make sure that malloc checking is not
   enabled when the heap to be restored was constructed without malloc
   checking, and thus does not contain the required magic bytes.
   Otherwise the heap would be corrupted by calls to free and realloc.  If
   it turns out that the heap was created with malloc checking and the
   user has requested it malloc_set_state just calls __malloc_check_init
   again to enable it.  On the other hand, reusing such a heap without
   further malloc checking is safe.  */
static int disallow_malloc_check;

/* Activate a standard set of debugging hooks. */
void
__malloc_check_init()
{
  if (disallow_malloc_check) {
    disallow_malloc_check = 0;
    return;
  }
  using_malloc_checking = 1;
  __malloc_hook = malloc_check;
  __free_hook = free_check;
  __realloc_hook = realloc_check;
  __memalign_hook = memalign_check;
}

/* A simple, standard set of debugging hooks.  Overhead is `only' one
   byte per chunk; still this will catch most cases of double frees or
   overruns.  The goal here is to avoid obscure crashes due to invalid
   usage, unlike in the MALLOC_DEBUG code. */

#define MAGICBYTE(p) ( ( ((size_t)p >> 3) ^ ((size_t)p >> 11)) & 0xFF )

/* Instrument a chunk with overrun detector byte(s) and convert it
   into a user pointer with requested size sz. */

static void*
internal_function
mem2mem_check(void *ptr, size_t sz)
{
  mchunkptr p;
  unsigned char* m_ptr = (unsigned char*)BOUNDED_N(ptr, sz);
  size_t i;

  if (!ptr)
    return ptr;
  p = mem2chunk(ptr);
  for(i = chunksize(p) - (chunk_is_mmapped(p) ? 2*SIZE_SZ+1 : SIZE_SZ+1);
      i > sz;
      i -= 0xFF) {
    if(i-sz < 0x100) {
      m_ptr[i] = (unsigned char)(i-sz);
      break;
    }
    m_ptr[i] = 0xFF;
  }
  m_ptr[sz] = MAGICBYTE(p);
  return (void*)m_ptr;
}

/* Convert a pointer to be free()d or realloc()ed to a valid chunk
   pointer.  If the provided pointer is not valid, return NULL. */

static mchunkptr
internal_function
mem2chunk_check(void* mem, unsigned char **magic_p)
{
  mchunkptr p;
  INTERNAL_SIZE_T sz, c;
  unsigned char magic;

  if(!aligned_OK(mem)) return NULL;
  p = mem2chunk(mem);
  if (!chunk_is_mmapped(p)) {
    /* Must be a chunk in conventional heap memory. */
    int contig = contiguous(&main_arena);
    sz = chunksize(p);
    if((contig &&
	((char*)p<mp_.sbrk_base ||
	 ((char*)p + sz)>=(mp_.sbrk_base+main_arena.system_mem) )) ||
       sz<MINSIZE || sz&MALLOC_ALIGN_MASK || !inuse(p) ||
       ( !prev_inuse(p) && (p->prev_size&MALLOC_ALIGN_MASK ||
			    (contig && (char*)prev_chunk(p)<mp_.sbrk_base) ||
			    next_chunk(prev_chunk(p))!=p) ))
      return NULL;
    magic = MAGICBYTE(p);
    for(sz += SIZE_SZ-1; (c = ((unsigned char*)p)[sz]) != magic; sz -= c) {
      if(c<=0 || sz<(c+2*SIZE_SZ)) return NULL;
    }
  } else {
    unsigned long offset, page_mask = GLRO(dl_pagesize)-1;

    /* mmap()ed chunks have MALLOC_ALIGNMENT or higher power-of-two
       alignment relative to the beginning of a page.  Check this
       first. */
    offset = (unsigned long)mem & page_mask;
    if((offset!=MALLOC_ALIGNMENT && offset!=0 && offset!=0x10 &&
	offset!=0x20 && offset!=0x40 && offset!=0x80 && offset!=0x100 &&
	offset!=0x200 && offset!=0x400 && offset!=0x800 && offset!=0x1000 &&
	offset<0x2000) ||
       !chunk_is_mmapped(p) || (p->size & PREV_INUSE) ||
       ( (((unsigned long)p - p->prev_size) & page_mask) != 0 ) ||
       ( (sz = chunksize(p)), ((p->prev_size + sz) & page_mask) != 0 ) )
      return NULL;
    magic = MAGICBYTE(p);
    for(sz -= 1; (c = ((unsigned char*)p)[sz]) != magic; sz -= c) {
      if(c<=0 || sz<(c+2*SIZE_SZ)) return NULL;
    }
  }
  ((unsigned char*)p)[sz] ^= 0xFF;
  if (magic_p)
    *magic_p = (unsigned char *)p + sz;
  return p;
}

/* Check for corruption of the top chunk, and try to recover if
   necessary. */

static int
internal_function
top_check(void)
{
  mchunkptr t = top(&main_arena);
  char* brk, * new_brk;
  INTERNAL_SIZE_T front_misalign, sbrk_size;
  unsigned long pagesz = GLRO(dl_pagesize);

  if (t == initial_top(&main_arena) ||
      (!chunk_is_mmapped(t) &&
       chunksize(t)>=MINSIZE &&
       prev_inuse(t) &&
       (!contiguous(&main_arena) ||
	(char*)t + chunksize(t) == mp_.sbrk_base + main_arena.system_mem)))
    return 0;

  malloc_printerr (check_action, "malloc: top chunk is corrupt", t);

  /* Try to set up a new top chunk. */
  brk = MORECORE(0);
  front_misalign = (unsigned long)chunk2mem(brk) & MALLOC_ALIGN_MASK;
  if (front_misalign > 0)
    front_misalign = MALLOC_ALIGNMENT - front_misalign;
  sbrk_size = front_misalign + mp_.top_pad + MINSIZE;
  sbrk_size += pagesz - ((unsigned long)(brk + sbrk_size) & (pagesz - 1));
  new_brk = (char*)(MORECORE (sbrk_size));
  if (new_brk == (char*)(MORECORE_FAILURE))
    {
      __set_errno (ENOMEM);
      return -1;
    }
  /* Call the `morecore' hook if necessary.  */
  void (*hook) (void) = force_reg (__after_morecore_hook);
  if (hook)
    (*hook) ();
  main_arena.system_mem = (new_brk - mp_.sbrk_base) + sbrk_size;

  top(&main_arena) = (mchunkptr)(brk + front_misalign);
  set_head(top(&main_arena), (sbrk_size - front_misalign) | PREV_INUSE);

  return 0;
}

static void*
malloc_check(size_t sz, const void *caller)
{
  void *victim;

  if (sz+1 == 0) {
    __set_errno (ENOMEM);
    return NULL;
  }

  (void)mutex_lock(&main_arena.mutex);
  victim = (top_check() >= 0) ? _int_malloc(&main_arena, sz+1) : NULL;
  (void)mutex_unlock(&main_arena.mutex);
  return mem2mem_check(victim, sz);
}

static void
free_check(void* mem, const void *caller)
{
  mchunkptr p;

  if(!mem) return;
  (void)mutex_lock(&main_arena.mutex);
  p = mem2chunk_check(mem, NULL);
  if(!p) {
    (void)mutex_unlock(&main_arena.mutex);

    malloc_printerr(check_action, "free(): invalid pointer", mem);
    return;
  }
  if (chunk_is_mmapped(p)) {
    (void)mutex_unlock(&main_arena.mutex);
    munmap_chunk(p);
    return;
  }
  _int_free(&main_arena, p, 1);
  (void)mutex_unlock(&main_arena.mutex);
}

static void*
realloc_check(void* oldmem, size_t bytes, const void *caller)
{
  INTERNAL_SIZE_T nb;
  void* newmem = 0;
  unsigned char *magic_p;

  if (bytes+1 == 0) {
    __set_errno (ENOMEM);
    return NULL;
  }
  if (oldmem == 0) return malloc_check(bytes, NULL);
  if (bytes == 0) {
    free_check (oldmem, NULL);
    return NULL;
  }
  (void)mutex_lock(&main_arena.mutex);
  const mchunkptr oldp = mem2chunk_check(oldmem, &magic_p);
  (void)mutex_unlock(&main_arena.mutex);
  if(!oldp) {
    malloc_printerr(check_action, "realloc(): invalid pointer", oldmem);
    return malloc_check(bytes, NULL);
  }
  const INTERNAL_SIZE_T oldsize = chunksize(oldp);

  checked_request2size(bytes+1, nb);
  (void)mutex_lock(&main_arena.mutex);

  if (chunk_is_mmapped(oldp)) {
#if HAVE_MREMAP
    mchunkptr newp = mremap_chunk(oldp, nb);
    if(newp)
      newmem = chunk2mem(newp);
    else
#endif
    {
      /* Note the extra SIZE_SZ overhead. */
      if(oldsize - SIZE_SZ >= nb)
	newmem = oldmem; /* do nothing */
      else {
	/* Must alloc, copy, free. */
	if (top_check() >= 0)
	  newmem = _int_malloc(&main_arena, bytes+1);
	if (newmem) {
	  MALLOC_COPY(BOUNDED_N(newmem, bytes+1), oldmem, oldsize - 2*SIZE_SZ);
	  munmap_chunk(oldp);
	}
      }
    }
  } else {
    if (top_check() >= 0) {
      INTERNAL_SIZE_T nb;
      checked_request2size(bytes + 1, nb);
      newmem = _int_realloc(&main_arena, oldp, oldsize, nb);
    }
  }

  /* mem2chunk_check changed the magic byte in the old chunk.
     If newmem is NULL, then the old chunk will still be used though,
     so we need to invert that change here.  */
  if (newmem == NULL) *magic_p ^= 0xFF;

  (void)mutex_unlock(&main_arena.mutex);

  return mem2mem_check(newmem, bytes);
}

static void*
memalign_check(size_t alignment, size_t bytes, const void *caller)
{
  void* mem;

  if (alignment <= MALLOC_ALIGNMENT) return malloc_check(bytes, NULL);
  if (alignment <  MINSIZE) alignment = MINSIZE;

  if (bytes+1 == 0) {
    __set_errno (ENOMEM);
    return NULL;
  }
  (void)mutex_lock(&main_arena.mutex);
  mem = (top_check() >= 0) ? _int_memalign(&main_arena, alignment, bytes+1) :
    NULL;
  (void)mutex_unlock(&main_arena.mutex);
  return mem2mem_check(mem, bytes);
}


/* Get/set state: malloc_get_state() records the current state of all
   malloc variables (_except_ for the actual heap contents and `hook'
   function pointers) in a system dependent, opaque data structure.
   This data structure is dynamically allocated and can be free()d
   after use.  malloc_set_state() restores the state of all malloc
   variables to the previously obtained state.  This is especially
   useful when using this malloc as part of a shared library, and when
   the heap contents are saved/restored via some other method.  The
   primary example for this is GNU Emacs with its `dumping' procedure.
   `Hook' function pointers are never saved or restored by these
   functions, with two exceptions: If malloc checking was in use when
   malloc_get_state() was called, then malloc_set_state() calls
   __malloc_check_init() if possible; if malloc checking was not in
   use in the recorded state but the user requested malloc checking,
   then the hooks are reset to 0.  */

#define MALLOC_STATE_MAGIC   0x444c4541l
#define MALLOC_STATE_VERSION (0*0x100l + 4l) /* major*0x100 + minor */

struct malloc_save_state {
  long          magic;
  long          version;
  mbinptr       av[NBINS * 2 + 2];
  char*         sbrk_base;
  int           sbrked_mem_bytes;
  unsigned long trim_threshold;
  unsigned long top_pad;
  unsigned int  n_mmaps_max;
  unsigned long mmap_threshold;
  int           check_action;
  unsigned long max_sbrked_mem;
  unsigned long max_total_mem;
  unsigned int  n_mmaps;
  unsigned int  max_n_mmaps;
  unsigned long mmapped_mem;
  unsigned long max_mmapped_mem;
  int           using_malloc_checking;
  unsigned long max_fast;
  unsigned long arena_test;
  unsigned long arena_max;
  unsigned long narenas;
};

void*
public_gET_STATe(void)
{
  struct malloc_save_state* ms;
  int i;
  mbinptr b;

  ms = (struct malloc_save_state*)public_mALLOc(sizeof(*ms));
  if (!ms)
    return 0;
  (void)mutex_lock(&main_arena.mutex);
  malloc_consolidate(&main_arena);
  ms->magic = MALLOC_STATE_MAGIC;
  ms->version = MALLOC_STATE_VERSION;
  ms->av[0] = 0;
  ms->av[1] = 0; /* used to be binblocks, now no longer used */
  ms->av[2] = top(&main_arena);
  ms->av[3] = 0; /* used to be undefined */
  for(i=1; i<NBINS; i++) {
    b = bin_at(&main_arena, i);
    if(first(b) == b)
      ms->av[2*i+2] = ms->av[2*i+3] = 0; /* empty bin */
    else {
      ms->av[2*i+2] = first(b);
      ms->av[2*i+3] = last(b);
    }
  }
  ms->sbrk_base = mp_.sbrk_base;
  ms->sbrked_mem_bytes = main_arena.system_mem;
  ms->trim_threshold = mp_.trim_threshold;
  ms->top_pad = mp_.top_pad;
  ms->n_mmaps_max = mp_.n_mmaps_max;
  ms->mmap_threshold = mp_.mmap_threshold;
  ms->check_action = check_action;
  ms->max_sbrked_mem = main_arena.max_system_mem;
  ms->max_total_mem = 0;
  ms->n_mmaps = mp_.n_mmaps;
  ms->max_n_mmaps = mp_.max_n_mmaps;
  ms->mmapped_mem = mp_.mmapped_mem;
  ms->max_mmapped_mem = mp_.max_mmapped_mem;
  ms->using_malloc_checking = using_malloc_checking;
  ms->max_fast = get_max_fast();
#ifdef PER_THREAD
  ms->arena_test = mp_.arena_test;
  ms->arena_max = mp_.arena_max;
  ms->narenas = narenas;
#endif
  (void)mutex_unlock(&main_arena.mutex);
  return (void*)ms;
}

int
public_sET_STATe(void* msptr)
{
  struct malloc_save_state* ms = (struct malloc_save_state*)msptr;
  size_t i;
  mbinptr b;

  disallow_malloc_check = 1;
  ptmalloc_init();
  if(ms->magic != MALLOC_STATE_MAGIC) return -1;
  /* Must fail if the major version is too high. */
  if((ms->version & ~0xffl) > (MALLOC_STATE_VERSION & ~0xffl)) return -2;
  (void)mutex_lock(&main_arena.mutex);
  /* There are no fastchunks.  */
  clear_fastchunks(&main_arena);
  if (ms->version >= 4)
    set_max_fast(ms->max_fast);
  else
    set_max_fast(64);	/* 64 used to be the value we always used.  */
  for (i=0; i<NFASTBINS; ++i)
    fastbin (&main_arena, i) = 0;
  for (i=0; i<BINMAPSIZE; ++i)
    main_arena.binmap[i] = 0;
  top(&main_arena) = ms->av[2];
  main_arena.last_remainder = 0;
  for(i=1; i<NBINS; i++) {
    b = bin_at(&main_arena, i);
    if(ms->av[2*i+2] == 0) {
      assert(ms->av[2*i+3] == 0);
      first(b) = last(b) = b;
    } else {
      if(ms->version >= 3 &&
	 (i<NSMALLBINS || (largebin_index(chunksize(ms->av[2*i+2]))==i &&
			   largebin_index(chunksize(ms->av[2*i+3]))==i))) {
	first(b) = ms->av[2*i+2];
	last(b) = ms->av[2*i+3];
	/* Make sure the links to the bins within the heap are correct.  */
	first(b)->bk = b;
	last(b)->fd = b;
	/* Set bit in binblocks.  */
	mark_bin(&main_arena, i);
      } else {
	/* Oops, index computation from chunksize must have changed.
	   Link the whole list into unsorted_chunks.  */
	first(b) = last(b) = b;
	b = unsorted_chunks(&main_arena);
	ms->av[2*i+2]->bk = b;
	ms->av[2*i+3]->fd = b->fd;
	b->fd->bk = ms->av[2*i+3];
	b->fd = ms->av[2*i+2];
      }
    }
  }
  if (ms->version < 3) {
    /* Clear fd_nextsize and bk_nextsize fields.  */
    b = unsorted_chunks(&main_arena)->fd;
    while (b != unsorted_chunks(&main_arena)) {
      if (!in_smallbin_range(chunksize(b))) {
	b->fd_nextsize = NULL;
	b->bk_nextsize = NULL;
      }
      b = b->fd;
    }
  }
  mp_.sbrk_base = ms->sbrk_base;
  main_arena.system_mem = ms->sbrked_mem_bytes;
  mp_.trim_threshold = ms->trim_threshold;
  mp_.top_pad = ms->top_pad;
  mp_.n_mmaps_max = ms->n_mmaps_max;
  mp_.mmap_threshold = ms->mmap_threshold;
  check_action = ms->check_action;
  main_arena.max_system_mem = ms->max_sbrked_mem;
  mp_.n_mmaps = ms->n_mmaps;
  mp_.max_n_mmaps = ms->max_n_mmaps;
  mp_.mmapped_mem = ms->mmapped_mem;
  mp_.max_mmapped_mem = ms->max_mmapped_mem;
  /* add version-dependent code here */
  if (ms->version >= 1) {
    /* Check whether it is safe to enable malloc checking, or whether
       it is necessary to disable it.  */
    if (ms->using_malloc_checking && !using_malloc_checking &&
	!disallow_malloc_check)
      __malloc_check_init ();
    else if (!ms->using_malloc_checking && using_malloc_checking) {
      __malloc_hook = NULL;
      __free_hook = NULL;
      __realloc_hook = NULL;
      __memalign_hook = NULL;
      using_malloc_checking = 0;
    }
  }
  if (ms->version >= 4) {
#ifdef PER_THREAD
    mp_.arena_test = ms->arena_test;
    mp_.arena_max = ms->arena_max;
    narenas = ms->narenas;
#endif
  }
  check_malloc_state(&main_arena);

  (void)mutex_unlock(&main_arena.mutex);
  return 0;
}

/*
 * Local variables:
 * c-basic-offset: 2
 * End:
 */
