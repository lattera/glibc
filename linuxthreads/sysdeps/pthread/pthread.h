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

#ifndef _PTHREAD_H
#define _PTHREAD_H	1

#include <features.h>

#include <errno.h>
#include <limits.h>
#include <sched.h>
#include <unistd.h>

#define __need_sigset_t
#include <signal.h>
#define __need_timespec
#include <time.h>

/* Linux has no ENOTSUP error code.  */
#ifndef ENOTSUP
#define ENOTSUP EOPNOTSUPP
#endif


__BEGIN_DECLS

/*** Types ***/

/* Thread identifiers */
typedef unsigned long int pthread_t;

/* Thread descriptors */
typedef struct _pthread_descr_struct *_pthread_descr;

/* Fast locks (not abstract because mutexes and conditions aren't abstract). */
struct _pthread_fastlock
{
  long int status;              /* "Free" or "taken" or head of waiting list */
  int spinlock;                 /* For compare-and-swap emulation */
};

/* Mutexes (not abstract because of PTHREAD_MUTEX_INITIALIZER).  */
/* (The layout is unnatural to maintain binary compatibility
    with earlier releases of LinuxThreads.) */

typedef struct
{
  int m_reserved;               /* Reserved for future use */
  int m_count;                  /* Depth of recursive locking */
  _pthread_descr m_owner;       /* Owner thread (if recursive or errcheck) */
  int m_kind;                   /* Mutex kind: fast, recursive or errcheck */
  struct _pthread_fastlock m_lock; /* Underlying fast lock */
} pthread_mutex_t;

#define PTHREAD_MUTEX_INITIALIZER \
  {0, 0, 0, PTHREAD_MUTEX_FAST_NP, {0, 0}}
#define PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP \
  {0, 0, 0, PTHREAD_MUTEX_RECURSIVE_NP, {0, 0}}

/* Conditions (not abstract because of PTHREAD_COND_INITIALIZER */
typedef struct
{
  struct _pthread_fastlock c_lock; /* Protect against concurrent access */
  _pthread_descr c_waiting;        /* Threads waiting on this condition */
} pthread_cond_t;

#define PTHREAD_COND_INITIALIZER {{0, 0}, 0}

#ifdef __USE_UNIX98
/* Read-write locks.  */
typedef struct
{
  struct _pthread_fastlock rw_lock; /* Lock to guarantee mutual exclusion */
  int rw_readers;               /* Number of readers */
  _pthread_descr rw_writer;     /* Identity of writer, or NULL if none */
  _pthread_descr rw_read_waiting; /* Threads waiting for reading */
  _pthread_descr rw_write_waiting; /* Threads waiting for writing */
  int rw_kind;                  /* Reader/Writer preference selection */
  int rw_pshared;               /* Shared between processes or not */
} pthread_rwlock_t;

# define PTHREAD_RWLOCK_INITIALIZER \
  { {0, 0}, 0, NULL, NULL, NULL,					      \
    PTHREAD_RWLOCK_DEFAULT_NP, PTHREAD_PROCESS_PRIVATE }
#endif

/* Attributes */

enum
{
  PTHREAD_CREATE_JOINABLE,
#define PTHREAD_CREATE_JOINABLE	PTHREAD_CREATE_JOINABLE
  PTHREAD_CREATE_DETACHED
#define PTHREAD_CREATE_DETACHED	PTHREAD_CREATE_DETACHED
};

enum
{
  PTHREAD_INHERIT_SCHED,
#define PTHREAD_INHERIT_SCHED	PTHREAD_INHERIT_SCHED
  PTHREAD_EXPLICIT_SCHED
#define PTHREAD_EXPLICIT_SCHED	PTHREAD_EXPLICIT_SCHED
};

enum
{
  PTHREAD_SCOPE_SYSTEM,
#define PTHREAD_SCOPE_SYSTEM	PTHREAD_SCOPE_SYSTEM
  PTHREAD_SCOPE_PROCESS
#define PTHREAD_SCOPE_PROCESS	PTHREAD_SCOPE_PROCESS
};

typedef struct
{
  int detachstate;
  int schedpolicy;
  struct sched_param schedparam;
  int inheritsched;
  int scope;
  size_t guardsize;
  int stackaddr_set;
  void *stackaddr;
  size_t stacksize;
} pthread_attr_t;

enum
{
  PTHREAD_MUTEX_FAST_NP,
  PTHREAD_MUTEX_RECURSIVE_NP,
  PTHREAD_MUTEX_ERRORCHECK_NP
#ifdef __USE_UNIX98
  ,
  PTHREAD_MUTEX_NORMAL = PTHREAD_MUTEX_FAST_NP,
  PTHREAD_MUTEX_RECURSIVE = PTHREAD_MUTEX_RECURSIVE_NP,
  PTHREAD_MUTEX_ERRORCHECK = PTHREAD_MUTEX_ERRORCHECK_NP,
  PTHREAD_MUTEX_DEFAULT = PTHREAD_MUTEX_NORMAL
#endif
};

typedef struct
{
  int mutexkind;
} pthread_mutexattr_t;

typedef struct
{
  int dummy;
} pthread_condattr_t;

#ifdef __USE_UNIX98
enum
{
  PTHREAD_PROCESS_PRIVATE,
# define PTHREAD_PROCESS_PRIVATE	PTHREAD_PROCESS_PRIVATE
  PTHREAD_PROCESS_SHARED
# define PTHREAD_PROCESS_SHARED		PTHREAD_PROCESS_SHARED
};

enum
{
  PTHREAD_RWLOCK_PREFER_READER_NP,
  PTHREAD_RWLOCK_PREFER_WRITER_NP,
  PTHREAD_RWLOCK_DEFAULT_NP = PTHREAD_RWLOCK_PREFER_WRITER_NP
};

typedef struct
{
  int lockkind;
  int pshared;
} pthread_rwlockattr_t;
#endif

/* Keys for thread-specific data */

typedef unsigned int pthread_key_t;

/* Once-only execution */

typedef int pthread_once_t;

#define PTHREAD_ONCE_INIT 0

/* Cleanup buffers */

struct _pthread_cleanup_buffer
{
  void (*routine) __PMT ((void *));	/* Function to call.  */
  void *arg;				/* Its argument.  */
  int canceltype;			/* Saved cancellation type. */
  struct _pthread_cleanup_buffer *prev;	/* Chaining of cleanup functions.  */
};

/* Cancellation */

enum
{
  PTHREAD_CANCEL_ENABLE,
#define PTHREAD_CANCEL_ENABLE	PTHREAD_CANCEL_ENABLE
  PTHREAD_CANCEL_DISABLE
#define PTHREAD_CANCEL_DISABLE	PTHREAD_CANCEL_DISABLE
};
enum
{
  PTHREAD_CANCEL_DEFERRED,
#define PTHREAD_CANCEL_DEFERRED	PTHREAD_CANCEL_DEFERRED
  PTHREAD_CANCEL_ASYNCHRONOUS
#define PTHREAD_CANCEL_ASYNCHRONOUS	PTHREAD_CANCEL_ASYNCHRONOUS
};
#define PTHREAD_CANCELED ((void *) -1)


/* Function for handling threads.  */

/* Create a thread with given attributes ATTR (or default attributes
   if ATTR is NULL), and call function START_ROUTINE with given
   arguments ARG.  */
extern int pthread_create __P ((pthread_t *__thread,
				__const pthread_attr_t *__attr,
				void *(*__start_routine) (void *),
				void *__arg));

/* Obtain the identifier of the current thread.  */
extern pthread_t pthread_self __P ((void));

/* Compare two thread identifiers.  */
extern int pthread_equal __P ((pthread_t __thread1, pthread_t __thread2));

/* Terminate calling thread.  */
extern void pthread_exit __P ((void *__retval)) __attribute__ ((__noreturn__));

/* Make calling thread wait for termination of the thread TH.  The
   exit status of the thread is stored in *THREAD_RETURN, if THREAD_RETURN
   is not NULL.  */
extern int pthread_join __P ((pthread_t __th, void **__thread_return));

/* Indicate that the thread TH is never to be joined with PTHREAD_JOIN.
   The resources of TH will therefore be freed immediately when it
   terminates, instead of waiting for another thread to perform PTHREAD_JOIN
   on it. */
extern int pthread_detach __P ((pthread_t __th));


/* Functions for handling attributes.  */

/* Initialize thread attribute *ATTR with default attributes
   (detachstate is PTHREAD_JOINABLE, scheduling policy is SCHED_OTHER,
    no user-provided stack).  */
extern int pthread_attr_init __P ((pthread_attr_t *__attr));

/* Destroy thread attribute *ATTR.  */
extern int pthread_attr_destroy __P ((pthread_attr_t *__attr));

/* Set the `detachstate' attribute in *ATTR according to DETACHSTATE.  */
extern int pthread_attr_setdetachstate __P ((pthread_attr_t *__attr,
					     int __detachstate));

/* Return in *DETACHSTATE the `detachstate' attribute in *ATTR.  */
extern int pthread_attr_getdetachstate __P ((__const pthread_attr_t *__attr,
					     int *__detachstate));

/* Set scheduling parameters (priority, etc) in *ATTR according to PARAM.  */
extern int pthread_attr_setschedparam __P ((pthread_attr_t *__attr,
				        __const struct sched_param *__param));

/* Return in *PARAM the scheduling parameters of *ATTR.  */
extern int pthread_attr_getschedparam __P ((__const pthread_attr_t *__attr,
					    struct sched_param *__param));

/* Set scheduling policy in *ATTR according to POLICY.  */
extern int pthread_attr_setschedpolicy __P ((pthread_attr_t *__attr,
					     int __policy));

/* Return in *POLICY the scheduling policy of *ATTR.  */
extern int pthread_attr_getschedpolicy __P ((__const pthread_attr_t *__attr,
					     int *__policy));

/* Set scheduling inheritance mode in *ATTR according to INHERIT.  */
extern int pthread_attr_setinheritsched __P ((pthread_attr_t *__attr,
					      int __inherit));

/* Return in *INHERIT the scheduling inheritance mode of *ATTR.  */
extern int pthread_attr_getinheritsched __P ((__const pthread_attr_t *__attr,
					      int *__inherit));

/* Set scheduling contention scope in *ATTR according to SCOPE.  */
extern int pthread_attr_setscope __P ((pthread_attr_t *__attr, int __scope));

/* Return in *SCOPE the scheduling contention scope of *ATTR.  */
extern int pthread_attr_getscope __P ((__const pthread_attr_t *__attr,
				       int *__scope));

#ifdef __USE_UNIX98
/* Set the size of the guard area at the bottom of the thread.  */
extern int __pthread_attr_setguardsize __P ((pthread_attr_t *__attr,
					     size_t __guardsize));
extern int pthread_attr_setguardsize __P ((pthread_attr_t *__attr,
					   size_t __guardsize));

/* Get the size of the guard area at the bottom of the thread.  */
extern int __pthread_attr_getguardsize __P ((__const pthread_attr_t *__attr,
					     size_t *__guardsize));
extern int pthread_attr_getguardsize __P ((__const pthread_attr_t *__attr,
					   size_t *__guardsize));
#endif

/* Set the starting address of the stack of the thread to be created.
   Depending on whether the stack grows up or doen the value must either
   be higher or lower than all the address in the memory block.  The
   minimal size of the block must be PTHREAD_STACK_SIZE.  */
extern int __pthread_attr_setstackaddr __P ((pthread_attr_t *__attr,
					     void *__stackaddr));
extern int pthread_attr_setstackaddr __P ((pthread_attr_t *__attr,
					   void *__stackaddr));

/* Return the previously set address for the stack.  */
extern int __pthread_attr_getstackaddr __P ((__const pthread_attr_t *__attr,
					     void **__stackaddr));
extern int pthread_attr_getstackaddr __P ((__const pthread_attr_t *__attr,
					   void **__stackaddr));

/* Add information about the minimum stack size needed for the thread
   to be started.  This size must never be less than PTHREAD_STACK_SIZE
   and must also not exceed the system limits.  */
extern int __pthread_attr_setstacksize __P ((pthread_attr_t *__attr,
					     size_t __stacksize));
extern int pthread_attr_setstacksize __P ((pthread_attr_t *__attr,
					   size_t __stacksize));

/* Return the currently used minimal stack size.  */
extern int __pthread_attr_getstacksize __P ((__const pthread_attr_t *__attr,
					     size_t *__stacksize));
extern int pthread_attr_getstacksize __P ((__const pthread_attr_t *__attr,
					   size_t *__stacksize));

/* Functions for scheduling control. */

/* Set the scheduling parameters for TARGET_THREAD according to POLICY
   and *PARAM. */
extern int pthread_setschedparam __P ((pthread_t __target_thread, int __policy,
				       __const struct sched_param *__param));

/* Return in *POLICY and *PARAM the scheduling parameters for TARGET_THREAD. */
extern int pthread_getschedparam __P ((pthread_t __target_thread,
				       int *__policy,
				       struct sched_param *__param));

#ifdef __USE_UNIX98
/* Determine  level of concurrency.  */
extern int __pthread_getconcurrency __P ((void));
extern int pthread_getconcurrency __P ((void));

/* Set new concurrency level to LEVEL.  */
extern int __pthread_setconcurrency __P ((int __level));
extern int pthread_setconcurrency __P ((int __level));
#endif

/* Functions for mutex handling. */

/* Initialize MUTEX using attributes in *MUTEX_ATTR, or use the
   default values if later is NULL.  */
extern int __pthread_mutex_init __P ((pthread_mutex_t *__mutex,
				   __const pthread_mutexattr_t *__mutex_attr));
extern int pthread_mutex_init __P ((pthread_mutex_t *__mutex,
				   __const pthread_mutexattr_t *__mutex_attr));

/* Destroy MUTEX.  */
extern int __pthread_mutex_destroy __P ((pthread_mutex_t *__mutex));
extern int pthread_mutex_destroy __P ((pthread_mutex_t *__mutex));

/* Try to lock MUTEX.  */
extern int __pthread_mutex_trylock __P ((pthread_mutex_t *__mutex));
extern int pthread_mutex_trylock __P ((pthread_mutex_t *__mutex));

/* Wait until lock for MUTEX becomes available and lock it.  */
extern int __pthread_mutex_lock __P ((pthread_mutex_t *__mutex));
extern int pthread_mutex_lock __P ((pthread_mutex_t *__mutex));

/* Unlock MUTEX.  */
extern int __pthread_mutex_unlock __P ((pthread_mutex_t *__mutex));
extern int pthread_mutex_unlock __P ((pthread_mutex_t *__mutex));


/* Functions for handling mutex attributes.  */

/* Initialize mutex attribute object ATTR with default attributes
   (kind is PTHREAD_MUTEX_FAST_NP).  */
extern int __pthread_mutexattr_init __P ((pthread_mutexattr_t *__attr));
extern int pthread_mutexattr_init __P ((pthread_mutexattr_t *__attr));

/* Destroy mutex attribute object ATTR.  */
extern int __pthread_mutexattr_destroy __P ((pthread_mutexattr_t *__attr));
extern int pthread_mutexattr_destroy __P ((pthread_mutexattr_t *__attr));

/* Set the mutex kind attribute in *ATTR to KIND (either PTHREAD_MUTEX_FAST_NP
   or PTHREAD_MUTEX_RECURSIVE_NP). */
extern int __pthread_mutexattr_setkind_np __P ((pthread_mutexattr_t *__attr,
						int __kind));
extern int pthread_mutexattr_setkind_np __P ((pthread_mutexattr_t *__attr,
					      int __kind));
/* Return in *KIND the mutex kind attribute in *ATTR. */
extern int pthread_mutexattr_getkind_np __P ((__const pthread_mutexattr_t *__attr,
                                              int *__kind));


/* Functions for handling conditional variables.  */

/* Initialize condition variable COND using attributes ATTR, or use
   the default values if later is NULL.  */
extern int pthread_cond_init __P ((pthread_cond_t *__cond,
				   __const pthread_condattr_t *__cond_attr));

/* Destroy condition variable COND.  */
extern int pthread_cond_destroy __P ((pthread_cond_t *__cond));

/* Wake up one thread waiting for condition variable COND.  */
extern int pthread_cond_signal __P ((pthread_cond_t *__cond));

/* Wake up all threads waiting for condition variables COND.  */
extern int pthread_cond_broadcast __P ((pthread_cond_t *__cond));

/* Wait for condition variable COND to be signaled or broadcast.
   MUTEX is assumed to be locked before.  */
extern int pthread_cond_wait __P ((pthread_cond_t *__cond,
				   pthread_mutex_t *__mutex));

/* Wait for condition variable COND to be signaled or broadcast until
   ABSTIME.  MUTEX is assumed to be locked before.  ABSTIME is an
   absolute time specification; zero is the beginning of the epoch
   (00:00:00 GMT, January 1, 1970). */
extern int pthread_cond_timedwait __P ((pthread_cond_t *__cond,
					pthread_mutex_t *__mutex,
					__const struct timespec *__abstime));

/* Functions for handling condition variable attributes.  */

/* Initialize condition variable attribute ATTR.  */
extern int pthread_condattr_init __P ((pthread_condattr_t *__attr));

/* Destroy condition variable attribute ATTR.  */
extern int pthread_condattr_destroy __P ((pthread_condattr_t *__attr));


#ifdef __USE_UNIX98
/* Functions for handling read-write locks.  */

/* Initialize read-write lock RWLOCK using attributes ATTR, or use
   the default values if later is NULL.  */
extern int pthread_rwlock_init __P ((pthread_rwlock_t *__rwlock,
				     __const pthread_rwlockattr_t *__attr));

/* Destroy read-write lock RWLOCK.  */
extern int pthread_rwlock_destroy __P ((pthread_rwlock_t *__rwlock));

/* Acquire read lock for RWLOCK.  */
extern int pthread_rwlock_rdlock __P ((pthread_rwlock_t *__rwlock));

/* Try to acquire read lock for RWLOCK.  */
extern int pthread_rwlock_tryrdlock __P ((pthread_rwlock_t *__rwlock));

/* Acquire write lock for RWLOCK.  */
extern int pthread_rwlock_wrlock __P ((pthread_rwlock_t *__rwlock));

/* Try to acquire writelock for RWLOCK.  */
extern int pthread_rwlock_trywrlock __P ((pthread_rwlock_t *__rwlock));

/* Unlock RWLOCK.  */
extern int pthread_rwlock_unlock __P ((pthread_rwlock_t *__rwlock));


/* Functions for handling read-write lock attributes.  */

/* Initialize attribute object ATTR with default values.  */
extern int pthread_rwlockattr_init __P ((pthread_rwlockattr_t *__attr));

/* Destroy attribute object ATTR.  */
extern int pthread_rwlockattr_destroy __P ((pthread_rwlockattr_t *__attr));

/* Return current setting of process-shared attribute of ATTR in PSHARED.  */
extern int pthread_rwlockattr_getpshared __P ((__const
					       pthread_rwlockattr_t *__attr,
					       int *__pshared));

/* Set process-shared attribute of ATTR to PSHARED.  */
extern int pthread_rwlockattr_setpshared __P ((pthread_rwlockattr_t *__attr,
					       int __pshared));

/* Return current setting of reader/writer preference.  */
extern int pthread_rwlockattr_getkind_np __P ((__const
					       pthread_rwlockattr_t *__attr,
					       int *__pref));

/* Set reader/write preference.  */
extern int pthread_rwlockattr_setkind_np __P ((pthread_rwlockattr_t *__attr,
					       int __pref));
#endif


/* Functions for handling thread-specific data */

/* Create a key value identifying a location in the thread-specific data
   area.  Each thread maintains a distinct thread-specific data area.
   DESTR_FUNCTION, if non-NULL, is called with
   the value associated to that key when the key is destroyed.
   DESTR_FUNCTION is not called if the value associated is NULL
   when the key is destroyed. */
extern int __pthread_key_create __P ((pthread_key_t *__key,
				      void (*__destr_function) (void *)));
extern int pthread_key_create __P ((pthread_key_t *__key,
				    void (*__destr_function) (void *)));

/* Destroy KEY.  */
extern int pthread_key_delete __P ((pthread_key_t __key));

/* Store POINTER in the thread-specific data slot identified by KEY. */
extern int __pthread_setspecific __P ((pthread_key_t __key,
				       __const void *__pointer));
extern int pthread_setspecific __P ((pthread_key_t __key,
				     __const void *__pointer));

/* Return current value of the thread-specific data slot identified by KEY.  */
extern void *__pthread_getspecific __P ((pthread_key_t __key));
extern void *pthread_getspecific __P ((pthread_key_t __key));


/* Functions for handling initialization */

/* Guarantee that the initialization function INIT_ROUTINE will be called
   only once, even if pthread_once is executed several times with the
   same ONCE_CONTROL argument. ONCE_CONTROL must point to a static or
   extern variable initialized to PTHREAD_ONCE_INIT. */
extern int __pthread_once __P ((pthread_once_t *__once_control,
				void (*__init_routine) (void)));
extern int pthread_once __P ((pthread_once_t *__once_control,
			      void (*__init_routine) (void)));


/* Functions for handling cancellation. */

/* Set cancelability state of current thread to STATE, returning old
   state in *OLDSTATE if OLDSTATE is not NULL.  */
extern int pthread_setcancelstate __P ((int __state, int *__oldstate));

/* Set cancellation state of current thread to TYPE, returning the old
   type in *OLDTYPE if OLDTYPE is not NULL.  */
extern int __pthread_setcanceltype __P ((int __type, int *__oldtype));
extern int pthread_setcanceltype __P ((int __type, int *__oldtype));

/* Cancel THREAD immediately or at the next possibility.  */
extern int pthread_cancel __P ((pthread_t __thread));

/* Test for pending cancellation for the current thread and terminate
   the thread as per pthread_exit(PTHREAD_CANCELED) if it has been
   cancelled. */
extern void pthread_testcancel __P ((void));


/* Install a cleanup handler: ROUTINE will be called with arguments ARG
   when the thread is cancelled or calls pthread_exit.  ROUTINE will also
   be called with arguments ARG when the matching pthread_cleanup_pop
   is executed with non-zero EXECUTE argument.
   pthread_cleanup_push and pthread_cleanup_pop are macros and must always
   be used in matching pairs at the same nesting level of braces. */

#define pthread_cleanup_push(routine,arg)				      \
  { struct _pthread_cleanup_buffer _buffer;				      \
    _pthread_cleanup_push (&_buffer, (routine), (arg));

extern void _pthread_cleanup_push __P ((struct _pthread_cleanup_buffer *__buffer,
					void (*__routine) (void *),
					void *__arg));

/* Remove a cleanup handler installed by the matching pthread_cleanup_push.
   If EXECUTE is non-zero, the handler function is called. */

#define pthread_cleanup_pop(execute)					      \
    _pthread_cleanup_pop (&_buffer, (execute)); }

extern void _pthread_cleanup_pop __P ((struct _pthread_cleanup_buffer *__buffer,
				       int __execute));

/* Install a cleanup handler as pthread_cleanup_push does, but also
   saves the current cancellation type and set it to deferred cancellation. */

#define pthread_cleanup_push_defer_np(routine,arg)			      \
  { struct _pthread_cleanup_buffer _buffer;				      \
    _pthread_cleanup_push_defer (&_buffer, (routine), (arg));

extern void _pthread_cleanup_push_defer __P ((struct _pthread_cleanup_buffer *__buffer,
					      void (*__routine) (void *),
					      void *__arg));

/* Remove a cleanup handler as pthread_cleanup_pop does, but also
   restores the cancellation type that was in effect when the matching
   pthread_cleanup_push_defer was called. */

#define pthread_cleanup_pop_restore_np(execute)				      \
  _pthread_cleanup_pop_restore (&_buffer, (execute)); }

extern void _pthread_cleanup_pop_restore __P ((struct _pthread_cleanup_buffer *__buffer,
					       int __execute));

/* Functions for handling signals. */

/* Modify the signal mask for the calling thread.  The arguments have
   the same meaning as for sigprocmask(2). */

extern int pthread_sigmask __P ((int __how, __const sigset_t *__newmask,
				 sigset_t *__oldmask));

/* Send signal SIGNO to the given thread. */

extern int pthread_kill __P ((pthread_t __thread, int __signo));


/* Functions for handling process creation and process execution. */

/* Install handlers to be called when a new process is created with FORK.
   The PREPARE handler is called in the parent process just before performing
   FORK. The PARENT handler is called in the parent process just after FORK.
   The CHILD handler is called in the child process.  Each of the three
   handlers can be NULL, meaning that no handler needs to be called at that
   point.
   PTHREAD_ATFORK can be called several times, in which case the PREPARE
   handlers are called in LIFO order (last added with PTHREAD_ATFORK,
   first called before FORK), and the PARENT and CHILD handlers are called
   in FIFO (first added, first called). */

extern int __pthread_atfork __P ((void (*__prepare) (void),
				  void (*__parent) (void),
				  void (*__child) (void)));
extern int pthread_atfork __P ((void (*__prepare) (void),
				void (*__parent) (void),
				void (*__child) (void)));

/* Terminate all threads in the program except the calling process.
   Should be called just before invoking one of the exec*() functions. */

extern void __pthread_kill_other_threads_np __P ((void));
extern void pthread_kill_other_threads_np __P ((void));


/* This function is called to initialize the pthread library. */
extern void __pthread_initialize __P ((void));

__END_DECLS

#endif	/* pthread.h */
