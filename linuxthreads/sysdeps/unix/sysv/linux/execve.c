/* Copyright (C) 1999, 2000, 2002 Free Software Foundation, Inc.
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

#include <errno.h>
#include <unistd.h>

#include <sysdep.h>
#include <alloca.h>
#include <sys/syscall.h>
#include <bp-checks.h>

extern int __syscall_execve (const char *__unbounded file,
			     char *__unbounded const *__unbounded argv,
			     char *__unbounded const *__unbounded envp);
extern void __pthread_kill_other_threads_np (void);
weak_extern (__pthread_kill_other_threads_np)


int
__execve (file, argv, envp)
     const char *file;
     char *const argv[];
     char *const envp[];
{
  /* If this is a threaded application kill all other threads.  */
  if (__pthread_kill_other_threads_np)
    __pthread_kill_other_threads_np ();
#if __BOUNDED_POINTERS__
  {
    char *const *v;
    int i;
    char *__unbounded *__unbounded ubp_argv;
    char *__unbounded *__unbounded ubp_envp;
    char *__unbounded *__unbounded ubp_v;

    for (v = argv; *v; v++)
      ;
    i = v - argv + 1;
    ubp_argv = (char *__unbounded *__unbounded) alloca (sizeof (*ubp_argv) * i);
    for (v = argv, ubp_v = ubp_argv; --i; v++, ubp_v++)
      *ubp_v = CHECK_STRING (*v);
    *ubp_v = 0;

    for (v = envp; *v; v++)
      ;
    i = v - envp + 1;
    ubp_envp = (char *__unbounded *__unbounded) alloca (sizeof (*ubp_envp) * i);
    for (v = envp, ubp_v = ubp_envp; --i; v++, ubp_v++)
      *ubp_v = CHECK_STRING (*v);
    *ubp_v = 0;

    return INLINE_SYSCALL (execve, 3, CHECK_STRING (file), ubp_argv, ubp_envp);
  }
#else
  return INLINE_SYSCALL (execve, 3, file, argv, envp);
#endif
}
weak_alias (__execve, execve)
