/* Operating system support for run-time dynamic linker.  Generic Unix version.
   Copyright (C) 1995,1996,1997,1998,2000,2001 Free Software Foundation, Inc.
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

#include <assert.h>
#include <elf.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <ldsodefs.h>
#include <stdio-common/_itoa.h>
#include <fpu_control.h>

#include <entry.h>
#include <dl-machine.h>
#include <dl-procinfo.h>
#include <dl-osinfo.h>

extern int _dl_argc;
extern char **_dl_argv;
extern char **_environ;
extern size_t _dl_pagesize;
extern int _dl_clktck;
extern const char *_dl_platform;
extern unsigned long int _dl_hwcap;
extern size_t _dl_platformlen;
extern fpu_control_t _dl_fpu_control;
extern void _end;

/* Protect SUID program against misuse of file descriptors.  */
extern void __libc_check_standard_fds (void);

#ifdef NEED_DL_BASE_ADDR
ElfW(Addr) _dl_base_addr;
#endif
int __libc_enable_secure;
int __libc_multiple_libcs = 0;	/* Defining this here avoids the inclusion
				   of init-first.  */
/* This variable contains the lowest stack address ever used.  */
void *__libc_stack_end;
static ElfW(auxv_t) *_dl_auxv;
unsigned long int _dl_hwcap_mask = HWCAP_IMPORTANT;


#ifndef DL_FIND_ARG_COMPONENTS
# define DL_FIND_ARG_COMPONENTS(cookie, argc, argv, envp, auxp)	\
  do {									      \
    void **_tmp;							      \
    (argc) = *(long int *) cookie;					      \
    (argv) = (char **) ((long int *) cookie + 1);			      \
    (envp) = (argv) + (argc) + 1;					      \
    for (_tmp = (void **) (envp); *_tmp; ++_tmp)			      \
      continue;								      \
    (auxp) = (void *) ++_tmp;						      \
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
#ifdef HAVE_AUX_XID
# define set_seen(tag) (tag) /* Evaluate for the side effects.  */
#else
  unsigned int seen = 0;
# define M(type) (1 << (type))
# define set_seen(tag) seen |= M ((tag)->a_type)
#endif

  DL_FIND_ARG_COMPONENTS (start_argptr, _dl_argc, _dl_argv, _environ,
			  _dl_auxv);

  user_entry = (ElfW(Addr)) ENTRY_POINT;
  _dl_platform = NULL; /* Default to nothing known about the platform.  */

  for (av = _dl_auxv; av->a_type != AT_NULL; set_seen (av++))
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
#ifdef NEED_DL_BASE_ADDR
      case AT_BASE:
	_dl_base_addr = av->a_un.a_val;
	break;
#endif
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
      case AT_CLKTCK:
	_dl_clktck = av->a_un.a_val;
	break;
      case AT_FPUCW:
	_dl_fpu_control = av->a_un.a_val;
	break;
      }

#ifdef DL_SYSDEP_OSCHECK
  DL_SYSDEP_OSCHECK (dl_fatal);
#endif

  /* Linux doesn't provide us with any of these values on the stack
     when the dynamic linker is run directly as a program.  */

#ifndef HAVE_AUX_XID
# define SEE(UID, uid) if ((seen & M (AT_##UID)) == 0) uid = __get##uid ()
  SEE (UID, uid);
  SEE (GID, gid);
  SEE (EUID, euid);
  SEE (EGID, egid);
#endif

  __libc_enable_secure = uid != euid || gid != egid;

#ifndef HAVE_AUX_PAGESIZE
  if (_dl_pagesize == 0)
    _dl_pagesize = __getpagesize ();
#endif

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

  /* If this is a SUID program we make sure that FDs 0, 1, and 2 are
     allocated.  If necessary we are doing it ourself.  If it is not
     possible we stop the program.  */
  if (__builtin_expect (__libc_enable_secure, 0))
    __libc_check_standard_fds ();

  (*dl_main) (phdr, phnum, &user_entry);
  return user_entry;
}

void
internal_function
_dl_sysdep_start_cleanup (void)
{
}

void
internal_function
_dl_show_auxv (void)
{
  char buf[64];
  ElfW(auxv_t) *av;

  /* Terminate string.  */
  buf[63] = '\0';

  /* The following code assumes that the AT_* values are encoded
  starting from 0 with AT_NULL, 1 for AT_IGNORE, and all other values
  close by (otherwise the array will be too large).  In case we have
  to support a platform where these requirements are not fulfilled
  some alternative implementation has to be used.  */
  for (av = _dl_auxv; av->a_type != AT_NULL; ++av)
    {
      static const struct
      {
	const char label[20];
	enum { dec, hex, str } form;
      } auxvars[] =
	{
	  [AT_EXECFD - 2] =		{ "AT_EXECFD:      ", dec },
	  [AT_PHDR - 2] =		{ "AT_PHDR:        0x", hex },
	  [AT_PHENT - 2] =		{ "AT_PHENT:       ", dec },
	  [AT_PHNUM - 2] =		{ "AT_PHNUM:       ", dec },
	  [AT_PAGESZ - 2] =		{ "AT_PAGESZ:      ", dec },
	  [AT_BASE - 2] =		{ "AT_BASE:        0x", hex },
	  [AT_FLAGS - 2] =		{ "AT_FLAGS:       0x", hex },
	  [AT_ENTRY - 2] =		{ "AT_ENTRY:       0x", hex },
	  [AT_NOTELF - 2] =		{ "AT_NOTELF:      ", hex },
	  [AT_UID - 2] =		{ "AT_UID:         ", dec },
	  [AT_EUID - 2] =		{ "AT_EUID:        ", dec },
	  [AT_GID - 2] =		{ "AT_GID:         ", dec },
	  [AT_EGID - 2] =		{ "AT_EGID:        ", dec },
	  [AT_PLATFORM - 2] =		{ "AT_PLATFORM:    ", str },
	  [AT_HWCAP - 2] =		{ "AT_HWCAP:       ", hex },
	  [AT_CLKTCK - 2] =		{ "AT_CLKTCK:      ", dec },
	  [AT_FPUCW - 2] =		{ "AT_FPUCW:       ", hex },
	  [AT_DCACHEBSIZE - 2] =	{ "AT_DCACHEBSIZE: 0x", hex },
	  [AT_ICACHEBSIZE - 2] =	{ "AT_ICACHEBSIZE: 0x", hex },
	  [AT_UCACHEBSIZE - 2] =	{ "AT_UCACHEBSIZE: 0x", hex }
	};
      unsigned int idx = (unsigned int) (av->a_type - 2);

      assert (AT_NULL == 0);
      assert (AT_IGNORE == 1);
      if (idx < sizeof (auxvars) / sizeof (auxvars[0]))
	{
	  if (av->a_type != AT_HWCAP || _dl_procinfo (av->a_un.a_val) < 0)
	    {
	      const char *val = av->a_un.a_ptr;

	      if (__builtin_expect (auxvars[idx].form, dec) == dec)
		val = _itoa_word (av->a_un.a_val, buf + sizeof buf - 1, 10, 0);
	      else if (__builtin_expect (auxvars[idx].form, hex) == hex)
		val = _itoa_word (av->a_un.a_val, buf + sizeof buf - 1, 16, 0);

	      _dl_printf ("%s%s\n", auxvars[idx].label, val);
	    }
	}
    }
}


/* Return an array of useful/necessary hardware capability names.  */
const struct r_strlenpair *
internal_function
_dl_important_hwcaps (const char *platform, size_t platform_len, size_t *sz,
		      size_t *max_capstrlen)
{
  /* Determine how many important bits are set.  */
  unsigned long int masked = _dl_hwcap & _dl_hwcap_mask;
  size_t cnt = platform != NULL;
  size_t n, m;
  size_t total;
  struct r_strlenpair *temp;
  struct r_strlenpair *result;
  struct r_strlenpair *rp;
  char *cp;

  /* Count the number of bits set in the masked value.  */
  for (n = 0; (~((1UL << n) - 1) & masked) != 0; ++n)
    if ((masked & (1UL << n)) != 0)
      ++cnt;

  if (cnt == 0)
    {
      /* If we have platform name and no important capability we only have
	 the base directory to search.  */
      result = (struct r_strlenpair *) malloc (sizeof (*result));
      if (result == NULL)
	{
	no_memory:
	  _dl_signal_error (ENOMEM, NULL, "cannot create capability list");
	}

      result[0].str = (char *) result;	/* Does not really matter.  */
      result[0].len = 0;

      *sz = 1;
      return result;
    }

  /* Create temporary data structure to generate result table.  */
  temp = (struct r_strlenpair *) alloca (cnt * sizeof (*temp));
  m = 0;
  for (n = 0; masked != 0; ++n)
    if ((masked & (1UL << n)) != 0)
      {
	temp[m].str = _dl_hwcap_string (n);
	temp[m].len = strlen (temp[m].str);
	masked ^= 1UL << n;
	++m;
      }
  if (platform != NULL)
    {
      temp[m].str = platform;
      temp[m].len = platform_len;
      ++m;
    }

  /* Determine the total size of all strings together.  */
  if (cnt == 1)
    total = temp[0].len;
  else
    {
      total = (1 << (cnt - 2)) * (temp[0].len + temp[cnt - 1].len + 2);
      for (n = 1; n + 1 < cnt; ++n)
	total += (1 << (cnt - 3)) * (temp[n].len + 1);
    }

  /* The result structure: we use a very compressed way to store the
     various combinations of capability names.  */
  *sz = 1 << cnt;
  result = (struct r_strlenpair *) malloc (*sz * sizeof (*result) + total);
  if (result == NULL)
    goto no_memory;

  if (cnt == 1)
    {
      result[0].str = (char *) (result + *sz);
      result[0].len = temp[0].len + 1;
      result[1].str = (char *) (result + *sz);
      result[1].len = 0;
      cp = __mempcpy ((char *) (result + *sz), temp[0].str, temp[0].len);
      *cp = '/';
      *sz = 2;
      *max_capstrlen = result[0].len;

      return result;
    }

  /* Fill in the information.  This follows the following scheme
     (indeces from TEMP for four strings):
	entry #0: 0, 1, 2, 3	binary: 1111
	      #1: 0, 1, 3	        1101
	      #2: 0, 2, 3	        1011
	      #3: 0, 3		        1001
     This allows the representation of all possible combinations of
     capability names in the string.  First generate the strings.  */
  result[1].str = result[0].str = cp = (char *) (result + *sz);
#define add(idx) \
      cp = __mempcpy (__mempcpy (cp, temp[idx].str, temp[idx].len), "/", 1);
  if (cnt == 2)
    {
      add (1);
      add (0);
    }
  else
    {
      n = 1 << cnt;
      do
	{
	  n -= 2;

	  /* We always add the last string.  */
	  add (cnt - 1);

	  /* Add the strings which have the bit set in N.  */
	  for (m = cnt - 2; m > 0; --m)
	    if ((n & (1 << m)) != 0)
	      add (m);

	  /* Always add the first string.  */
	  add (0);
	}
      while (n != 0);
    }
#undef add

  /* Now we are ready to install the string pointers and length.  */
  for (n = 0; n < (1UL << cnt); ++n)
    result[n].len = 0;
  n = cnt;
  do
    {
      size_t mask = 1 << --n;

      rp = result;
      for (m = 1 << cnt; m > 0; ++rp)
	if ((--m & mask) != 0)
	  rp->len += temp[n].len + 1;
    }
  while (n != 0);

  /* The first half of the strings all include the first string.  */
  n = (1 << cnt) - 2;
  rp = &result[2];
  while (n != (1UL << (cnt - 1)))
    {
      if ((n & 1) != 0)
	rp[0].str = rp[-2].str + rp[-2].len;
      else
	rp[0].str = rp[-1].str;
      ++rp;
      --n;
    }

  /* The second have starts right after the first part of the string of
     corresponding entry in the first half.  */
  do
    {
      rp[0].str = rp[-(1 << (cnt - 1))].str + temp[cnt - 1].len + 1;
      ++rp;
    }
  while (--n != 0);

  /* The maximum string length.  */
  *max_capstrlen = result[0].len;

  return result;
}
