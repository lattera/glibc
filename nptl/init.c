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
#include <limits.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/resource.h>
#include <pthreadP.h>
#include <atomic.h>
#include <ldsodefs.h>
#include <tls.h>
#include <fork.h>
#include <version.h>
#include <shlib-compat.h>
#include <smp.h>
#include <lowlevellock.h>


#ifndef __NR_set_tid_address
/* XXX For the time being...  Once we can rely on the kernel headers
   having the definition remove these lines.  */
#if defined __s390__
# define __NR_set_tid_address	252
#elif defined __ia64__
# define __NR_set_tid_address	1233
#elif defined __i386__
# define __NR_set_tid_address	258
#elif defined __x86_64__
# define __NR_set_tid_address	218
#elif defined __powerpc__
# define __NR_set_tid_address	232
#elif defined __sparc__
# define __NR_set_tid_address	166
#else
# error "define __NR_set_tid_address"
#endif
#endif


/* Size and alignment of static TLS block.  */
size_t __static_tls_size;
size_t __static_tls_align_m1;

/* Version of the library, used in libthread_db to detect mismatches.  */
static const char nptl_version[] __attribute_used__ = VERSION;


#if defined USE_TLS && !defined SHARED
extern void __libc_setup_tls (size_t tcbsize, size_t tcbalign);
#endif


#ifdef SHARED
static const struct pthread_functions pthread_functions =
  {
    .ptr_pthread_attr_destroy = __pthread_attr_destroy,
# if SHLIB_COMPAT(libpthread, GLIBC_2_0, GLIBC_2_1)
    .ptr___pthread_attr_init_2_0 = __pthread_attr_init_2_0,
# endif
    .ptr___pthread_attr_init_2_1 = __pthread_attr_init_2_1,
    .ptr_pthread_attr_getdetachstate = __pthread_attr_getdetachstate,
    .ptr_pthread_attr_setdetachstate = __pthread_attr_setdetachstate,
    .ptr_pthread_attr_getinheritsched = __pthread_attr_getinheritsched,
    .ptr_pthread_attr_setinheritsched = __pthread_attr_setinheritsched,
    .ptr_pthread_attr_getschedparam = __pthread_attr_getschedparam,
    .ptr_pthread_attr_setschedparam = __pthread_attr_setschedparam,
    .ptr_pthread_attr_getschedpolicy = __pthread_attr_getschedpolicy,
    .ptr_pthread_attr_setschedpolicy = __pthread_attr_setschedpolicy,
    .ptr_pthread_attr_getscope = __pthread_attr_getscope,
    .ptr_pthread_attr_setscope = __pthread_attr_setscope,
    .ptr_pthread_condattr_destroy = __pthread_condattr_destroy,
    .ptr_pthread_condattr_init = __pthread_condattr_init,
    .ptr___pthread_cond_broadcast = __pthread_cond_broadcast,
    .ptr___pthread_cond_destroy = __pthread_cond_destroy,
    .ptr___pthread_cond_init = __pthread_cond_init,
    .ptr___pthread_cond_signal = __pthread_cond_signal,
    .ptr___pthread_cond_wait = __pthread_cond_wait,
    .ptr___pthread_cond_timedwait = __pthread_cond_timedwait,
# if SHLIB_COMPAT(libpthread, GLIBC_2_0, GLIBC_2_3_2)
    .ptr___pthread_cond_broadcast_2_0 = __pthread_cond_broadcast_2_0,
    .ptr___pthread_cond_destroy_2_0 = __pthread_cond_destroy_2_0,
    .ptr___pthread_cond_init_2_0 = __pthread_cond_init_2_0,
    .ptr___pthread_cond_signal_2_0 = __pthread_cond_signal_2_0,
    .ptr___pthread_cond_wait_2_0 = __pthread_cond_wait_2_0,
    .ptr___pthread_cond_timedwait_2_0 = __pthread_cond_timedwait_2_0,
# endif
    .ptr_pthread_equal = __pthread_equal,
    .ptr___pthread_exit = __pthread_exit,
    .ptr_pthread_getschedparam = __pthread_getschedparam,
    .ptr_pthread_setschedparam = __pthread_setschedparam,
    .ptr_pthread_mutex_destroy = INTUSE(__pthread_mutex_destroy),
    .ptr_pthread_mutex_init = INTUSE(__pthread_mutex_init),
    .ptr_pthread_mutex_lock = INTUSE(__pthread_mutex_lock),
    .ptr_pthread_mutex_unlock = INTUSE(__pthread_mutex_unlock),
    .ptr_pthread_self = __pthread_self,
    .ptr_pthread_setcancelstate = __pthread_setcancelstate,
    .ptr_pthread_setcanceltype = __pthread_setcanceltype,
    .ptr___pthread_cleanup_upto = __pthread_cleanup_upto,
    .ptr___pthread_once = __pthread_once_internal,
    .ptr___pthread_rwlock_rdlock = __pthread_rwlock_rdlock_internal,
    .ptr___pthread_rwlock_wrlock = __pthread_rwlock_wrlock_internal,
    .ptr___pthread_rwlock_unlock = __pthread_rwlock_unlock_internal,
    .ptr___pthread_key_create = __pthread_key_create_internal,
    .ptr___pthread_getspecific = __pthread_getspecific_internal,
    .ptr___pthread_setspecific = __pthread_setspecific_internal,
    .ptr__pthread_cleanup_push_defer = __pthread_cleanup_push_defer,
    .ptr__pthread_cleanup_pop_restore = __pthread_cleanup_pop_restore,
    .ptr_nthreads = &__nptl_nthreads,
    .ptr___pthread_unwind = &__pthread_unwind,
    .ptr__nptl_deallocate_tsd = __nptl_deallocate_tsd,
    .ptr__nptl_setxid = __nptl_setxid
  };
# define ptr_pthread_functions &pthread_functions
#else
# define ptr_pthread_functions NULL
#endif


/* For asynchronous cancellation we use a signal.  This is the handler.  */
static void
sigcancel_handler (int sig, siginfo_t *si, void *ctx)
{
  /* Safety check.  It would be possible to call this function for
     other signals and send a signal from another process.  This is not
     correct and might even be a security problem.  Try to catch as
     many incorrect invocations as possible.  */
  if (sig != SIGCANCEL
#ifdef __ASSUME_CORRECT_SI_PID
      /* Kernels before 2.5.75 stored the thread ID and not the process
	 ID in si_pid so we skip this test.  */
      || si->si_pid != THREAD_GETMEM (THREAD_SELF, pid)
#endif
      || si->si_code != SI_TKILL)
    return;

  struct pthread *self = THREAD_SELF;

  int oldval = THREAD_GETMEM (self, cancelhandling);
  while (1)
    {
      /* We are canceled now.  When canceled by another thread this flag
	 is already set but if the signal is directly send (internally or
	 from another process) is has to be done here.  */
      int newval = oldval | CANCELING_BITMASK | CANCELED_BITMASK;

      if (oldval == newval || (oldval & EXITING_BITMASK) != 0)
	/* Already canceled or exiting.  */
	break;

      int curval = THREAD_ATOMIC_CMPXCHG_VAL (self, cancelhandling, newval,
					      oldval);
      if (curval == oldval)
	{
	  /* Set the return value.  */
	  THREAD_SETMEM (self, result, PTHREAD_CANCELED);

	  /* Make sure asynchronous cancellation is still enabled.  */
	  if ((newval & CANCELTYPE_BITMASK) != 0)
	    /* Run the registered destructors and terminate the thread.  */
	    __do_cancel ();

	  break;
	}

      oldval = curval;
    }
}


struct xid_command *__xidcmd attribute_hidden;

/* For asynchronous cancellation we use a signal.  This is the handler.  */
static void
sighandler_setxid (int sig, siginfo_t *si, void *ctx)
{
  /* Safety check.  It would be possible to call this function for
     other signals and send a signal from another process.  This is not
     correct and might even be a security problem.  Try to catch as
     many incorrect invocations as possible.  */
  if (sig != SIGSETXID
#ifdef __ASSUME_CORRECT_SI_PID
      /* Kernels before 2.5.75 stored the thread ID and not the process
	 ID in si_pid so we skip this test.  */
      || si->si_pid != THREAD_GETMEM (THREAD_SELF, pid)
#endif
      || si->si_code != SI_TKILL)
    return;

  INTERNAL_SYSCALL_DECL (err);
  INTERNAL_SYSCALL_NCS (__xidcmd->syscall_no, err, 3, __xidcmd->id[0],
			__xidcmd->id[1], __xidcmd->id[2]);

  if (atomic_decrement_val (&__xidcmd->cntr) == 0)
    lll_futex_wake (&__xidcmd->cntr, 1);
}


/* When using __thread for this, we do it in libc so as not
   to give libpthread its own TLS segment just for this.  */
extern void **__libc_dl_error_tsd (void) __attribute__ ((const));


void
__pthread_initialize_minimal_internal (void)
{
#ifndef SHARED
  /* Unlike in the dynamically linked case the dynamic linker has not
     taken care of initializing the TLS data structures.  */
  __libc_setup_tls (TLS_TCB_SIZE, TLS_TCB_ALIGN);

  /* We must prevent gcc from being clever and move any of the
     following code ahead of the __libc_setup_tls call.  This function
     will initialize the thread register which is subsequently
     used.  */
  __asm __volatile ("");
#endif

  /* Minimal initialization of the thread descriptor.  */
  struct pthread *pd = THREAD_SELF;
  INTERNAL_SYSCALL_DECL (err);
  pd->pid = pd->tid = INTERNAL_SYSCALL (set_tid_address, err, 1, &pd->tid);
  THREAD_SETMEM (pd, specific[0], &pd->specific_1stblock[0]);
  THREAD_SETMEM (pd, user_stack, true);
  if (LLL_LOCK_INITIALIZER != 0)
    THREAD_SETMEM (pd, lock, LLL_LOCK_INITIALIZER);
#if HP_TIMING_AVAIL
  THREAD_SETMEM (pd, cpuclock_offset, GL(dl_cpuclock_offset));
#endif

  /* Set initial thread's stack block from 0 up to __libc_stack_end.
     It will be bigger than it actually is, but for unwind.c/pt-longjmp.c
     purposes this is good enough.  */
  THREAD_SETMEM (pd, stackblock_size, (size_t) __libc_stack_end);

  /* Initialize the list of all running threads with the main thread.  */
  INIT_LIST_HEAD (&__stack_user);
  list_add (&pd->list, &__stack_user);


  /* Install the cancellation signal handler.  If for some reason we
     cannot install the handler we do not abort.  Maybe we should, but
     it is only asynchronous cancellation which is affected.  */
  struct sigaction sa;
  sa.sa_sigaction = sigcancel_handler;
  sa.sa_flags = SA_SIGINFO;
  __sigemptyset (&sa.sa_mask);

  (void) __libc_sigaction (SIGCANCEL, &sa, NULL);

  /* Install the handle to change the threads' uid/gid.  */
  sa.sa_sigaction = sighandler_setxid;
  sa.sa_flags = SA_SIGINFO | SA_RESTART;

  (void) __libc_sigaction (SIGSETXID, &sa, NULL);

  /* The parent process might have left the signals blocked.  Just in
     case, unblock it.  We reuse the signal mask in the sigaction
     structure.  It is already cleared.  */
  __sigaddset (&sa.sa_mask, SIGCANCEL);
  __sigaddset (&sa.sa_mask, SIGSETXID);
  (void) INTERNAL_SYSCALL (rt_sigprocmask, err, 4, SIG_UNBLOCK, &sa.sa_mask,
			   NULL, _NSIG / 8);


  /* Determine the default allowed stack size.  This is the size used
     in case the user does not specify one.  */
  struct rlimit limit;
  if (getrlimit (RLIMIT_STACK, &limit) != 0
      || limit.rlim_cur == RLIM_INFINITY)
    /* The system limit is not usable.  Use an architecture-specific
       default.  */
    __default_stacksize = ARCH_STACK_DEFAULT_SIZE;
  else if (limit.rlim_cur < PTHREAD_STACK_MIN)
    /* The system limit is unusably small.
       Use the minimal size acceptable.  */
    __default_stacksize = PTHREAD_STACK_MIN;
  else
    {
      /* Round the resource limit up to page size.  */
      const uintptr_t pagesz = __sysconf (_SC_PAGESIZE);
      __default_stacksize = (limit.rlim_cur + pagesz - 1) & -pagesz;
    }

  /* Get the size of the static and alignment requirements for the TLS
     block.  */
  size_t static_tls_align;
  _dl_get_tls_static_info (&__static_tls_size, &static_tls_align);

  /* Make sure the size takes all the alignments into account.  */
  if (STACK_ALIGN > static_tls_align)
    static_tls_align = STACK_ALIGN;
  __static_tls_align_m1 = static_tls_align - 1;

  __static_tls_size = roundup (__static_tls_size, static_tls_align);

#ifdef SHARED
  /* Transfer the old value from the dynamic linker's internal location.  */
  *__libc_dl_error_tsd () = *(*GL(dl_error_catch_tsd)) ();
  GL(dl_error_catch_tsd) = &__libc_dl_error_tsd;

  /* Make __rtld_lock_{,un}lock_recursive use pthread_mutex_{,un}lock,
     keep the lock count from the ld.so implementation.  */
  GL(dl_rtld_lock_recursive) = (void *) INTUSE (__pthread_mutex_lock);
  GL(dl_rtld_unlock_recursive) = (void *) INTUSE (__pthread_mutex_unlock);
  unsigned int rtld_lock_count = GL(dl_load_lock).mutex.__data.__count;
  GL(dl_load_lock).mutex.__data.__count = 0;
  while (rtld_lock_count-- > 0)
    INTUSE (__pthread_mutex_lock) (&GL(dl_load_lock).mutex);

  GL(dl_make_stack_executable_hook) = &__make_stacks_executable;
#endif

  GL(dl_init_static_tls) = &__pthread_init_static_tls;

  /* Register the fork generation counter with the libc.  */
#ifndef TLS_MULTIPLE_THREADS_IN_TCB
  __libc_multiple_threads_ptr =
#endif
    __libc_pthread_init (&__fork_generation, __reclaim_stacks,
			 ptr_pthread_functions);

  /* Determine whether the machine is SMP or not.  */
  __is_smp = is_smp_system ();
}
strong_alias (__pthread_initialize_minimal_internal,
	      __pthread_initialize_minimal)
