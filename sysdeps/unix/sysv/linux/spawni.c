/* POSIX spawn interface.  Linux version.
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

#include <spawn.h>
#include <assert.h>
#include <fcntl.h>
#include <paths.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/param.h>
#include <sys/mman.h>
#include <not-cancel.h>
#include <local-setxid.h>
#include <shlib-compat.h>
#include <nptl/pthreadP.h>
#include <dl-sysdep.h>
#include <ldsodefs.h>
#include <libc-internal.h>
#include "spawn_int.h"

/* The Linux implementation of posix_spawn{p} uses the clone syscall directly
   with CLONE_VM and CLONE_VFORK flags and an allocated stack.  The new stack
   and start function solves most the vfork limitation (possible parent
   clobber due stack spilling). The remaining issue are:

   1. That no signal handlers must run in child context, to avoid corrupting
      parent's state.
   2. The parent must ensure child's stack freeing.
   3. Child must synchronize with parent to enforce 2. and to possible
      return execv issues.

   The first issue is solved by blocking all signals in child, even
   the NPTL-internal ones (SIGCANCEL and SIGSETXID).  The second and
   third issue is done by a stack allocation in parent, and by using a
   field in struct spawn_args where the child can write an error
   code. CLONE_VFORK ensures that the parent does not run until the
   child has either exec'ed successfully or exited.  */


/* The Unix standard contains a long explanation of the way to signal
   an error after the fork() was successful.  Since no new wait status
   was wanted there is no way to signal an error using one of the
   available methods.  The committee chose to signal an error by a
   normal program exit with the exit code 127.  */
#define SPAWN_ERROR	127

#ifdef __ia64__
# define CLONE(__fn, __stack, __stacksize, __flags, __args) \
  __clone2 (__fn, __stack, __stacksize, __flags, __args, 0, 0, 0)
#else
# define CLONE(__fn, __stack, __stacksize, __flags, __args) \
  __clone (__fn, __stack, __flags, __args)
#endif

#if _STACK_GROWS_DOWN
# define STACK(__stack, __stack_size) (__stack + __stack_size)
#elif _STACK_GROWS_UP
# define STACK(__stack, __stack_size) (__stack)
#endif


struct posix_spawn_args
{
  sigset_t oldmask;
  const char *file;
  int (*exec) (const char *, char *const *, char *const *);
  const posix_spawn_file_actions_t *fa;
  const posix_spawnattr_t *restrict attr;
  char *const *argv;
  ptrdiff_t argc;
  char *const *envp;
  int xflags;
  int err;
};

/* Older version requires that shell script without shebang definition
   to be called explicitly using /bin/sh (_PATH_BSHELL).  */
static void
maybe_script_execute (struct posix_spawn_args *args)
{
  if (SHLIB_COMPAT (libc, GLIBC_2_2, GLIBC_2_15)
      && (args->xflags & SPAWN_XFLAGS_TRY_SHELL) && errno == ENOEXEC)
    {
      char *const *argv = args->argv;
      ptrdiff_t argc = args->argc;

      /* Construct an argument list for the shell.  */
      char *new_argv[argc + 1];
      new_argv[0] = (char *) _PATH_BSHELL;
      new_argv[1] = (char *) args->file;
      if (argc > 1)
	memcpy (new_argv + 2, argv + 1, argc * sizeof(char *));
      else
	new_argv[2] = NULL;

      /* Execute the shell.  */
      args->exec (new_argv[0], new_argv, args->envp);
    }
}

/* Function used in the clone call to setup the signals mask, posix_spawn
   attributes, and file actions.  It run on its own stack (provided by the
   posix_spawn call).  */
static int
__spawni_child (void *arguments)
{
  struct posix_spawn_args *args = arguments;
  const posix_spawnattr_t *restrict attr = args->attr;
  const posix_spawn_file_actions_t *file_actions = args->fa;
  int ret;

  /* The child must ensure that no signal handler are enabled because it shared
     memory with parent, so the signal disposition must be either SIG_DFL or
     SIG_IGN.  It does by iterating over all signals and although it could
     possibly be more optimized (by tracking which signal potentially have a
     signal handler), it might requires system specific solutions (since the
     sigset_t data type can be very different on different architectures).  */
  struct sigaction sa;
  memset (&sa, '\0', sizeof (sa));

  sigset_t hset;
  __sigprocmask (SIG_BLOCK, 0, &hset);
  for (int sig = 1; sig < _NSIG; ++sig)
    {
      if ((attr->__flags & POSIX_SPAWN_SETSIGDEF)
	  && sigismember (&attr->__sd, sig))
	{
	  sa.sa_handler = SIG_DFL;
	}
      else if (sigismember (&hset, sig))
	{
	  if (__nptl_is_internal_signal (sig))
	    sa.sa_handler = SIG_IGN;
	  else
	    {
	      __libc_sigaction (sig, 0, &sa);
	      if (sa.sa_handler == SIG_IGN)
		continue;
	      sa.sa_handler = SIG_DFL;
	    }
	}
      else
	continue;

      __libc_sigaction (sig, &sa, 0);
    }

#ifdef _POSIX_PRIORITY_SCHEDULING
  /* Set the scheduling algorithm and parameters.  */
  if ((attr->__flags & (POSIX_SPAWN_SETSCHEDPARAM | POSIX_SPAWN_SETSCHEDULER))
      == POSIX_SPAWN_SETSCHEDPARAM)
    {
      if ((ret = __sched_setparam (0, &attr->__sp)) == -1)
	goto fail;
    }
  else if ((attr->__flags & POSIX_SPAWN_SETSCHEDULER) != 0)
    {
      if ((ret = __sched_setscheduler (0, attr->__policy, &attr->__sp)) == -1)
	goto fail;
    }
#endif

  /* Set the process group ID.  */
  if ((attr->__flags & POSIX_SPAWN_SETPGROUP) != 0
      && (ret = __setpgid (0, attr->__pgrp)) != 0)
    goto fail;

  /* Set the effective user and group IDs.  */
  if ((attr->__flags & POSIX_SPAWN_RESETIDS) != 0
      && ((ret = local_seteuid (__getuid ())) != 0
	  || (ret = local_setegid (__getgid ())) != 0))
    goto fail;

  /* Execute the file actions.  */
  if (file_actions != 0)
    {
      int cnt;
      struct rlimit64 fdlimit;
      bool have_fdlimit = false;

      for (cnt = 0; cnt < file_actions->__used; ++cnt)
	{
	  struct __spawn_action *action = &file_actions->__actions[cnt];

	  switch (action->tag)
	    {
	    case spawn_do_close:
	      if ((ret =
		   close_not_cancel (action->action.close_action.fd)) != 0)
		{
		  if (!have_fdlimit)
		    {
		      __getrlimit64 (RLIMIT_NOFILE, &fdlimit);
		      have_fdlimit = true;
		    }

		  /* Signal errors only for file descriptors out of range.  */
		  if (action->action.close_action.fd < 0
		      || action->action.close_action.fd >= fdlimit.rlim_cur)
		    goto fail;
		}
	      break;

	    case spawn_do_open:
	      {
		/* POSIX states that if fildes was already an open file descriptor,
		   it shall be closed before the new file is opened.  This avoid
		   pontential issues when posix_spawn plus addopen action is called
		   with the process already at maximum number of file descriptor
		   opened and also for multiple actions on single-open special
		   paths (like /dev/watchdog).  */
		close_not_cancel (action->action.open_action.fd);

		ret = open_not_cancel (action->action.open_action.path,
				       action->action.
				       open_action.oflag | O_LARGEFILE,
				       action->action.open_action.mode);

		if (ret == -1)
		  goto fail;

		int new_fd = ret;

		/* Make sure the desired file descriptor is used.  */
		if (ret != action->action.open_action.fd)
		  {
		    if ((ret = __dup2 (new_fd, action->action.open_action.fd))
			!= action->action.open_action.fd)
		      goto fail;

		    if ((ret = close_not_cancel (new_fd)) != 0)
		      goto fail;
		  }
	      }
	      break;

	    case spawn_do_dup2:
	      if ((ret = __dup2 (action->action.dup2_action.fd,
				 action->action.dup2_action.newfd))
		  != action->action.dup2_action.newfd)
		goto fail;
	      break;
	    }
	}
    }

  /* Set the initial signal mask of the child if POSIX_SPAWN_SETSIGMASK
     is set, otherwise restore the previous one.  */
  __sigprocmask (SIG_SETMASK, (attr->__flags & POSIX_SPAWN_SETSIGMASK)
		 ? &attr->__ss : &args->oldmask, 0);

  args->err = 0;
  args->exec (args->file, args->argv, args->envp);

  /* This is compatibility function required to enable posix_spawn run
     script without shebang definition for older posix_spawn versions
     (2.15).  */
  maybe_script_execute (args);

fail:
  /* errno should have an appropriate non-zero value; otherwise,
     there's a bug in glibc or the kernel.  For lack of an error code
     (EINTERNALBUG) describing that, use ECHILD.  Another option would
     be to set args->err to some negative sentinel and have the parent
     abort(), but that seems needlessly harsh.  */
  args->err = errno ? : ECHILD;
  _exit (SPAWN_ERROR);
}

/* Spawn a new process executing PATH with the attributes describes in *ATTRP.
   Before running the process perform the actions described in FILE-ACTIONS. */
static int
__spawnix (pid_t * pid, const char *file,
	   const posix_spawn_file_actions_t * file_actions,
	   const posix_spawnattr_t * attrp, char *const argv[],
	   char *const envp[], int xflags,
	   int (*exec) (const char *, char *const *, char *const *))
{
  pid_t new_pid;
  struct posix_spawn_args args;
  int ec;

  /* To avoid imposing hard limits on posix_spawn{p} the total number of
     arguments is first calculated to allocate a mmap to hold all possible
     values.  */
  ptrdiff_t argc = 0;
  /* Linux allows at most max (0x7FFFFFFF, 1/4 stack size) arguments
     to be used in a execve call.  We limit to INT_MAX minus one due the
     compatiblity code that may execute a shell script (maybe_script_execute)
     where it will construct another argument list with an additional
     argument.  */
  ptrdiff_t limit = INT_MAX - 1;
  while (argv[argc++] != NULL)
    if (argc == limit)
      {
	errno = E2BIG;
	return errno;
      }

  int prot = (PROT_READ | PROT_WRITE
	     | ((GL (dl_stack_flags) & PF_X) ? PROT_EXEC : 0));

  /* Add a slack area for child's stack.  */
  size_t argv_size = (argc * sizeof (void *)) + 512;
  size_t stack_size = ALIGN_UP (argv_size, GLRO(dl_pagesize));
  void *stack = __mmap (NULL, stack_size, prot,
			MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
  if (__glibc_unlikely (stack == MAP_FAILED))
    return errno;

  /* Disable asynchronous cancellation.  */
  int state;
  __libc_ptf_call (__pthread_setcancelstate,
                   (PTHREAD_CANCEL_DISABLE, &state), 0);

  /* Child must set args.err to something non-negative - we rely on
     the parent and child sharing VM.  */
  args.err = -1;
  args.file = file;
  args.exec = exec;
  args.fa = file_actions;
  args.attr = attrp ? attrp : &(const posix_spawnattr_t) { 0 };
  args.argv = argv;
  args.argc = argc;
  args.envp = envp;
  args.xflags = xflags;

  __libc_signal_block_all (&args.oldmask);

  /* The clone flags used will create a new child that will run in the same
     memory space (CLONE_VM) and the execution of calling thread will be
     suspend until the child calls execve or _exit.

     Also since the calling thread execution will be suspend, there is not
     need for CLONE_SETTLS.  Although parent and child share the same TLS
     namespace, there will be no concurrent access for TLS variables (errno
     for instance).  */
  new_pid = CLONE (__spawni_child, STACK (stack, stack_size), stack_size,
		   CLONE_VM | CLONE_VFORK | SIGCHLD, &args);

  if (new_pid > 0)
    {
      ec = args.err;
      assert (ec >= 0);
      if (ec != 0)
	  __waitpid (new_pid, NULL, 0);
    }
  else
    ec = -new_pid;

  __munmap (stack, stack_size);

  if ((ec == 0) && (pid != NULL))
    *pid = new_pid;

  __libc_signal_restore_set (&args.oldmask);

  __libc_ptf_call (__pthread_setcancelstate, (state, NULL), 0);

  return ec;
}

/* Spawn a new process executing PATH with the attributes describes in *ATTRP.
   Before running the process perform the actions described in FILE-ACTIONS. */
int
__spawni (pid_t * pid, const char *file,
	  const posix_spawn_file_actions_t * acts,
	  const posix_spawnattr_t * attrp, char *const argv[],
	  char *const envp[], int xflags)
{
  return __spawnix (pid, file, acts, attrp, argv, envp, xflags,
		    xflags & SPAWN_XFLAGS_USE_PATH ? __execvpe : __execve);
}
