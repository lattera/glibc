/* Copyright (C) 2002, 2003, 2004 Free Software Foundation, Inc.
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
#include <signal.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/param.h>
#include <dl-sysdep.h>
#include <tls.h>
#include <lowlevellock.h>


#ifndef NEED_SEPARATE_REGISTER_STACK

/* Most architectures have exactly one stack pointer.  Some have more.  */
# define STACK_VARIABLES void *stackaddr

/* How to pass the values to the 'create_thread' function.  */
# define STACK_VARIABLES_ARGS stackaddr

/* How to declare function which gets there parameters.  */
# define STACK_VARIABLES_PARMS void *stackaddr

/* How to declare allocate_stack.  */
# define ALLOCATE_STACK_PARMS void **stack

/* This is how the function is called.  We do it this way to allow
   other variants of the function to have more parameters.  */
# define ALLOCATE_STACK(attr, pd) allocate_stack (attr, pd, &stackaddr)

#else

/* We need two stacks.  The kernel will place them but we have to tell
   the kernel about the size of the reserved address space.  */
# define STACK_VARIABLES void *stackaddr; size_t stacksize

/* How to pass the values to the 'create_thread' function.  */
# define STACK_VARIABLES_ARGS stackaddr, stacksize

/* How to declare function which gets there parameters.  */
# define STACK_VARIABLES_PARMS void *stackaddr, size_t stacksize

/* How to declare allocate_stack.  */
# define ALLOCATE_STACK_PARMS void **stack, size_t *stacksize

/* This is how the function is called.  We do it this way to allow
   other variants of the function to have more parameters.  */
# define ALLOCATE_STACK(attr, pd) \
  allocate_stack (attr, pd, &stackaddr, &stacksize)

#endif


/* Default alignment of stack.  */
#ifndef STACK_ALIGN
# define STACK_ALIGN __alignof__ (long double)
#endif

/* Default value for minimal stack size after allocating thread
   descriptor and guard.  */
#ifndef MINIMAL_REST_STACK
# define MINIMAL_REST_STACK	4096
#endif


/* Let the architecture add some flags to the mmap() call used to
   allocate stacks.  */
#ifndef ARCH_MAP_FLAGS
# define ARCH_MAP_FLAGS 0
#endif

/* This yields the pointer that TLS support code calls the thread pointer.  */
#if TLS_TCB_AT_TP
# define TLS_TPADJ(pd) (pd)
#elif TLS_DTV_AT_TP
# define TLS_TPADJ(pd) ((struct pthread *)((char *) (pd) + TLS_PRE_TCB_SIZE))
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
hidden_data_def (__stack_user)

#if COLORING_INCREMENT != 0
/* Number of threads created.  */
static unsigned int nptl_ncreated;
#endif


/* Check whether the stack is still used or not.  */
#define FREE_P(descr) ((descr)->tid <= 0)


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

      curr = list_entry (entry, struct pthread, list);
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
  list_del (&result->list);

  /* And add to the list of stacks in use.  */
  list_add (&result->list, &stack_used);

  /* And decrease the cache size.  */
  stack_cache_actsize -= result->stackblock_size;

  /* Release the lock early.  */
  lll_unlock (stack_cache_lock);

  /* Report size and location of the stack to the caller.  */
  *sizep = result->stackblock_size;
  *memp = result->stackblock;

  /* Cancellation handling is back to the default.  */
  result->cancelhandling = 0;
  result->cleanup = NULL;

  /* No pending event.  */
  result->nextevent = NULL;

  /* Clear the DTV.  */
  dtv_t *dtv = GET_DTV (TLS_TPADJ (result));
  memset (dtv, '\0', (dtv[-1].counter + 1) * sizeof (dtv_t));

  /* Re-initialize the TLS.  */
  _dl_allocate_tls_init (TLS_TPADJ (result));

  return result;
}


/* Add a stack frame which is not used anymore to the stack.  Must be
   called with the cache lock held.  */
static inline void
__attribute ((always_inline))
queue_stack (struct pthread *stack)
{
  /* We unconditionally add the stack to the list.  The memory may
     still be in use but it will not be reused until the kernel marks
     the stack as not used anymore.  */
  list_add (&stack->list, &stack_cache);

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

	  curr = list_entry (entry, struct pthread, list);
	  if (FREE_P (curr))
	    {
	      /* Unlink the block.  */
	      list_del (entry);

	      /* Account for the freed memory.  */
	      stack_cache_actsize -= curr->stackblock_size;

	      /* Free the memory associated with the ELF TLS.  */
	      _dl_deallocate_tls (TLS_TPADJ (curr), false);

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
internal_function
change_stack_perm (struct pthread *pd
#ifdef NEED_SEPARATE_REGISTER_STACK
		   , size_t pagemask
#endif
		   )
{
#ifdef NEED_SEPARATE_REGISTER_STACK
  void *stack = (pd->stackblock
		 + (((((pd->stackblock_size - pd->guardsize) / 2)
		      & pagemask) + pd->guardsize) & pagemask));
  size_t len = pd->stackblock + pd->stackblock_size - stack;
#else
  void *stack = pd->stackblock + pd->guardsize;
  size_t len = pd->stackblock_size - pd->guardsize;
#endif
  if (mprotect (stack, len, PROT_READ | PROT_WRITE | PROT_EXEC) != 0)
    return errno;

  return 0;
}


static int
allocate_stack (const struct pthread_attr *attr, struct pthread **pdp,
		ALLOCATE_STACK_PARMS)
{
  struct pthread *pd;
  size_t size;
  size_t pagesize_m1 = __getpagesize () - 1;
  void *stacktop;

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
#if TLS_TCB_AT_TP
      adj = ((uintptr_t) attr->stackaddr - TLS_TCB_SIZE)
	    & __static_tls_align_m1;
      assert (size > adj + TLS_TCB_SIZE);
#elif TLS_DTV_AT_TP
      adj = ((uintptr_t) attr->stackaddr - __static_tls_size)
	    & __static_tls_align_m1;
      assert (size > adj);
#endif

      /* The user provided some memory.  Let's hope it matches the
	 size...  We do not allocate guard pages if the user provided
	 the stack.  It is the user's responsibility to do this if it
	 is wanted.  */
#if TLS_TCB_AT_TP
      pd = (struct pthread *) ((uintptr_t) attr->stackaddr
			       - TLS_TCB_SIZE - adj);
#elif TLS_DTV_AT_TP
      pd = (struct pthread *) (((uintptr_t) attr->stackaddr
			        - __static_tls_size - adj)
			       - TLS_PRE_TCB_SIZE);
#endif

      /* The user provided stack memory needs to be cleared.  */
      memset (pd, '\0', sizeof (struct pthread));

      /* The first TSD block is included in the TCB.  */
      pd->specific[0] = pd->specific_1stblock;

      /* Remember the stack-related values.  */
      pd->stackblock = (char *) attr->stackaddr - size;
      pd->stackblock_size = size;

      /* This is a user-provided stack.  It will not be queued in the
	 stack cache nor will the memory (except the TLS memory) be freed.  */
      pd->user_stack = true;

      /* This is at least the second thread.  */
      pd->header.multiple_threads = 1;
#ifndef TLS_MULTIPLE_THREADS_IN_TCB
      __pthread_multiple_threads = *__libc_multiple_threads_ptr = 1;
#endif

#ifdef NEED_DL_SYSINFO
      /* Copy the sysinfo value from the parent.  */
      THREAD_SYSINFO(pd) = THREAD_SELF_SYSINFO;
#endif

      /* The process ID is also the same as that of the caller.  */
      pd->pid = THREAD_GETMEM (THREAD_SELF, pid);

      /* Allocate the DTV for this thread.  */
      if (_dl_allocate_tls (TLS_TPADJ (pd)) == NULL)
	{
	  /* Something went wrong.  */
	  assert (errno == ENOMEM);
	  return EAGAIN;
	}


      /* Prepare to modify global data.  */
      lll_lock (stack_cache_lock);

      /* And add to the list of stacks in use.  */
      list_add (&pd->list, &__stack_user);

      lll_unlock (stack_cache_lock);
    }
  else
    {
      /* Allocate some anonymous memory.  If possible use the cache.  */
      size_t guardsize;
      size_t reqsize;
      void *mem;
      const int prot = (PROT_READ | PROT_WRITE
			| ((GL(dl_stack_flags) & PF_X) ? PROT_EXEC : 0));

#if COLORING_INCREMENT != 0
      /* Add one more page for stack coloring.  Don't do it for stacks
	 with 16 times pagesize or larger.  This might just cause
	 unnecessary misalignment.  */
      if (size <= 16 * pagesize_m1)
	size += pagesize_m1 + 1;
#endif

      /* Adjust the stack size for alignment.  */
      size &= ~__static_tls_align_m1;
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
	  /* To avoid aliasing effects on a larger scale than pages we
	     adjust the allocated stack size if necessary.  This way
	     allocations directly following each other will not have
	     aliasing problems.  */
#if MULTI_PAGE_ALIASING != 0
	  if ((size % MULTI_PAGE_ALIASING) == 0)
	    size += pagesize_m1 + 1;
#endif

	  mem = mmap (NULL, size, prot,
		      MAP_PRIVATE | MAP_ANONYMOUS | ARCH_MAP_FLAGS, -1, 0);

	  if (__builtin_expect (mem == MAP_FAILED, 0))
	    {
#ifdef ARCH_RETRY_MMAP
	      mem = ARCH_RETRY_MMAP (size);
	      if (__builtin_expect (mem == MAP_FAILED, 0))
#endif
		return errno;
	    }

	  /* SIZE is guaranteed to be greater than zero.
	     So we can never get a null pointer back from mmap.  */
	  assert (mem != NULL);

#if COLORING_INCREMENT != 0
	  /* Atomically increment NCREATED.  */
	  unsigned int ncreated = atomic_increment_val (&nptl_ncreated);

	  /* We chose the offset for coloring by incrementing it for
	     every new thread by a fixed amount.  The offset used
	     module the page size.  Even if coloring would be better
	     relative to higher alignment values it makes no sense to
	     do it since the mmap() interface does not allow us to
	     specify any alignment for the returned memory block.  */
	  size_t coloring = (ncreated * COLORING_INCREMENT) & pagesize_m1;

	  /* Make sure the coloring offsets does not disturb the alignment
	     of the TCB and static TLS block.  */
	  if (__builtin_expect ((coloring & __static_tls_align_m1) != 0, 0))
	    coloring = (((coloring + __static_tls_align_m1)
			 & ~(__static_tls_align_m1))
			& ~pagesize_m1);
#else
	  /* Unless specified we do not make any adjustments.  */
# define coloring 0
#endif

	  /* Place the thread descriptor at the end of the stack.  */
#if TLS_TCB_AT_TP
	  pd = (struct pthread *) ((char *) mem + size - coloring) - 1;
#elif TLS_DTV_AT_TP
	  pd = (struct pthread *) ((((uintptr_t) mem + size - coloring
				    - __static_tls_size)
				    & ~__static_tls_align_m1)
				   - TLS_PRE_TCB_SIZE);
#endif

	  /* Remember the stack-related values.  */
	  pd->stackblock = mem;
	  pd->stackblock_size = size;

	  /* We allocated the first block thread-specific data array.
	     This address will not change for the lifetime of this
	     descriptor.  */
	  pd->specific[0] = pd->specific_1stblock;

	  /* This is at least the second thread.  */
	  pd->header.multiple_threads = 1;
#ifndef TLS_MULTIPLE_THREADS_IN_TCB
	  __pthread_multiple_threads = *__libc_multiple_threads_ptr = 1;
#endif

#ifdef NEED_DL_SYSINFO
	  /* Copy the sysinfo value from the parent.  */
	  THREAD_SYSINFO(pd) = THREAD_SELF_SYSINFO;
#endif

	  /* The process ID is also the same as that of the caller.  */
	  pd->pid = THREAD_GETMEM (THREAD_SELF, pid);

	  /* Allocate the DTV for this thread.  */
	  if (_dl_allocate_tls (TLS_TPADJ (pd)) == NULL)
	    {
	      /* Something went wrong.  */
	      assert (errno == ENOMEM);

	      /* Free the stack memory we just allocated.  */
	      (void) munmap (mem, size);

	      return EAGAIN;
	    }


	  /* Prepare to modify global data.  */
	  lll_lock (stack_cache_lock);

	  /* And add to the list of stacks in use.  */
	  list_add (&pd->list, &stack_used);

	  lll_unlock (stack_cache_lock);


	  /* There might have been a race.  Another thread might have
	     caused the stacks to get exec permission while this new
	     stack was prepared.  Detect if this was possible and
	     change the permission if necessary.  */
	  if (__builtin_expect ((GL(dl_stack_flags) & PF_X) != 0
				&& (prot & PROT_EXEC) == 0, 0))
	    {
	      int err = change_stack_perm (pd
#ifdef NEED_SEPARATE_REGISTER_STACK
					   , ~pagesize_m1
#endif
					   );
	      if (err != 0)
		{
		  /* Free the stack memory we just allocated.  */
		  (void) munmap (mem, size);

		  return err;
		}
	    }


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
#ifdef NEED_SEPARATE_REGISTER_STACK
	  char *guard = mem + (((size - guardsize) / 2) & ~pagesize_m1);
#else
	  char *guard = mem;
#endif
	  if (mprotect (guard, guardsize, PROT_NONE) != 0)
	    {
	      int err;
	    mprot_error:
	      err = errno;

	      lll_lock (stack_cache_lock);

	      /* Remove the thread from the list.  */
	      list_del (&pd->list);

	      lll_unlock (stack_cache_lock);

	      /* Get rid of the TLS block we allocated.  */
	      _dl_deallocate_tls (TLS_TPADJ (pd), false);

	      /* Free the stack memory regardless of whether the size
		 of the cache is over the limit or not.  If this piece
		 of memory caused problems we better do not use it
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

#ifdef NEED_SEPARATE_REGISTER_STACK
	  char *guard = mem + (((size - guardsize) / 2) & ~pagesize_m1);
	  char *oldguard = mem + (((size - pd->guardsize) / 2) & ~pagesize_m1);

	  if (oldguard < guard
	      && mprotect (oldguard, guard - oldguard, prot) != 0)
	    goto mprot_error;

	  if (mprotect (guard + guardsize,
			oldguard + pd->guardsize - guard - guardsize,
			prot) != 0)
	    goto mprot_error;
#else
	  if (mprotect ((char *) mem + guardsize, pd->guardsize - guardsize,
			prot) != 0)
	    goto mprot_error;
#endif

	  pd->guardsize = guardsize;
	}
      /* The pthread_getattr_np() calls need to get passed the size
	 requested in the attribute, regardless of how large the
	 actually used guardsize is.  */
      pd->reported_guardsize = guardsize;
    }

  /* Initialize the lock.  We have to do this unconditionally since the
     stillborn thread could be canceled while the lock is taken.  */
  pd->lock = LLL_LOCK_INITIALIZER;

  /* We place the thread descriptor at the end of the stack.  */
  *pdp = pd;

#if TLS_TCB_AT_TP
  /* The stack begins before the TCB and the static TLS block.  */
  stacktop = ((char *) (pd + 1) - __static_tls_size);
#elif TLS_DTV_AT_TP
  stacktop = (char *) (pd - 1);
#endif

#ifdef NEED_SEPARATE_REGISTER_STACK
  *stack = pd->stackblock;
  *stacksize = stacktop - *stack;
#else
  *stack = stacktop;
#endif

  return 0;
}


void
internal_function
__deallocate_stack (struct pthread *pd)
{
  lll_lock (stack_cache_lock);

  /* Remove the thread from the list of threads with user defined
     stacks.  */
  list_del (&pd->list);

  /* Not much to do.  Just free the mmap()ed memory.  Note that we do
     not reset the 'used' flag in the 'tid' field.  This is done by
     the kernel.  If no thread has been created yet this field is
     still zero.  */
  if (__builtin_expect (! pd->user_stack, 1))
    (void) queue_stack (pd);
  else
    /* Free the memory associated with the ELF TLS.  */
    _dl_deallocate_tls (TLS_TPADJ (pd), false);

  lll_unlock (stack_cache_lock);
}


int
internal_function
__make_stacks_executable (void **stack_endp)
{
  /* First the main thread's stack.  */
  int err = _dl_make_stack_executable (stack_endp);
  if (err != 0)
    return err;

#ifdef NEED_SEPARATE_REGISTER_STACK
  const size_t pagemask = ~(__getpagesize () - 1);
#endif

  lll_lock (stack_cache_lock);

  list_t *runp;
  list_for_each (runp, &stack_used)
    {
      err = change_stack_perm (list_entry (runp, struct pthread, list)
#ifdef NEED_SEPARATE_REGISTER_STACK
			       , pagemask
#endif
			       );
      if (err != 0)
	break;
    }

  /* Also change the permission for the currently unused stacks.  This
     might be wasted time but better spend it here than adding a check
     in the fast path.  */
  if (err == 0)
    list_for_each (runp, &stack_cache)
      {
	err = change_stack_perm (list_entry (runp, struct pthread, list)
#ifdef NEED_SEPARATE_REGISTER_STACK
				 , pagemask
#endif
				 );
	if (err != 0)
	  break;
      }

  lll_unlock (stack_cache_lock);

  return err;
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

      curp = list_entry (runp, struct pthread, list);
      if (curp != self)
	{
	  /* This marks the stack as free.  */
	  curp->tid = 0;

	  /* The PID field must be initialized for the new process.  */
	  curp->pid = self->pid;

	  /* Account for the size of the stack.  */
	  stack_cache_actsize += curp->stackblock_size;
	}
    }

  /* Add the stack of all running threads to the cache.  */
  list_splice (&stack_used, &stack_cache);

  /* Remove the entry for the current thread to from the cache list
     and add it to the list of running threads.  Which of the two
     lists is decided by the user_stack flag.  */
  list_del (&self->list);

  /* Re-initialize the lists for all the threads.  */
  INIT_LIST_HEAD (&stack_used);
  INIT_LIST_HEAD (&__stack_user);

  if (__builtin_expect (THREAD_GETMEM (self, user_stack), 0))
    list_add (&self->list, &__stack_user);
  else
    list_add (&self->list, &stack_used);

  /* There is one thread running.  */
  __nptl_nthreads = 1;

  /* Initialize the lock.  */
  stack_cache_lock = LLL_LOCK_INITIALIZER;
}


#if HP_TIMING_AVAIL
# undef __find_thread_by_id
/* Find a thread given the thread ID.  */
attribute_hidden
struct pthread *
__find_thread_by_id (pid_t tid)
{
  struct pthread *result = NULL;

  lll_lock (stack_cache_lock);

  /* Iterate over the list with system-allocated threads first.  */
  list_t *runp;
  list_for_each (runp, &stack_used)
    {
      struct pthread *curp;

      curp = list_entry (runp, struct pthread, list);

      if (curp->tid == tid)
	{
	  result = curp;
	  goto out;
	}
    }

  /* Now the list with threads using user-allocated stacks.  */
  list_for_each (runp, &__stack_user)
    {
      struct pthread *curp;

      curp = list_entry (runp, struct pthread, list);

      if (curp->tid == tid)
	{
	  result = curp;
	  goto out;
	}
    }

 out:
  lll_unlock (stack_cache_lock);

  return result;
}
#endif

int
attribute_hidden
__nptl_setxid (struct xid_command *cmdp)
{
  int result;
  lll_lock (stack_cache_lock);

  __xidcmd = cmdp;
  cmdp->cntr = 0;

  INTERNAL_SYSCALL_DECL (err);

  struct pthread *self = THREAD_SELF;

  /* Iterate over the list with system-allocated threads first.  */
  list_t *runp;
  list_for_each (runp, &stack_used)
    {
      struct pthread *t = list_entry (runp, struct pthread, list);
      if (t != self)
	{
	  int val;
#if __ASSUME_TGKILL
	  val = INTERNAL_SYSCALL (tgkill, err, 3,
				  THREAD_GETMEM (THREAD_SELF, pid),
				  t->tid, SIGSETXID);
#else
# ifdef __NR_tgkill
	  val = INTERNAL_SYSCALL (tgkill, err, 3,
				  THREAD_GETMEM (THREAD_SELF, pid),
				  t->tid, SIGSETXID);
	  if (INTERNAL_SYSCALL_ERROR_P (val, err)
	      && INTERNAL_SYSCALL_ERRNO (val, err) == ENOSYS)
# endif
	    val = INTERNAL_SYSCALL (tkill, err, 2, t->tid, SIGSETXID);
#endif

	  if (!INTERNAL_SYSCALL_ERROR_P (val, err))
	    atomic_increment (&cmdp->cntr);
	}
    }

  /* Now the list with threads using user-allocated stacks.  */
  list_for_each (runp, &__stack_user)
    {
      struct pthread *t = list_entry (runp, struct pthread, list);
      if (t != self)
	{
	  int val;
#if __ASSUME_TGKILL
	  val = INTERNAL_SYSCALL (tgkill, err, 3,
				  THREAD_GETMEM (THREAD_SELF, pid),
				  t->tid, SIGSETXID);
#else
# ifdef __NR_tgkill
	  val = INTERNAL_SYSCALL (tgkill, err, 3,
				  THREAD_GETMEM (THREAD_SELF, pid),
				  t->tid, SIGSETXID);
	  if (INTERNAL_SYSCALL_ERROR_P (val, err)
	      && INTERNAL_SYSCALL_ERRNO (val, err) == ENOSYS)
# endif
	    val = INTERNAL_SYSCALL (tkill, err, 2, t->tid, SIGSETXID);
#endif

	  if (!INTERNAL_SYSCALL_ERROR_P (val, err))
	    atomic_increment (&cmdp->cntr);
	}
    }

  int cur = cmdp->cntr;
  while (cur != 0)
    {
      lll_futex_wait (&cmdp->cntr, cur);
      cur = cmdp->cntr;
    }

  /* This must be last, otherwise the current thread might not have
     permissions to send SIGSETXID syscall to the other threads.  */
  result = INTERNAL_SYSCALL_NCS (cmdp->syscall_no, err, 3,
				 cmdp->id[0], cmdp->id[1], cmdp->id[2]);
  if (INTERNAL_SYSCALL_ERROR_P (result, err))
    {
      __set_errno (INTERNAL_SYSCALL_ERRNO (result, err));
      result = -1;
    }

  lll_unlock (stack_cache_lock);
  return result;
}

static inline void __attribute__((always_inline))
init_one_static_tls (struct pthread *curp, struct link_map *map)
{
  dtv_t *dtv = GET_DTV (TLS_TPADJ (curp));
# if TLS_TCB_AT_TP
  void *dest = (char *) curp - map->l_tls_offset;
# elif TLS_DTV_AT_TP
  void *dest = (char *) curp + map->l_tls_offset + TLS_PRE_TCB_SIZE;
# else
#  error "Either TLS_TCB_AT_TP or TLS_DTV_AT_TP must be defined"
# endif

  /* Fill in the DTV slot so that a later LD/GD access will find it.  */
  dtv[map->l_tls_modid].pointer = dest;

  /* Initialize the memory.  */
  memset (__mempcpy (dest, map->l_tls_initimage, map->l_tls_initimage_size),
	  '\0', map->l_tls_blocksize - map->l_tls_initimage_size);
}

void
attribute_hidden
__pthread_init_static_tls (struct link_map *map)
{
  lll_lock (stack_cache_lock);

  /* Iterate over the list with system-allocated threads first.  */
  list_t *runp;
  list_for_each (runp, &stack_used)
    init_one_static_tls (list_entry (runp, struct pthread, list), map);

  /* Now the list with threads using user-allocated stacks.  */
  list_for_each (runp, &__stack_user)
    init_one_static_tls (list_entry (runp, struct pthread, list), map);

  lll_unlock (stack_cache_lock);
}
