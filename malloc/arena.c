/* Malloc implementation for multiple threads without lock contention.
   Copyright (C) 2001,2002,2003,2004,2005,2006,2007,2009,2010,2011
   Free Software Foundation, Inc.
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

#include <stdbool.h>

/* Compile-time constants.  */

#define HEAP_MIN_SIZE (32*1024)
#ifndef HEAP_MAX_SIZE
# ifdef DEFAULT_MMAP_THRESHOLD_MAX
#  define HEAP_MAX_SIZE (2 * DEFAULT_MMAP_THRESHOLD_MAX)
# else
#  define HEAP_MAX_SIZE (1024*1024) /* must be a power of two */
# endif
#endif

/* HEAP_MIN_SIZE and HEAP_MAX_SIZE limit the size of mmap()ed heaps
   that are dynamically created for multi-threaded programs.  The
   maximum size must be a power of two, for fast determination of
   which heap belongs to a chunk.  It should be much larger than the
   mmap threshold, so that requests with a size just below that
   threshold can be fulfilled without creating too many heaps.  */


#ifndef THREAD_STATS
#define THREAD_STATS 0
#endif

/* If THREAD_STATS is non-zero, some statistics on mutex locking are
   computed.  */

/***************************************************************************/

#define top(ar_ptr) ((ar_ptr)->top)

/* A heap is a single contiguous memory region holding (coalesceable)
   malloc_chunks.  It is allocated with mmap() and always starts at an
   address aligned to HEAP_MAX_SIZE.  */

typedef struct _heap_info {
  mstate ar_ptr; /* Arena for this heap. */
  struct _heap_info *prev; /* Previous heap. */
  size_t size;   /* Current size in bytes. */
  size_t mprotect_size;	/* Size in bytes that has been mprotected
			   PROT_READ|PROT_WRITE.  */
  /* Make sure the following data is properly aligned, particularly
     that sizeof (heap_info) + 2 * SIZE_SZ is a multiple of
     MALLOC_ALIGNMENT. */
  char pad[-6 * SIZE_SZ & MALLOC_ALIGN_MASK];
} heap_info;

/* Get a compile-time error if the heap_info padding is not correct
   to make alignment work as expected in sYSMALLOc.  */
extern int sanity_check_heap_info_alignment[(sizeof (heap_info)
					     + 2 * SIZE_SZ) % MALLOC_ALIGNMENT
					    ? -1 : 1];

/* Thread specific data */

static tsd_key_t arena_key;
static mutex_t list_lock = MUTEX_INITIALIZER;
#ifdef PER_THREAD
static size_t narenas = 1;
static mstate free_list;
#endif

#if THREAD_STATS
static int stat_n_heaps;
#define THREAD_STAT(x) x
#else
#define THREAD_STAT(x) do ; while(0)
#endif

/* Mapped memory in non-main arenas (reliable only for NO_THREADS). */
static unsigned long arena_mem;

/* Already initialized? */
int __malloc_initialized = -1;

/**************************************************************************/


/* arena_get() acquires an arena and locks the corresponding mutex.
   First, try the one last locked successfully by this thread.  (This
   is the common case and handled with a macro for speed.)  Then, loop
   once over the circularly linked list of arenas.  If no arena is
   readily available, create a new one.  In this latter case, `size'
   is just a hint as to how much memory will be required immediately
   in the new arena. */

#define arena_get(ptr, size) do { \
  arena_lookup(ptr); \
  arena_lock(ptr, size); \
} while(0)

#define arena_lookup(ptr) do { \
  void *vptr = NULL; \
  ptr = (mstate)tsd_getspecific(arena_key, vptr); \
} while(0)

#ifdef PER_THREAD
# define arena_lock(ptr, size) do { \
  if(ptr) \
    (void)mutex_lock(&ptr->mutex); \
  else \
    ptr = arena_get2(ptr, (size)); \
} while(0)
#else
# define arena_lock(ptr, size) do { \
  if(ptr && !mutex_trylock(&ptr->mutex)) { \
    THREAD_STAT(++(ptr->stat_lock_direct)); \
  } else \
    ptr = arena_get2(ptr, (size)); \
} while(0)
#endif

/* find the heap and corresponding arena for a given ptr */

#define heap_for_ptr(ptr) \
 ((heap_info *)((unsigned long)(ptr) & ~(HEAP_MAX_SIZE-1)))
#define arena_for_chunk(ptr) \
 (chunk_non_main_arena(ptr) ? heap_for_ptr(ptr)->ar_ptr : &main_arena)


/**************************************************************************/

/* atfork support.  */

static __malloc_ptr_t (*save_malloc_hook) (size_t __size,
					   __const __malloc_ptr_t);
static void           (*save_free_hook) (__malloc_ptr_t __ptr,
					 __const __malloc_ptr_t);
static void*        save_arena;

#ifdef ATFORK_MEM
ATFORK_MEM;
#endif

/* Magic value for the thread-specific arena pointer when
   malloc_atfork() is in use.  */

#define ATFORK_ARENA_PTR ((void*)-1)

/* The following hooks are used while the `atfork' handling mechanism
   is active. */

static void*
malloc_atfork(size_t sz, const void *caller)
{
  void *vptr = NULL;
  void *victim;

  tsd_getspecific(arena_key, vptr);
  if(vptr == ATFORK_ARENA_PTR) {
    /* We are the only thread that may allocate at all.  */
    if(save_malloc_hook != malloc_check) {
      return _int_malloc(&main_arena, sz);
    } else {
      if(top_check()<0)
	return 0;
      victim = _int_malloc(&main_arena, sz+1);
      return mem2mem_check(victim, sz);
    }
  } else {
    /* Suspend the thread until the `atfork' handlers have completed.
       By that time, the hooks will have been reset as well, so that
       mALLOc() can be used again. */
    (void)mutex_lock(&list_lock);
    (void)mutex_unlock(&list_lock);
    return public_mALLOc(sz);
  }
}

static void
free_atfork(void* mem, const void *caller)
{
  void *vptr = NULL;
  mstate ar_ptr;
  mchunkptr p;                          /* chunk corresponding to mem */

  if (mem == 0)                              /* free(0) has no effect */
    return;

  p = mem2chunk(mem);         /* do not bother to replicate free_check here */

  if (chunk_is_mmapped(p))                       /* release mmapped memory. */
  {
    munmap_chunk(p);
    return;
  }

  ar_ptr = arena_for_chunk(p);
  tsd_getspecific(arena_key, vptr);
  _int_free(ar_ptr, p, vptr == ATFORK_ARENA_PTR);
}


/* Counter for number of times the list is locked by the same thread.  */
static unsigned int atfork_recursive_cntr;

/* The following two functions are registered via thread_atfork() to
   make sure that the mutexes remain in a consistent state in the
   fork()ed version of a thread.  Also adapt the malloc and free hooks
   temporarily, because the `atfork' handler mechanism may use
   malloc/free internally (e.g. in LinuxThreads). */

static void
ptmalloc_lock_all (void)
{
  mstate ar_ptr;

  if(__malloc_initialized < 1)
    return;
  if (mutex_trylock(&list_lock))
    {
      void *my_arena;
      tsd_getspecific(arena_key, my_arena);
      if (my_arena == ATFORK_ARENA_PTR)
	/* This is the same thread which already locks the global list.
	   Just bump the counter.  */
	goto out;

      /* This thread has to wait its turn.  */
      (void)mutex_lock(&list_lock);
    }
  for(ar_ptr = &main_arena;;) {
    (void)mutex_lock(&ar_ptr->mutex);
    ar_ptr = ar_ptr->next;
    if(ar_ptr == &main_arena) break;
  }
  save_malloc_hook = __malloc_hook;
  save_free_hook = __free_hook;
  __malloc_hook = malloc_atfork;
  __free_hook = free_atfork;
  /* Only the current thread may perform malloc/free calls now. */
  tsd_getspecific(arena_key, save_arena);
  tsd_setspecific(arena_key, ATFORK_ARENA_PTR);
 out:
  ++atfork_recursive_cntr;
}

static void
ptmalloc_unlock_all (void)
{
  mstate ar_ptr;

  if(__malloc_initialized < 1)
    return;
  if (--atfork_recursive_cntr != 0)
    return;
  tsd_setspecific(arena_key, save_arena);
  __malloc_hook = save_malloc_hook;
  __free_hook = save_free_hook;
  for(ar_ptr = &main_arena;;) {
    (void)mutex_unlock(&ar_ptr->mutex);
    ar_ptr = ar_ptr->next;
    if(ar_ptr == &main_arena) break;
  }
  (void)mutex_unlock(&list_lock);
}

#ifdef __linux__

/* In NPTL, unlocking a mutex in the child process after a
   fork() is currently unsafe, whereas re-initializing it is safe and
   does not leak resources.  Therefore, a special atfork handler is
   installed for the child. */

static void
ptmalloc_unlock_all2 (void)
{
  mstate ar_ptr;

  if(__malloc_initialized < 1)
    return;
  tsd_setspecific(arena_key, save_arena);
  __malloc_hook = save_malloc_hook;
  __free_hook = save_free_hook;
#ifdef PER_THREAD
  free_list = NULL;
#endif
  for(ar_ptr = &main_arena;;) {
    mutex_init(&ar_ptr->mutex);
#ifdef PER_THREAD
    if (ar_ptr != save_arena) {
      ar_ptr->next_free = free_list;
      free_list = ar_ptr;
    }
#endif
    ar_ptr = ar_ptr->next;
    if(ar_ptr == &main_arena) break;
  }
  mutex_init(&list_lock);
  atfork_recursive_cntr = 0;
}

#else

#define ptmalloc_unlock_all2 ptmalloc_unlock_all

#endif

/* Initialization routine. */
#include <string.h>
extern char **_environ;

static char *
internal_function
next_env_entry (char ***position)
{
  char **current = *position;
  char *result = NULL;

  while (*current != NULL)
    {
      if (__builtin_expect ((*current)[0] == 'M', 0)
	  && (*current)[1] == 'A'
	  && (*current)[2] == 'L'
	  && (*current)[3] == 'L'
	  && (*current)[4] == 'O'
	  && (*current)[5] == 'C'
	  && (*current)[6] == '_')
	{
	  result = &(*current)[7];

	  /* Save current position for next visit.  */
	  *position = ++current;

	  break;
	}

      ++current;
    }

  return result;
}


#ifdef SHARED
static void *
__failing_morecore (ptrdiff_t d)
{
  return (void *) MORECORE_FAILURE;
}

extern struct dl_open_hook *_dl_open_hook;
libc_hidden_proto (_dl_open_hook);
#endif

static void
ptmalloc_init (void)
{
  if(__malloc_initialized >= 0) return;
  __malloc_initialized = 0;

#ifdef SHARED
  /* In case this libc copy is in a non-default namespace, never use brk.
     Likewise if dlopened from statically linked program.  */
  Dl_info di;
  struct link_map *l;

  if (_dl_open_hook != NULL
      || (_dl_addr (ptmalloc_init, &di, &l, NULL) != 0
	  && l->l_ns != LM_ID_BASE))
    __morecore = __failing_morecore;
#endif

  tsd_key_create(&arena_key, NULL);
  tsd_setspecific(arena_key, (void *)&main_arena);
  thread_atfork(ptmalloc_lock_all, ptmalloc_unlock_all, ptmalloc_unlock_all2);
  const char *s = NULL;
  if (__builtin_expect (_environ != NULL, 1))
    {
      char **runp = _environ;
      char *envline;

      while (__builtin_expect ((envline = next_env_entry (&runp)) != NULL,
			       0))
	{
	  size_t len = strcspn (envline, "=");

	  if (envline[len] != '=')
	    /* This is a "MALLOC_" variable at the end of the string
	       without a '=' character.  Ignore it since otherwise we
	       will access invalid memory below.  */
	    continue;

	  switch (len)
	    {
	    case 6:
	      if (memcmp (envline, "CHECK_", 6) == 0)
		s = &envline[7];
	      break;
	    case 8:
	      if (! __builtin_expect (__libc_enable_secure, 0))
		{
		  if (memcmp (envline, "TOP_PAD_", 8) == 0)
		    mALLOPt(M_TOP_PAD, atoi(&envline[9]));
		  else if (memcmp (envline, "PERTURB_", 8) == 0)
		    mALLOPt(M_PERTURB, atoi(&envline[9]));
		}
	      break;
	    case 9:
	      if (! __builtin_expect (__libc_enable_secure, 0))
		{
		  if (memcmp (envline, "MMAP_MAX_", 9) == 0)
		    mALLOPt(M_MMAP_MAX, atoi(&envline[10]));
#ifdef PER_THREAD
		  else if (memcmp (envline, "ARENA_MAX", 9) == 0)
		    mALLOPt(M_ARENA_MAX, atoi(&envline[10]));
#endif
		}
	      break;
#ifdef PER_THREAD
	    case 10:
	      if (! __builtin_expect (__libc_enable_secure, 0))
		{
		  if (memcmp (envline, "ARENA_TEST", 10) == 0)
		    mALLOPt(M_ARENA_TEST, atoi(&envline[11]));
		}
	      break;
#endif
	    case 15:
	      if (! __builtin_expect (__libc_enable_secure, 0))
		{
		  if (memcmp (envline, "TRIM_THRESHOLD_", 15) == 0)
		    mALLOPt(M_TRIM_THRESHOLD, atoi(&envline[16]));
		  else if (memcmp (envline, "MMAP_THRESHOLD_", 15) == 0)
		    mALLOPt(M_MMAP_THRESHOLD, atoi(&envline[16]));
		}
	      break;
	    default:
	      break;
	    }
	}
    }
  if(s && s[0]) {
    mALLOPt(M_CHECK_ACTION, (int)(s[0] - '0'));
    if (check_action != 0)
      __malloc_check_init();
  }
  void (*hook) (void) = force_reg (__malloc_initialize_hook);
  if (hook != NULL)
    (*hook)();
  __malloc_initialized = 1;
}

/* There are platforms (e.g. Hurd) with a link-time hook mechanism. */
#ifdef thread_atfork_static
thread_atfork_static(ptmalloc_lock_all, ptmalloc_unlock_all, \
		     ptmalloc_unlock_all2)
#endif



/* Managing heaps and arenas (for concurrent threads) */

#if MALLOC_DEBUG > 1

/* Print the complete contents of a single heap to stderr. */

static void
dump_heap(heap_info *heap)
{
  char *ptr;
  mchunkptr p;

  fprintf(stderr, "Heap %p, size %10lx:\n", heap, (long)heap->size);
  ptr = (heap->ar_ptr != (mstate)(heap+1)) ?
    (char*)(heap + 1) : (char*)(heap + 1) + sizeof(struct malloc_state);
  p = (mchunkptr)(((unsigned long)ptr + MALLOC_ALIGN_MASK) &
		  ~MALLOC_ALIGN_MASK);
  for(;;) {
    fprintf(stderr, "chunk %p size %10lx", p, (long)p->size);
    if(p == top(heap->ar_ptr)) {
      fprintf(stderr, " (top)\n");
      break;
    } else if(p->size == (0|PREV_INUSE)) {
      fprintf(stderr, " (fence)\n");
      break;
    }
    fprintf(stderr, "\n");
    p = next_chunk(p);
  }
}

#endif /* MALLOC_DEBUG > 1 */

/* If consecutive mmap (0, HEAP_MAX_SIZE << 1, ...) calls return decreasing
   addresses as opposed to increasing, new_heap would badly fragment the
   address space.  In that case remember the second HEAP_MAX_SIZE part
   aligned to HEAP_MAX_SIZE from last mmap (0, HEAP_MAX_SIZE << 1, ...)
   call (if it is already aligned) and try to reuse it next time.  We need
   no locking for it, as kernel ensures the atomicity for us - worst case
   we'll call mmap (addr, HEAP_MAX_SIZE, ...) for some value of addr in
   multiple threads, but only one will succeed.  */
static char *aligned_heap_area;

/* Create a new heap.  size is automatically rounded up to a multiple
   of the page size. */

static heap_info *
internal_function
new_heap(size_t size, size_t top_pad)
{
  size_t page_mask = GLRO(dl_pagesize) - 1;
  char *p1, *p2;
  unsigned long ul;
  heap_info *h;

  if(size+top_pad < HEAP_MIN_SIZE)
    size = HEAP_MIN_SIZE;
  else if(size+top_pad <= HEAP_MAX_SIZE)
    size += top_pad;
  else if(size > HEAP_MAX_SIZE)
    return 0;
  else
    size = HEAP_MAX_SIZE;
  size = (size + page_mask) & ~page_mask;

  /* A memory region aligned to a multiple of HEAP_MAX_SIZE is needed.
     No swap space needs to be reserved for the following large
     mapping (on Linux, this is the case for all non-writable mappings
     anyway). */
  p2 = MAP_FAILED;
  if(aligned_heap_area) {
    p2 = (char *)MMAP(aligned_heap_area, HEAP_MAX_SIZE, PROT_NONE,
		      MAP_PRIVATE|MAP_NORESERVE);
    aligned_heap_area = NULL;
    if (p2 != MAP_FAILED && ((unsigned long)p2 & (HEAP_MAX_SIZE-1))) {
      munmap(p2, HEAP_MAX_SIZE);
      p2 = MAP_FAILED;
    }
  }
  if(p2 == MAP_FAILED) {
    p1 = (char *)MMAP(0, HEAP_MAX_SIZE<<1, PROT_NONE,
		      MAP_PRIVATE|MAP_NORESERVE);
    if(p1 != MAP_FAILED) {
      p2 = (char *)(((unsigned long)p1 + (HEAP_MAX_SIZE-1))
		    & ~(HEAP_MAX_SIZE-1));
      ul = p2 - p1;
      if (ul)
	munmap(p1, ul);
      else
	aligned_heap_area = p2 + HEAP_MAX_SIZE;
      munmap(p2 + HEAP_MAX_SIZE, HEAP_MAX_SIZE - ul);
    } else {
      /* Try to take the chance that an allocation of only HEAP_MAX_SIZE
	 is already aligned. */
      p2 = (char *)MMAP(0, HEAP_MAX_SIZE, PROT_NONE, MAP_PRIVATE|MAP_NORESERVE);
      if(p2 == MAP_FAILED)
	return 0;
      if((unsigned long)p2 & (HEAP_MAX_SIZE-1)) {
	munmap(p2, HEAP_MAX_SIZE);
	return 0;
      }
    }
  }
  if(mprotect(p2, size, PROT_READ|PROT_WRITE) != 0) {
    munmap(p2, HEAP_MAX_SIZE);
    return 0;
  }
  h = (heap_info *)p2;
  h->size = size;
  h->mprotect_size = size;
  THREAD_STAT(stat_n_heaps++);
  return h;
}

/* Grow a heap.  size is automatically rounded up to a
   multiple of the page size. */

static int
grow_heap(heap_info *h, long diff)
{
  size_t page_mask = GLRO(dl_pagesize) - 1;
  long new_size;

  diff = (diff + page_mask) & ~page_mask;
  new_size = (long)h->size + diff;
  if((unsigned long) new_size > (unsigned long) HEAP_MAX_SIZE)
    return -1;
  if((unsigned long) new_size > h->mprotect_size) {
    if (mprotect((char *)h + h->mprotect_size,
		 (unsigned long) new_size - h->mprotect_size,
		 PROT_READ|PROT_WRITE) != 0)
      return -2;
    h->mprotect_size = new_size;
  }

  h->size = new_size;
  return 0;
}

/* Shrink a heap.  */

static int
shrink_heap(heap_info *h, long diff)
{
  long new_size;

  new_size = (long)h->size - diff;
  if(new_size < (long)sizeof(*h))
    return -1;
  /* Try to re-map the extra heap space freshly to save memory, and
     make it inaccessible. */
  if (__builtin_expect (__libc_enable_secure, 0))
    {
      if((char *)MMAP((char *)h + new_size, diff, PROT_NONE,
		      MAP_PRIVATE|MAP_FIXED) == (char *) MAP_FAILED)
	return -2;
      h->mprotect_size = new_size;
    }
  else
    madvise ((char *)h + new_size, diff, MADV_DONTNEED);
  /*fprintf(stderr, "shrink %p %08lx\n", h, new_size);*/

  h->size = new_size;
  return 0;
}

/* Delete a heap. */

#define delete_heap(heap) \
  do {								\
    if ((char *)(heap) + HEAP_MAX_SIZE == aligned_heap_area)	\
      aligned_heap_area = NULL;					\
    munmap((char*)(heap), HEAP_MAX_SIZE);			\
  } while (0)

static int
internal_function
heap_trim(heap_info *heap, size_t pad)
{
  mstate ar_ptr = heap->ar_ptr;
  unsigned long pagesz = GLRO(dl_pagesize);
  mchunkptr top_chunk = top(ar_ptr), p, bck, fwd;
  heap_info *prev_heap;
  long new_size, top_size, extra;

  /* Can this heap go away completely? */
  while(top_chunk == chunk_at_offset(heap, sizeof(*heap))) {
    prev_heap = heap->prev;
    p = chunk_at_offset(prev_heap, prev_heap->size - (MINSIZE-2*SIZE_SZ));
    assert(p->size == (0|PREV_INUSE)); /* must be fencepost */
    p = prev_chunk(p);
    new_size = chunksize(p) + (MINSIZE-2*SIZE_SZ);
    assert(new_size>0 && new_size<(long)(2*MINSIZE));
    if(!prev_inuse(p))
      new_size += p->prev_size;
    assert(new_size>0 && new_size<HEAP_MAX_SIZE);
    if(new_size + (HEAP_MAX_SIZE - prev_heap->size) < pad + MINSIZE + pagesz)
      break;
    ar_ptr->system_mem -= heap->size;
    arena_mem -= heap->size;
    delete_heap(heap);
    heap = prev_heap;
    if(!prev_inuse(p)) { /* consolidate backward */
      p = prev_chunk(p);
      unlink(p, bck, fwd);
    }
    assert(((unsigned long)((char*)p + new_size) & (pagesz-1)) == 0);
    assert( ((char*)p + new_size) == ((char*)heap + heap->size) );
    top(ar_ptr) = top_chunk = p;
    set_head(top_chunk, new_size | PREV_INUSE);
    /*check_chunk(ar_ptr, top_chunk);*/
  }
  top_size = chunksize(top_chunk);
  extra = (top_size - pad - MINSIZE - 1) & ~(pagesz - 1);
  if(extra < (long)pagesz)
    return 0;
  /* Try to shrink. */
  if(shrink_heap(heap, extra) != 0)
    return 0;
  ar_ptr->system_mem -= extra;
  arena_mem -= extra;

  /* Success. Adjust top accordingly. */
  set_head(top_chunk, (top_size - extra) | PREV_INUSE);
  /*check_chunk(ar_ptr, top_chunk);*/
  return 1;
}

/* Create a new arena with initial size "size".  */

static mstate
_int_new_arena(size_t size)
{
  mstate a;
  heap_info *h;
  char *ptr;
  unsigned long misalign;

  h = new_heap(size + (sizeof(*h) + sizeof(*a) + MALLOC_ALIGNMENT),
	       mp_.top_pad);
  if(!h) {
    /* Maybe size is too large to fit in a single heap.  So, just try
       to create a minimally-sized arena and let _int_malloc() attempt
       to deal with the large request via mmap_chunk().  */
    h = new_heap(sizeof(*h) + sizeof(*a) + MALLOC_ALIGNMENT, mp_.top_pad);
    if(!h)
      return 0;
  }
  a = h->ar_ptr = (mstate)(h+1);
  malloc_init_state(a);
  /*a->next = NULL;*/
  a->system_mem = a->max_system_mem = h->size;
  arena_mem += h->size;

  /* Set up the top chunk, with proper alignment. */
  ptr = (char *)(a + 1);
  misalign = (unsigned long)chunk2mem(ptr) & MALLOC_ALIGN_MASK;
  if (misalign > 0)
    ptr += MALLOC_ALIGNMENT - misalign;
  top(a) = (mchunkptr)ptr;
  set_head(top(a), (((char*)h + h->size) - ptr) | PREV_INUSE);

  tsd_setspecific(arena_key, (void *)a);
  mutex_init(&a->mutex);
  (void)mutex_lock(&a->mutex);

#ifdef PER_THREAD
  (void)mutex_lock(&list_lock);
#endif

  /* Add the new arena to the global list.  */
  a->next = main_arena.next;
  atomic_write_barrier ();
  main_arena.next = a;

#ifdef PER_THREAD
  (void)mutex_unlock(&list_lock);
#endif

  THREAD_STAT(++(a->stat_lock_loop));

  return a;
}


#ifdef PER_THREAD
static mstate
get_free_list (void)
{
  mstate result = free_list;
  if (result != NULL)
    {
      (void)mutex_lock(&list_lock);
      result = free_list;
      if (result != NULL)
	free_list = result->next_free;
      (void)mutex_unlock(&list_lock);

      if (result != NULL)
	{
	  (void)mutex_lock(&result->mutex);
	  tsd_setspecific(arena_key, (void *)result);
	  THREAD_STAT(++(result->stat_lock_loop));
	}
    }

  return result;
}


static mstate
reused_arena (void)
{
  mstate result;
  static mstate next_to_use;
  if (next_to_use == NULL)
    next_to_use = &main_arena;

  result = next_to_use;
  do
    {
      if (!mutex_trylock(&result->mutex))
	goto out;

      result = result->next;
    }
  while (result != next_to_use);

  /* No arena available.  Wait for the next in line.  */
  (void)mutex_lock(&result->mutex);

 out:
  tsd_setspecific(arena_key, (void *)result);
  THREAD_STAT(++(result->stat_lock_loop));
  next_to_use = result->next;

  return result;
}
#endif

static mstate
internal_function
arena_get2(mstate a_tsd, size_t size)
{
  mstate a;

#ifdef PER_THREAD
  static size_t narenas_limit;

  a = get_free_list ();
  if (a == NULL)
    {
      /* Nothing immediately available, so generate a new arena.  */
      if (narenas_limit == 0)
	{
	  if (mp_.arena_max != 0)
	    narenas_limit = mp_.arena_max;
	  else
	    {
	      int n  = __get_nprocs ();

	      if (n >= 1)
		narenas_limit = NARENAS_FROM_NCORES (n);
	      else
		/* We have no information about the system.  Assume two
		   cores.  */
		narenas_limit = NARENAS_FROM_NCORES (2);
	    }
	}
    repeat:;
      size_t n = narenas;
      if (__builtin_expect (n <= mp_.arena_test || n < narenas_limit, 0))
	{
	  if (catomic_compare_and_exchange_bool_acq(&narenas, n + 1, n))
	    goto repeat;
	  a = _int_new_arena (size);
	  if (__builtin_expect (a != NULL, 1))
	    return a;
	  catomic_decrement(&narenas);
	}
      a = reused_arena ();
    }
#else
  if(!a_tsd)
    a = a_tsd = &main_arena;
  else {
    a = a_tsd->next;
    if(!a) {
      /* This can only happen while initializing the new arena. */
      (void)mutex_lock(&main_arena.mutex);
      THREAD_STAT(++(main_arena.stat_lock_wait));
      return &main_arena;
    }
  }

  /* Check the global, circularly linked list for available arenas. */
  bool retried = false;
 repeat:
  do {
    if(!mutex_trylock(&a->mutex)) {
      if (retried)
	(void)mutex_unlock(&list_lock);
      THREAD_STAT(++(a->stat_lock_loop));
      tsd_setspecific(arena_key, (void *)a);
      return a;
    }
    a = a->next;
  } while(a != a_tsd);

  /* If not even the list_lock can be obtained, try again.  This can
     happen during `atfork', or for example on systems where thread
     creation makes it temporarily impossible to obtain _any_
     locks. */
  if(!retried && mutex_trylock(&list_lock)) {
    /* We will block to not run in a busy loop.  */
    (void)mutex_lock(&list_lock);

    /* Since we blocked there might be an arena available now.  */
    retried = true;
    a = a_tsd;
    goto repeat;
  }

  /* Nothing immediately available, so generate a new arena.  */
  a = _int_new_arena(size);
  (void)mutex_unlock(&list_lock);
#endif

  return a;
}

#ifdef PER_THREAD
static void __attribute__ ((section ("__libc_thread_freeres_fn")))
arena_thread_freeres (void)
{
  void *vptr = NULL;
  mstate a = tsd_getspecific(arena_key, vptr);
  tsd_setspecific(arena_key, NULL);

  if (a != NULL)
    {
      (void)mutex_lock(&list_lock);
      a->next_free = free_list;
      free_list = a;
      (void)mutex_unlock(&list_lock);
    }
}
text_set_element (__libc_thread_subfreeres, arena_thread_freeres);
#endif

/*
 * Local variables:
 * c-basic-offset: 2
 * End:
 */
