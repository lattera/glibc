/* Copyright (C) 1995-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@gnu.ai.mit.edu>, August 1995.

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

#include <sys/sem.h>
#include <stdarg.h>
#include <ipc_priv.h>
#include <sysdep.h>
#include <shlib-compat.h>
#include <errno.h>

/* Define a `union semun' suitable for Linux here.  */
union semun
{
  int val;			/* value for SETVAL */
  struct semid_ds *buf;		/* buffer for IPC_STAT & IPC_SET */
  unsigned short int *array;	/* array for GETALL & SETALL */
  struct seminfo *__buf;	/* buffer for IPC_INFO */
};

#ifndef DEFAULT_VERSION
# define DEFAULT_VERSION GLIBC_2_2
#endif

int
__new_semctl (int semid, int semnum, int cmd, ...)
{
  union semun arg = { 0 };
  va_list ap;

  /* Get the argument only if required.  */
  switch (cmd)
    {
    case SETVAL:        /* arg.val */
    case GETALL:        /* arg.array */
    case SETALL:
    case IPC_STAT:      /* arg.buf */
    case IPC_SET:
    case SEM_STAT:
    case IPC_INFO:      /* arg.__buf */
    case SEM_INFO:
      va_start (ap, cmd);
      arg = va_arg (ap, union semun);
      va_end (ap);
      break;
    }

#ifdef __ASSUME_DIRECT_SYSVIPC_SYSCALLS
  return INLINE_SYSCALL_CALL (semctl, semid, semnum, cmd | __IPC_64,
			      arg.array);
#else
  return INLINE_SYSCALL_CALL (ipc, IPCOP_semctl, semid, semnum, cmd | __IPC_64,
			      SEMCTL_ARG_ADDRESS (arg));
#endif
}
versioned_symbol (libc, __new_semctl, semctl, DEFAULT_VERSION);


#if SHLIB_COMPAT (libc, GLIBC_2_0, GLIBC_2_2)
/* Since semctl use a variadic argument for semid_ds there is not need to
   define and tie the compatibility symbol to the old 'union semun'
   definition.  */
int
attribute_compat_text_section
__old_semctl (int semid, int semnum, int cmd, ...)
{
  union semun arg = { 0 };
  va_list ap;

  /* Get the argument only if required.  */
  switch (cmd)
    {
    case SETVAL:        /* arg.val */
    case GETALL:        /* arg.array */
    case SETALL:
    case IPC_STAT:      /* arg.buf */
    case IPC_SET:
    case SEM_STAT:
    case IPC_INFO:      /* arg.__buf */
    case SEM_INFO:
      va_start (ap, cmd);
      arg = va_arg (ap, union semun);
      va_end (ap);
      break;
    }

# ifdef __ASSUME_DIRECT_SYSVIPC_SYSCALLS
  return INLINE_SYSCALL_CALL (semctl, semid, semnum, cmd, arg.array);
# else
  return INLINE_SYSCALL_CALL (ipc, IPCOP_semctl, semid, semnum, cmd,
			      SEMCTL_ARG_ADDRESS (arg));
# endif
}
compat_symbol (libc, __old_semctl, semctl, GLIBC_2_0);
#endif
