/* Operating system support for run-time dynamic linker.  Generic Unix version.
   Copyright (C) 1995, 1996, 1997, 1998 Free Software Foundation, Inc.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <elf.h>
#include <entry.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <link.h>
#include <unistd.h>
#include <stdio-common/_itoa.h>

#include <dl-machine.h>
#include <dl-procinfo.h>

extern int _dl_argc;
extern char **_dl_argv;
extern char **_environ;
extern size_t _dl_pagesize;
extern const char *_dl_platform;
extern unsigned long _dl_hwcap;
extern size_t _dl_platformlen;
extern void _end;
extern void ENTRY_POINT (void);

ElfW(Addr) _dl_base_addr;
int __libc_enable_secure;
int __libc_multiple_libcs;	/* Defining this here avoids the inclusion
				   of init-first.  */
static ElfW(auxv_t) *_dl_auxv;


#ifndef DL_FIND_ARG_COMPONENTS
#define DL_FIND_ARG_COMPONENTS(cookie, argc, argv, envp, auxp)	\
  do {								\
    void **_tmp;						\
    (argc) = *(long *) cookie;					\
    (argv) = (char **) cookie + 1;				\
    (envp) = (argv) + (argc) + 1;				\
    for (_tmp = (void **) (envp); *_tmp; ++_tmp)		\
      continue;							\
    (auxp) = (void *) ++_tmp;					\
  } while (0)
#endif


ElfW(Addr)
_dl_sysdep_start (void **start_argptr,
		  void (*dl_main) (const ElfW(Phdr) *phdr, ElfW(Word) phnum,
				   ElfW(Addr) *user_entry))
{
  const ElfW(Phdr) *phdr = NULL;
  ElfW(Word) phnum = 0;
  ElfW(Addr) user_entry;
  ElfW(auxv_t) *av;
  uid_t uid = 0;
  uid_t euid = 0;
  gid_t gid = 0;
  gid_t egid = 0;
  unsigned int seen;

  DL_FIND_ARG_COMPONENTS (start_argptr, _dl_argc, _dl_argv, _environ, _dl_auxv);

  user_entry = (ElfW(Addr)) &ENTRY_POINT;
  _dl_platform = NULL; /* Default to nothing known about the platform.  */

  seen = 0;
#define M(type) (1 << (type))

  for (av = _dl_auxv; av->a_type != AT_NULL; seen |= M ((++av)->a_type))
    switch (av->a_type)
      {
      case AT_PHDR:
	phdr = av->a_un.a_ptr;
	break;
      case AT_PHNUM:
	phnum = av->a_un.a_val;
	break;
      case AT_PAGESZ:
	_dl_pagesize = av->a_un.a_val;
	break;
      case AT_ENTRY:
	user_entry = av->a_un.a_val;
	break;
      case AT_BASE:
	_dl_base_addr = av->a_un.a_val;
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
      case AT_PLATFORM:
	_dl_platform = av->a_un.a_ptr;
	break;
      case AT_HWCAP:
	_dl_hwcap = av->a_un.a_val;
	break;
      }

  /* Linux doesn't provide us with any of these values on the stack
     when the dynamic linker is run directly as a program.  */

#define SEE(UID, uid) if ((seen & M (AT_##UID)) == 0) uid = __get##uid ()
  SEE (UID, uid);
  SEE (GID, gid);
  SEE (EUID, euid);
  SEE (EGID, egid);

  __libc_enable_secure = uid != euid || gid != egid;

  if (_dl_pagesize == 0)
    _dl_pagesize = __getpagesize ();

#ifdef DL_SYSDEP_INIT
  DL_SYSDEP_INIT;
#endif

#ifdef DL_PLATFORM_INIT
  DL_PLATFORM_INIT;
#endif

  /* Determine the length of the platform name.  */
  if (_dl_platform != NULL)
    _dl_platformlen = strlen (_dl_platform);

  if (__sbrk (0) == &_end)
    /* The dynamic linker was run as a program, and so the initial break
       starts just after our bss, at &_end.  The malloc in dl-minimal.c
       will consume the rest of this page, so tell the kernel to move the
       break up that far.  When the user program examines its break, it
       will see this new value and not clobber our data.  */
    __sbrk (_dl_pagesize - ((&_end - (void *) 0) & (_dl_pagesize - 1)));

  (*dl_main) (phdr, phnum, &user_entry);
  return user_entry;
}

void
_dl_sysdep_start_cleanup (void)
{
}

void
_dl_show_auxv (void)
{
  char buf[64];
  ElfW(auxv_t) *av;

  /* Terminate string.  */
  buf[63] = '\0';

  for (av = _dl_auxv; av->a_type != AT_NULL; ++av)
    switch (av->a_type)
      {
      case AT_PHDR:
	_dl_sysdep_message ("AT_PHDR:     0x",
			    _itoa_word (av->a_un.a_val, buf + sizeof buf - 1,
					16, 0),
			    "\n", NULL);
	break;
      case AT_PHNUM:
	_dl_sysdep_message ("AT_PHNUM:    ",
			    _itoa_word (av->a_un.a_val, buf + sizeof buf - 1,
					10, 0),
			    "\n", NULL);
	break;
      case AT_PAGESZ:
	_dl_sysdep_message ("AT_PAGESZ:   ",
			    _itoa_word (av->a_un.a_val, buf + sizeof buf - 1,
					10, 0),
			    "\n", NULL);
	break;
      case AT_ENTRY:
	_dl_sysdep_message ("AT_ENTRY:    0x",
			    _itoa_word (av->a_un.a_val, buf + sizeof buf - 1,
					16, 0),
			    "\n", NULL);
	break;
      case AT_BASE:
	_dl_sysdep_message ("AT_BASE:     0x",
			    _itoa_word (av->a_un.a_val, buf + sizeof buf - 1,
					16, 0),
			    "\n", NULL);
	break;
      case AT_UID:
	_dl_sysdep_message ("AT_UID:      ",
			    _itoa_word (av->a_un.a_val, buf + sizeof buf - 1,
					10, 0),
			    "\n", NULL);
	break;
      case AT_GID:
	_dl_sysdep_message ("AT_GID:      ",
			    _itoa_word (av->a_un.a_val, buf + sizeof buf - 1,
					10, 0),
			    "\n", NULL);
	break;
      case AT_EUID:
	_dl_sysdep_message ("AT_EUID:     ",
			    _itoa_word (av->a_un.a_val, buf + sizeof buf - 1,
					10, 0),
			    "\n", NULL);
	break;
      case AT_EGID:
	_dl_sysdep_message ("AT_EGID:     ",
			    _itoa_word (av->a_un.a_val, buf + sizeof buf - 1,
					10, 0),
			    "\n", NULL);
	break;
      case AT_PLATFORM:
	_dl_sysdep_message ("AT_PLATFORM: ", av->a_un.a_ptr, "\n", NULL);
	break;
      case AT_HWCAP:
	if (_dl_procinfo (av->a_un.a_val) < 0)
	  _dl_sysdep_message ("AT_HWCAP:    ",
			      _itoa_word (av->a_un.a_val, buf + sizeof buf - 1,
					  16, 0),
			      "\n", NULL);
	break;
      }
}
