/* Copyright (C) 2002, 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2002.

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

#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/param.h>
#include <dl-sysdep.h>
#include <tls.h>




/* Most architectures have exactly one stack pointer.  Some have more.  */
#define STACK_VARIABLES void *stackaddr

/* How to pass the values to the 'create_thread' function.  */
#define STACK_VARIABLES_ARGS stackaddr

/* How to declare function which gets there parameters.  */
#define STACK_VARIABLES_PARMS void *stackaddr


/* Default alignment of stack.  */
#ifndef STACK_ALIGN
# define STACK_ALIGN __alignof__ (long double)
#endif

/* Default value for minimal stack size after allocating thread
   descriptor and guard.  */
#ifndef MINIMAL_REST_STACK
# define MINIMAL_REST_STACK	4096
#endif




/* Cache handling for not-yet free stacks.  */

/* Maximum size in kB of cache.  */
static size_t stack_cache_maxsize = 40 * 1024 * 1024; /* 40MiBi by default.  */
static size_t stack_cache_actsize;

/* Mutex protecting this variable.  */
static lll_lock_t stack_cache_lock = LLL_LOCK_INITIALIZER;

/* List of queued stack frames.  */
static LIST_HEAD (stack_cache);

/* List of the stacks in use.  */
static LIST_HEAD (stack_used);

/* List of the threads with user provided stacks in use.  No need to
   initialize this, since it's done in __pthread_initialize_minimal.  */
list_t __stack_user __attribute__ ((nocommon));
hidden_def (__stack_user)

/* Number of threads created.  */
static unsigned int nptl_ncreated;


/* Check whether the stack is still used or not.  */
#define FREE_P(descr) ((descr)->tid == 0)


/* We create a double linked list of all cache entries.  Double linked
   because this allows removing entries from the end.  */


/* Get a stack frame from the cache.  We have to match by size since
   some blocks might be too small or far too large.  */
static struct pthread *
get_cached_stack (size_t *sizep, void **memp)
{
  size_t size = *sizep;
  struct pthread *result = NULL;
  list_t *entry;

  lll_lock (stack_cache_lock);

  /* Search the cache for a matching entry.  We search for the
     smallest stack which has at least the required size.  Note that
     in normal situations the size of all allocated stacks is the
     same.  As the very least there are only a few different sizes.
     Therefore this loop will exit early most of the time with an
     exact match.  */
  list_for_each (entry, &stack_cache)
    {
      struct pthread *curr;

      curr = list_entry (entry, struct pthread, header.data.list);
      if (FREE_P (curr) && curr->stackblock_size >= size)
	{
	  if (curr->stackblock_size == size)
	    {
	      result = curr;
	      break;
	    }

	  if (result == NULL
	      || result->stackblock_size > curr->stackblock_size)
	    result = curr;
	}
    }

  if (__builtin_expect (result == NULL, 0)
      /* Make sure the size difference is not too excessive.  In that
	 case we do not use the block.  */
      || __builtin_expect (result->stackblock_size > 4 * size, 0))
    {
      /* Release the lock.  */
      lll_unlock (stack_cache_lock);

      return NULL;
    }

  /* Dequeue the entry.  */
  list_del (&result->header.data.list);

  /* And add to the list of stacks in use.  */
  list_add (&result->header.data.list, &stack_used);

  /* And decrease the cache size.  */
  stack_cache_actsize -= result->stackblock_size;

  /* Release the lock early.  */
  lll_unlock (stack_cache_lock);


  *sizep = result->stackblock_size;
  *memp = result->stackblock;

  /* Cancellation handling is back to the default.  */
  result->cancelhandling = 0;
  result->cleanup = NULL;

  /* No pending event.  */
  result->nextevent = NULL;

  /* Clear the DTV.  */
  dtv_t *dtv = GET_DTV (result);
  memset (dtv, '\0', (dtv[-1].counter + 1) * sizeof (dtv_t));

  /* Re-initialize the TLS.  */
  return _dl_allocate_tls_init (result);
}


/* Add a stack frame which is not used anymore to the stack.  Must be
   called with the cache lock held.  */
static inline void
queue_stack (struct pthread *stack)
{
  /* We unconditionally add the stack to the list.  The memory may
     still be in use but it will not be reused until the kernel marks
     the stack as not used anymore.  */
  list_add (&stack->header.data.list, &stack_cache);

  stack_cache_actsize += stack->stackblock_size;
  if (__builtin_expect (stack_cache_actsize > stack_cache_maxsize, 0))
    {
      /* We reduce the size of the cache.  Remove the last entries
	 until the size is below the limit.  */
      list_t *entry;
      list_t *prev;

      /* Search from the end of the list.  */
      list_for_each_prev_safe (entry, prev, &stack_cache)
	{
	  struct pthread *curr;

	  curr = list_entry (entry, struct pthread, header.data.list);
	  if (FREE_P (curr))
	    {
	      /* Unlink the block.  */
	      list_del (entry);

	      /* Account for the freed memory.  */
	      stack_cache_actsize -= curr->stackblock_size;

	      /* Free the memory associated with the ELF TLS.  */
	      _dl_deallocate_tls (curr, false);

	      /* Remove this block.  This should never fail.  If it
		 does something is really wrong.  */
	      if (munmap (curr->stackblock, curr->stackblock_size) != 0)
		abort ();

	      /* Maybe we have freed enough.  */
	      if (stack_cache_actsize <= stack_cache_maxsize)
		break;
	    }
	}
    }
}



static int
allocate_stack (const struct pthread_attr *attr, struct pthread **pdp,
		void **stack)
{
  struct pthread *pd;
  size_t size;
  size_t pagesize_m1 = __getpagesize () - 1;

  assert (attr != NULL);
  assert (powerof2 (pagesize_m1 + 1));
  assert (TCB_ALIGNMENT >= STACK_ALIGN);

  /* Get the stack size from the attribute if it is set.  Otherwise we
     use the default we determined at start time.  */
  size = attr->stacksize ?: __default_stacksize;

  /* Get memory for the stack.  */
  if (__builtin_expect (attr->flags & ATTR_FLAG_STACKADDR, 0))
    {
      uintptr_t adj;

      /* If the user also specified the size of the stack make sure it
	 is large enough.  */
      if (attr->stacksize != 0
	  && attr->stacksize < (__static_tls_size + MINIMAL_REST_STACK))
	return EINVAL;

      /* Adjust stack size for alignment of the TLS block.  */
      adj = ((uintptr_t) attr->stackaddr) & (__static_tls_align - 1);
      assert (size > adj);

      /* The user provided some memory.  Let's hope it matches the
	 size...  We do not allocate guard pages if the user provided
	 the stack.  It is the user's responsibility to do this if it
	 is wanted.  */
      pd = (struct pthread *) (((uintptr_t) attr->stackaddr - adj)
			       & ~(__alignof (struct pthread) - 1)) - 1;

      /* The user provided stack memory needs to be cleared.  */
      memset (pd, '\0', sizeof (struct pthread));

      /* The first TSD block is included in the TCB.  */
      pd->specific[0] = pd->specific_1stblock;

#if LLL_LOCK_INITIALIZER != 0
      /* Initialize the lock.  */
      pd->lock = LLL_LOCK_INITIALIZER;
#endif

      /* Remember the stack-related values.  */
      pd->stackblock = (char *) attr->stackaddr - size;
      pd->stackblock_size = size - adj;

      /* This is a user-provided stack.  It will not be queued in the
	 stack cache nor will the memory (except the TLS memory) be freed.  */
      pd->user_stack = true;

      /* This is at least the second thread.  */
      pd->header.data.multiple_threads = 1;

#ifdef NEED_DL_SYSINFO
      /* Copy the sysinfo value from the parent.  */
      pd->header.data.sysinfo
	= THREAD_GETMEM (THREAD_SELF, header.data.sysinfo);
#endif

      /* Allocate the DTV for this thread.  */
      if (_dl_allocate_tls (pd) == NULL)
	/* Something went wrong.  */
	return errno;


      /* Prepare to modify global data.  */
      lll_lock (stack_cache_lock);

      /* And add to the list of stacks in use.  */
      list_add (&pd->header.data.list, &__stack_user);

      lll_unlock (stack_cache_lock);
    }
  else
    {
      /* Allocate some anonymous memory.  If possible use the
	 cache.  */
      size_t guardsize;
      size_t reqsize;
      void *mem;

#if COLORING_INCREMENT != 0
      /* Add one more page for stack coloring.  Don't to it for stacks
	 with 16 times pagesize or larger.  This might just cause
	 unnecessary misalignment.  */
      if (size <= 16 * pagesize_m1)
	size += pagesize_m1 + 1;
#endif

      /* Adjust the stack size for alignment.  */
      size &= ~(__static_tls_align - 1);
      assert (size != 0);

      /* Make sure the size of the stack is enough for the guard and
	 eventually the thread descriptor.  */
      guardsize = (attr->guardsize + pagesize_m1) & ~pagesize_m1;
      if (__builtin_expect (size < (guardsize + __static_tls_size
				    + MINIMAL_REST_STACK + pagesize_m1 + 1),
			    0))
	/* The stack is too small (or the guard too large).  */
	return EINVAL;

      /* Try to get a stack from the cache.  */
      reqsize = size;
      pd = get_cached_stack (&size, &mem);
      if (pd == NULL)
	{
	  mem = mmap (NULL, size, PROT_READ | PROT_WRITE | PROT_EXEC,
		      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	  if (__builtin_expect (mem == MAP_FAILED, 0))
	    return errno;

	  /* 'size' is guaranteed to be greater than zero.  So we can
	     never get a NULL pointer back from MMAP.  */
	  assert (mem != NULL);

#if COLORING_INCREMENT != 0
	  /* Atomically increment NCREATED.  */
	  unsigned int ncreated = atomic_exchange_and_add (&nptl_ncreated, 1);

	  /* We chose the offset for coloring by incrementing it for
	     every new thread by a fixed amount.  The offset used
	     module the page size.  Even if coloring would be better
	     relative to higher alignment values it makes no sense to
	     do it since the mmap() interface does not allow us to
	     specify any alignment for the returned memory block.  */
	  size_t coloring = (ncreated * COLORING_INCREMENT) & pagesize_m1;

	  /* Make sure the coloring offsets does not disturb the alignment
	     of the TCB and static TLS block.  */
	  if (__builtin_expect ((coloring & (__static_tls_align - 1)) != 0, 0))
	    coloring = (((coloring + __static_tls_align - 1)
			 & ~(__static_tls_align - 1))
			& ~pagesize_m1);
#else
	  /* Unless specified we do not make any adjustments.  */
# define coloring 0
#endif

	  /* Place the thread descriptor at the end of the stack.  */
	  pd = (struct pthread *) ((char *) mem + size - coloring) - 1;

	  /* Remember the stack-related values.  */
	  pd->stackblock = mem;
	  pd->stackblock_size = size;

	  /* We allocated the first block thread-specific data array.
	     This address will not change for the lifetime of this
	     descriptor.  */
	  pd->specific[0] = pd->specific_1stblock;

#if LLL_LOCK_INITIALIZER != 0
	  /* Initialize the lock.  */
	  pd->lock = LLL_LOCK_INITIALIZER;
#endif

	  /* This is at least the second thread.  */
	  pd->header.data.multiple_threads = 1;

#ifdef NEED_DL_SYSINFO
	  /* Copy the sysinfo value from the parent.  */
	  pd->header.data.sysinfo
	    = THREAD_GETMEM (THREAD_SELF, header.data.sysinfo);
#endif

	  /* Allocate the DTV for this thread.  */
	  if (_dl_allocate_tls (pd) == NULL)
	    {
	      /* Something went wrong.  */
	      int err = errno;

	      /* Free the stack memory we just allocated.  */
	      (void) munmap (mem, size);

	      return err;
	    }


	  /* Prepare to modify global data.  */
	  lll_lock (stack_cache_lock);

	  /* And add to the list of stacks in use.  */
	  list_add (&pd->header.data.list, &stack_used);

	  lll_unlock (stack_cache_lock);


	  /* Note that all of the stack and the thread descriptor is
	     zeroed.  This means we do not have to initialize fields
	     with initial value zero.  This is specifically true for
	     the 'tid' field which is always set back to zero once the
	     stack is not used anymore and for the 'guardsize' field
	     which will be read next.  */
	}

      /* Create or resize the guard area if necessary.  */
      if (__builtin_expect (guardsize > pd->guardsize, 0))
	{
	  if (mprotect (mem, guardsize, PROT_NONE) != 0)
	    {
	      int err;
	    mprot_error:
	      err = errno;

	      lll_lock (stack_cache_lock);

	      /* Remove the thread from the list.  */
	      list_del (&pd->header.data.list);

	      lll_unlock (stack_cache_lock);

	      /* Free the memory regardless of whether the size of the
		 cache is over the limit or not.  If this piece of
		 memory caused problems we better do not use it
		 anymore.  Uh, and we ignore possible errors.  There
		 is nothing we could do.  */
	      (void) munmap (mem, size);

	      return err;
	    }

	  pd->guardsize = guardsize;
	}
      else if (__builtin_expect (pd->guardsize - guardsize > size - reqsize,
				 0))
	{
	  /* The old guard area is too large.  */
	  if (mprotect ((char *) mem + guardsize, pd->guardsize - guardsize,
			PROT_READ | PROT_WRITE | PROT_EXEC) != 0)
	    goto mprot_error;

	  pd->guardsize = guardsize;
	}
    }

  /* We place the thread descriptor at the end of the stack.  */
  *pdp = pd;

#if TLS_TCB_AT_TP
  /* The stack begin before the TCB and the static TLS block.  */
  *stack = ((char *) (pd + 1) - __static_tls_size);
#else
# error "Implement me"
#endif

  return 0;
}

/* This is how the function is called.  We do it this way to allow
   other variants of the function to have more parameters.  */
#define ALLOCATE_STACK(attr, pd) allocate_stack (attr, pd, &stackaddr)


void
internal_function
__deallocate_stack (struct pthread *pd)
{
  lll_lock (stack_cache_lock);

  /* Remove the thread from the list of threads with user defined
     stacks.  */
  list_del (&pd->header.data.list);

  /* Not much to do.  Just free the mmap()ed memory.  Note that we do
     not reset the 'used' flag in the 'tid' field.  This is done by
     the kernel.  If no thread has been created yet this field is
     still zero.  */
  if (__builtin_expect (! pd->user_stack, 1))
    (void) queue_stack (pd);
  else
    /* Free the memory associated with the ELF TLS.  */
    _dl_deallocate_tls (pd, false);

  lll_unlock (stack_cache_lock);
}


/* In case of a fork() call the memory allocation in the child will be
   the same but only one thread is running.  All stacks except that of
   the one running thread are not used anymore.  We have to recycle
   them.  */
void
__reclaim_stacks (void)
{
  struct pthread *self = (struct pthread *) THREAD_SELF;

  /* No locking necessary.  The caller is the only stack in use.  */

  /* Mark all stacks except the still running one as free.  */
  list_t *runp;
  list_for_each (runp, &stack_used)
    {
      struct pthread *curp;

      curp = list_entry (runp, struct pthread, header.data.list);
      if (curp != self)
	{
	  /* This marks the stack as free.  */
	  curp->tid = 0;

	  /* Account for the size of the stack.  */
	  stack_cache_actsize += curp->stackblock_size;
	}
    }

  /* Add the stack of all running threads to the cache.  */
  list_splice (&stack_used, &stack_cache);

  /* Remove the entry for the current thread to from the cache list
     and add it to the list of running threads.  Which of the two
     lists is decided by the user_stack flag.  */
  list_del (&self->header.data.list);

  /* Re-initialize the lists for all the threads.  */
  INIT_LIST_HEAD (&stack_used);
  INIT_LIST_HEAD (&__stack_user);

  if (__builtin_expect (THREAD_GETMEM (self, user_stack), 0))
    list_add (&self->header.data.list, &__stack_user);
  else
    list_add (&self->header.data.list, &stack_used);

  /* There is one thread running.  */
  __nptl_nthreads = 1;

  /* Initialize the lock.  */
  stack_cache_lock = LLL_LOCK_INITIALIZER;
}
