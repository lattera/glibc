/* Linuxthreads - a simple clone()-based implementation of Posix        */
/* threads for Linux.                                                   */
/* Copyright (C) 1996 Xavier Leroy (Xavier.Leroy@inria.fr)              */
/*                                                                      */
/* This program is free software; you can redistribute it and/or        */
/* modify it under the terms of the GNU Library General Public License  */
/* as published by the Free Software Foundation; either version 2       */
/* of the License, or (at your option) any later version.               */
/*                                                                      */
/* This program is distributed in the hope that it will be useful,      */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of       */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        */
/* GNU Library General Public License for more details.                 */

#ifndef _INTERNALS_H
#define _INTERNALS_H	1

/* Internal data structures */

/* Includes */

#include <limits.h>
#include <signal.h>
#include <unistd.h>
#include <stackinfo.h>
#include <sigcontextinfo.h>

#include <tls.h>
#include "descr.h"

#include "semaphore.h"

#ifndef THREAD_GETMEM
# define THREAD_GETMEM(descr, member) descr->member
#endif
#ifndef THREAD_GETMEM_NC
# define THREAD_GETMEM_NC(descr, member) descr->member
#endif
#ifndef THREAD_SETMEM
# define THREAD_SETMEM(descr, member, value) descr->member = (value)
#endif
#ifndef THREAD_SETMEM_NC
# define THREAD_SETMEM_NC(descr, member, value) descr->member = (value)
#endif

typedef void (*destr_function)(void *);

struct pthread_key_struct {
  int in_use;                   /* already allocated? */
  destr_function destr;         /* destruction routine */
};


#define PTHREAD_START_ARGS_INITIALIZER(fct) \
  { (void *(*) (void *)) fct, NULL, {{0, }}, 0, { 0 } }


/* The type of thread handles. */

typedef struct pthread_handle_struct * pthread_handle;

struct pthread_handle_struct {
  struct _pthread_fastlock h_lock; /* Fast lock for sychronized access */
  pthread_descr h_descr;        /* Thread descriptor or NULL if invalid */
  char * h_bottom;              /* Lowest address in the stack thread */
};

/* The type of messages sent to the thread manager thread */

struct pthread_request {
  pthread_descr req_thread;     /* Thread doing the request */
  enum {                        /* Request kind */
    REQ_CREATE, REQ_FREE, REQ_PROCESS_EXIT, REQ_MAIN_THREAD_EXIT,
    REQ_POST, REQ_DEBUG, REQ_KICK, REQ_FOR_EACH_THREAD
  } req_kind;
  union {                       /* Arguments for request */
    struct {                    /* For REQ_CREATE: */
      const pthread_attr_t * attr; /* thread attributes */
      void * (*fn)(void *);     /*   start function */
      void * arg;               /*   argument to start function */
      sigset_t mask;            /*   signal mask */
    } create;
    struct {                    /* For REQ_FREE: */
      pthread_t thread_id;      /*   identifier of thread to free */
    } free;
    struct {                    /* For REQ_PROCESS_EXIT: */
      int code;                 /*   exit status */
    } exit;
    void * post;                /* For REQ_POST: the semaphore */
    struct {			/* For REQ_FOR_EACH_THREAD: callback */
      void (*fn)(void *, pthread_descr);
      void *arg;
    } for_each;
  } req_args;
};



typedef void (*arch_sighandler_t) (int, SIGCONTEXT);
union sighandler
{
  arch_sighandler_t old;
  void (*rt) (int, struct siginfo *, struct ucontext *);
};
extern union sighandler __sighandler[NSIG];


/* Signals used for suspend/restart and for cancellation notification.  */

extern int __pthread_sig_restart;
extern int __pthread_sig_cancel;

/* Signal used for interfacing with gdb */

extern int __pthread_sig_debug;

/* Global array of thread handles, used for validating a thread id
   and retrieving the corresponding thread descriptor. Also used for
   mapping the available stack segments. */

extern struct pthread_handle_struct __pthread_handles[PTHREAD_THREADS_MAX];

/* Descriptor of the main thread */

extern pthread_descr __pthread_main_thread;

/* File descriptor for sending requests to the thread manager.
   Initially -1, meaning that __pthread_initialize_manager must be called. */

extern int __pthread_manager_request;

/* Other end of the pipe for sending requests to the thread manager. */

extern int __pthread_manager_reader;

#ifdef FLOATING_STACKS
/* Maximum stack size.  */
extern size_t __pthread_max_stacksize;
#endif

/* Pending request for a process-wide exit */

extern int __pthread_exit_requested, __pthread_exit_code;

/* Set to 1 by gdb if we're debugging */

extern volatile int __pthread_threads_debug;

/* Globally enabled events.  */
extern volatile td_thr_events_t __pthread_threads_events;

/* Pointer to descriptor of thread with last event.  */
extern volatile pthread_descr __pthread_last_event;

/* Flag which tells whether we are executing on SMP kernel. */
extern int __pthread_smp_kernel;

/* Return the handle corresponding to a thread id */

static inline pthread_handle thread_handle(pthread_t id)
{
  return &__pthread_handles[id % PTHREAD_THREADS_MAX];
}

/* Validate a thread handle. Must have acquired h->h_spinlock before. */

static inline int invalid_handle(pthread_handle h, pthread_t id)
{
  return h->h_descr == NULL || h->h_descr->p_tid != id || h->h_descr->p_terminated;
}

static inline int nonexisting_handle(pthread_handle h, pthread_t id)
{
  return h->h_descr == NULL || h->h_descr->p_tid != id;
}

/* Fill in defaults left unspecified by pt-machine.h.  */

/* We round up a value with page size. */
#ifndef page_roundup
#define page_roundup(v,p) ((((size_t) (v)) + (p) - 1) & ~((p) - 1))
#endif

/* The page size we can get from the system.  This should likely not be
   changed by the machine file but, you never know.  */
#ifndef PAGE_SIZE
#define PAGE_SIZE  (sysconf (_SC_PAGE_SIZE))
#endif

/* The initial size of the thread stack.  Must be a multiple of PAGE_SIZE.  */
#ifndef INITIAL_STACK_SIZE
#define INITIAL_STACK_SIZE  (4 * PAGE_SIZE)
#endif

/* Size of the thread manager stack. The "- 32" avoids wasting space
   with some malloc() implementations. */
#ifndef THREAD_MANAGER_STACK_SIZE
#define THREAD_MANAGER_STACK_SIZE  (2 * PAGE_SIZE - 32)
#endif

/* The base of the "array" of thread stacks.  The array will grow down from
   here.  Defaults to the calculated bottom of the initial application
   stack.  */
#ifndef THREAD_STACK_START_ADDRESS
#define THREAD_STACK_START_ADDRESS  __pthread_initial_thread_bos
#endif

/* If MEMORY_BARRIER isn't defined in pt-machine.h, assume the
   architecture doesn't need a memory barrier instruction (e.g. Intel
   x86).  Still we need the compiler to respect the barrier and emit
   all outstanding operations which modify memory.  Some architectures
   distinguish between full, read and write barriers.  */

#ifndef MEMORY_BARRIER
#define MEMORY_BARRIER() asm ("" : : : "memory")
#endif
#ifndef READ_MEMORY_BARRIER
#define READ_MEMORY_BARRIER() MEMORY_BARRIER()
#endif
#ifndef WRITE_MEMORY_BARRIER
#define WRITE_MEMORY_BARRIER() MEMORY_BARRIER()
#endif

/* Max number of times we must spin on a spinlock calling sched_yield().
   After MAX_SPIN_COUNT iterations, we put the calling thread to sleep. */

#ifndef MAX_SPIN_COUNT
#define MAX_SPIN_COUNT 50
#endif

/* Max number of times the spinlock in the adaptive mutex implementation
   spins actively on SMP systems.  */

#ifndef MAX_ADAPTIVE_SPIN_COUNT
#define MAX_ADAPTIVE_SPIN_COUNT 100
#endif

/* Duration of sleep (in nanoseconds) when we can't acquire a spinlock
   after MAX_SPIN_COUNT iterations of sched_yield().
   With the 2.0 and 2.1 kernels, this MUST BE > 2ms.
   (Otherwise the kernel does busy-waiting for realtime threads,
    giving other threads no chance to run.) */

#ifndef SPIN_SLEEP_DURATION
#define SPIN_SLEEP_DURATION 2000001
#endif

/* Debugging */

#ifdef DEBUG
#include <assert.h>
#define ASSERT assert
#define MSG __pthread_message
#else
#define ASSERT(x)
#define MSG(msg,arg...)
#endif

/* Internal global functions */

extern void __pthread_do_exit (void *retval, char *currentframe)
     __attribute__ ((__noreturn__));
extern void __pthread_destroy_specifics (void);
extern void __pthread_perform_cleanup (char *currentframe);
extern void __pthread_init_max_stacksize (void);
extern int __pthread_initialize_manager (void);
extern void __pthread_message (const char * fmt, ...);
extern int __pthread_manager (void *reqfd);
extern int __pthread_manager_event (void *reqfd);
extern void __pthread_manager_sighandler (int sig);
extern void __pthread_reset_main_thread (void);
extern void __pthread_once_fork_prepare (void);
extern void __pthread_once_fork_parent (void);
extern void __pthread_once_fork_child (void);
extern void __flockfilelist (void);
extern void __funlockfilelist (void);
extern void __fresetlockfiles (void);
extern void __pthread_manager_adjust_prio (int thread_prio);
extern void __pthread_initialize_minimal (void);

extern int __pthread_attr_setguardsize (pthread_attr_t *__attr,
					size_t __guardsize);
extern int __pthread_attr_getguardsize (const pthread_attr_t *__attr,
					size_t *__guardsize);
extern int __pthread_attr_setstackaddr (pthread_attr_t *__attr,
					void *__stackaddr);
extern int __pthread_attr_getstackaddr (const pthread_attr_t *__attr,
					void **__stackaddr);
extern int __pthread_attr_setstacksize (pthread_attr_t *__attr,
					size_t __stacksize);
extern int __pthread_attr_getstacksize (const pthread_attr_t *__attr,
					size_t *__stacksize);
extern int __pthread_attr_setstack (pthread_attr_t *__attr, void *__stackaddr,
				    size_t __stacksize);
extern int __pthread_attr_getstack (const pthread_attr_t *__attr, void **__stackaddr,
				    size_t *__stacksize);
extern int __pthread_getconcurrency (void);
extern int __pthread_setconcurrency (int __level);
extern int __pthread_mutex_timedlock (pthread_mutex_t *__mutex,
				      const struct timespec *__abstime);
extern int __pthread_mutexattr_getpshared (const pthread_mutexattr_t *__attr,
					   int *__pshared);
extern int __pthread_mutexattr_setpshared (pthread_mutexattr_t *__attr,
					   int __pshared);
extern int __pthread_mutexattr_gettype (const pthread_mutexattr_t *__attr,
					int *__kind);
extern void __pthread_kill_other_threads_np (void);

extern void __pthread_restart_old(pthread_descr th);
extern void __pthread_suspend_old(pthread_descr self);
extern int __pthread_timedsuspend_old(pthread_descr self, const struct timespec *abs);

extern void __pthread_restart_new(pthread_descr th);
extern void __pthread_suspend_new(pthread_descr self);
extern int __pthread_timedsuspend_new(pthread_descr self, const struct timespec *abs);

extern void __pthread_wait_for_restart_signal(pthread_descr self);

extern int __pthread_yield (void);

extern int __pthread_rwlock_timedrdlock (pthread_rwlock_t *__restrict __rwlock,
					 __const struct timespec *__restrict
					 __abstime);
extern int __pthread_rwlock_timedwrlock (pthread_rwlock_t *__restrict __rwlock,
					 __const struct timespec *__restrict
					 __abstime);
extern int __pthread_rwlockattr_destroy (pthread_rwlockattr_t *__attr);

extern int __pthread_barrierattr_getpshared (__const pthread_barrierattr_t *
					     __restrict __attr,
					     int *__restrict __pshared);

extern int __pthread_spin_lock (pthread_spinlock_t *__lock);
extern int __pthread_spin_trylock (pthread_spinlock_t *__lock);
extern int __pthread_spin_unlock (pthread_spinlock_t *__lock);
extern int __pthread_spin_init (pthread_spinlock_t *__lock, int __pshared);
extern int __pthread_spin_destroy (pthread_spinlock_t *__lock);

extern int __pthread_clock_gettime (hp_timing_t freq, struct timespec *tp);
extern void __pthread_clock_settime (hp_timing_t offset);


/* Global pointers to old or new suspend functions */

extern void (*__pthread_restart)(pthread_descr);
extern void (*__pthread_suspend)(pthread_descr);
extern int (*__pthread_timedsuspend)(pthread_descr, const struct timespec *);

/* Prototypes for the function without cancelation support when the
   normal version has it.  */
extern int __libc_close (int fd);
extern int __libc_nanosleep (const struct timespec *requested_time,
			     struct timespec *remaining);
/* Prototypes for some of the new semaphore functions.  */
extern int __new_sem_post (sem_t * sem);
extern int __new_sem_init (sem_t *__sem, int __pshared, unsigned int __value);
extern int __new_sem_wait (sem_t *__sem);
extern int __new_sem_trywait (sem_t *__sem);
extern int __new_sem_getvalue (sem_t *__restrict __sem, int *__restrict __sval);
extern int __new_sem_destroy (sem_t *__sem);

/* Prototypes for compatibility functions.  */
extern int __pthread_attr_init_2_1 (pthread_attr_t *__attr);
extern int __pthread_attr_init_2_0 (pthread_attr_t *__attr);
extern int __pthread_create_2_1 (pthread_t *__restrict __threadp,
				 const pthread_attr_t *__attr,
				 void *(*__start_routine) (void *),
				 void *__restrict __arg);
extern int __pthread_create_2_0 (pthread_t *__restrict thread,
				 const pthread_attr_t *__attr,
				 void *(*__start_routine) (void *),
				 void *__restrict arg);

/* The functions called the signal events.  */
extern void __linuxthreads_create_event (void);
extern void __linuxthreads_death_event (void);
extern void __linuxthreads_reap_event (void);

/* This function is called to initialize the pthread library.  */
extern void __pthread_initialize (void);


/* Sighandler wrappers.  */
extern void __pthread_sighandler(int signo, SIGCONTEXT ctx);
extern void __pthread_sighandler_rt(int signo, struct siginfo *si,
				    struct ucontext *uc);
extern void __pthread_null_sighandler(int sig);

#endif /* internals.h */
