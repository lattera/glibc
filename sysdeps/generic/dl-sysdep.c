/* Operating system support for run-time dynamic linker.  Generic Unix version.
Copyright (C) 1995 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <elf.h>
#include <sys/types.h>
#include <fcntl.h>
#include <link.h>
#include <unistd.h>

extern int _dl_argc;
extern char **_dl_argv;
extern char **_environ;
extern void _start (void);

Elf32_Addr
_dl_sysdep_start (void **start_argptr,
		  void (*dl_main) (const Elf32_Phdr *phdr, Elf32_Word phnum,
				   Elf32_Addr *user_entry))
{
  const Elf32_Phdr *phdr;
  Elf32_Word phnum;
  Elf32_Addr user_entry;
  Elf32_auxv_t *av;
  uid_t uid, euid;
  gid_t gid, egid;

  user_entry = (Elf32_Addr) &_start;
  _dl_argc = *(int *) start_argptr;
  _dl_argv = start_argptr + 1;
  _environ = &_dl_argv[_dl_argc + 1];
  start_argptr = (void **) _environ;
  while (*start_argptr)
    ++start_argptr;

  for (av = ++start_argptr; av->a_type != AT_NULL; ++av)
    switch (av->a_type)
      {
      case AT_PHDR:
	phdr = av->a_un.a_ptr;
	break;
      case AT_PHNUM:
	phnum = av->a_un.a_val;
	break;
      case AT_ENTRY:
	user_entry = av->a_un.a_val;
	break;
      case AT_UID:
	uid = av->a_un.a_val;
	break;
      case AT_GID:
	gid = av->a_un.a_val;
	break;
      case AT_EUID:
	euid = av->a_un.a_val;
	break;
      case AT_EGID:
	egid = av->a_un.a_val;
	break;
      }

  _dl_secure = uid != euid || gid != egid;

  (*dl_main) (phdr, phnum, &user_entry);
  start_argptr[-1] = (void *) user_entry;
}

int
_dl_sysdep_open_zero_fill (void)
{
  return open ("/dev/zero", O_RDONLY);
}

#include <stdarg.h>

void
_dl_sysdep_fatal (const char *msg, ...)
{
  va_list ap;

  va_start (ap, msg);
  do
    {
      size_t len = strlen (msg);
      write (STDERR_FILENO, msg, len);
      msg = va_arg (ap, const char *);
    } while (msg);
  va_end (ap);

  _exit (127);
}
