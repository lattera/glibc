/* Operating system support for run-time dynamic linker.  Generic Unix version.
   Copyright (C) 1995-1998, 2000-2003, 2004 Free Software Foundation, Inc.
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

#include <assert.h>
#include <elf.h>
#include <errno.h>
#include <fcntl.h>
#include <libintl.h>
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
#include <hp-timing.h>
#include <tls.h>

extern char **_environ attribute_hidden;
extern void _end attribute_hidden;

/* Protect SUID program against misuse of file descriptors.  */
extern void __libc_check_standard_fds (void);

#ifdef NEED_DL_BASE_ADDR
ElfW(Addr) _dl_base_addr;
#endif
int __libc_enable_secure attribute_relro = 0;
INTVARDEF(__libc_enable_secure)
int __libc_multiple_libcs = 0;	/* Defining this here avoids the inclusion
				   of init-first.  */
/* This variable contains the lowest stack address ever used.  */
void *__libc_stack_end attribute_relro = NULL;
rtld_hidden_data_def(__libc_stack_end)
static ElfW(auxv_t) *_dl_auxv attribute_relro;

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

#ifndef DL_STACK_END
# define DL_STACK_END(cookie) ((void *) (cookie))
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
#ifdef HAVE_AUX_SECURE
# define set_seen(tag) (tag)	/* Evaluate for the side effects.  */
# define set_seen_secure() ((void) 0)
#else
  uid_t uid = 0;
  gid_t gid = 0;
  unsigned int seen = 0;
# define set_seen_secure() (seen = -1)
# ifdef HAVE_AUX_XID
#  define set_seen(tag) (tag)	/* Evaluate for the side effects.  */
# else
#  define M(type) (1 << (type))
#  define set_seen(tag) seen |= M ((tag)->a_type)
# endif
#endif
#ifdef NEED_DL_SYSINFO
  uintptr_t new_sysinfo = 0;
#endif

  __libc_stack_end = DL_STACK_END (start_argptr);
  DL_FIND_ARG_COMPONENTS (start_argptr, _dl_argc, INTUSE(_dl_argv), _environ,
			  _dl_auxv);

  user_entry = (ElfW(Addr)) ENTRY_POINT;
  GLRO(dl_platform) = NULL; /* Default to nothing known about the platform.  */

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
	GLRO(dl_pagesize) = av->a_un.a_val;
	break;
      case AT_ENTRY:
	user_entry = av->a_un.a_val;
	break;
#ifdef NEED_DL_BASE_ADDR
      case AT_BASE:
	_dl_base_addr = av->a_un.a_val;
	break;
#endif
#ifndef HAVE_AUX_SECURE
      case AT_UID:
      case AT_EUID:
	uid ^= av->a_un.a_val;
	break;
      case AT_GID:
      case AT_EGID:
	gid ^= av->a_un.a_val;
	break;
#endif
      case AT_SECURE:
#ifndef HAVE_AUX_SECURE
	seen = -1;
#endif
	INTUSE(__libc_enable_secure) = av->a_un.a_val;
	break;
      case AT_PLATFORM:
	GLRO(dl_platform) = av->a_un.a_ptr;
	break;
      case AT_HWCAP:
	GLRO(dl_hwcap) = av->a_un.a_val;
	break;
      case AT_CLKTCK:
	GLRO(dl_clktck) = av->a_un.a_val;
	break;
      case AT_FPUCW:
	GLRO(dl_fpu_control) = av->a_un.a_val;
	break;
#ifdef NEED_DL_SYSINFO
      case AT_SYSINFO:
	new_sysinfo = av->a_un.a_val;
	break;
#endif
#if defined NEED_DL_SYSINFO || defined NEED_DL_SYSINFO_DSO
      case AT_SYSINFO_EHDR:
	GLRO(dl_sysinfo_dso) = av->a_un.a_ptr;
	break;
#endif
#ifdef DL_PLATFORM_AUXV
      DL_PLATFORM_AUXV
#endif
      }

#ifdef DL_SYSDEP_OSCHECK
  DL_SYSDEP_OSCHECK (dl_fatal);
#endif

#ifndef HAVE_AUX_SECURE
  if (seen != -1)
    {
      /* Fill in the values we have not gotten from the kernel through the
	 auxiliary vector.  */
# ifndef HAVE_AUX_XID
#  define SEE(UID, var, uid) \
   if ((seen & M (AT_##UID)) == 0) var ^= __get##uid ()
      SEE (UID, uid, uid);
      SEE (EUID, uid, euid);
      SEE (GID, gid, gid);
      SEE (EGID, gid, egid);
# endif

      /* If one of the two pairs of IDs does not match this is a setuid
	 or setgid run.  */
      INTUSE(__libc_enable_secure) = uid | gid;
    }
#endif

#ifndef HAVE_AUX_PAGESIZE
  if (GLRO(dl_pagesize) == 0)
    GLRO(dl_pagesize) = __getpagesize ();
#endif

#if defined NEED_DL_SYSINFO
  /* Only set the sysinfo value if we also have the vsyscall DSO.  */
  if (GLRO(dl_sysinfo_dso) != 0 && new_sysinfo)
    GLRO(dl_sysinfo) = new_sysinfo;
#endif

#ifdef DL_SYSDEP_INIT
  DL_SYSDEP_INIT;
#endif

#ifdef DL_PLATFORM_INIT
  DL_PLATFORM_INIT;
#endif

  /* Determine the length of the platform name.  */
  if (GLRO(dl_platform) != NULL)
    GLRO(dl_platformlen) = strlen (GLRO(dl_platform));

  if (__sbrk (0) == &_end)
    /* The dynamic linker was run as a program, and so the initial break
       starts just after our bss, at &_end.  The malloc in dl-minimal.c
       will consume the rest of this page, so tell the kernel to move the
       break up that far.  When the user program examines its break, it
       will see this new value and not clobber our data.  */
    __sbrk (GLRO(dl_pagesize)
	    - ((&_end - (void *) 0) & (GLRO(dl_pagesize) - 1)));

  /* If this is a SUID program we make sure that FDs 0, 1, and 2 are
     allocated.  If necessary we are doing it ourself.  If it is not
     possible we stop the program.  */
  if (__builtin_expect (INTUSE(__libc_enable_secure), 0))
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
	enum { unknown = 0, dec, hex, str, ignore } form;
      } auxvars[] =
	{
	  [AT_EXECFD - 2] =		{ "AT_EXECFD:       ", dec },
	  [AT_PHDR - 2] =		{ "AT_PHDR:         0x", hex },
	  [AT_PHENT - 2] =		{ "AT_PHENT:        ", dec },
	  [AT_PHNUM - 2] =		{ "AT_PHNUM:        ", dec },
	  [AT_PAGESZ - 2] =		{ "AT_PAGESZ:       ", dec },
	  [AT_BASE - 2] =		{ "AT_BASE:         0x", hex },
	  [AT_FLAGS - 2] =		{ "AT_FLAGS:        0x", hex },
	  [AT_ENTRY - 2] =		{ "AT_ENTRY:        0x", hex },
	  [AT_NOTELF - 2] =		{ "AT_NOTELF:       ", hex },
	  [AT_UID - 2] =		{ "AT_UID:          ", dec },
	  [AT_EUID - 2] =		{ "AT_EUID:         ", dec },
	  [AT_GID - 2] =		{ "AT_GID:          ", dec },
	  [AT_EGID - 2] =		{ "AT_EGID:         ", dec },
	  [AT_PLATFORM - 2] =		{ "AT_PLATFORM:     ", str },
	  [AT_HWCAP - 2] =		{ "AT_HWCAP:        ", hex },
	  [AT_CLKTCK - 2] =		{ "AT_CLKTCK:       ", dec },
	  [AT_FPUCW - 2] =		{ "AT_FPUCW:        ", hex },
	  [AT_DCACHEBSIZE - 2] =	{ "AT_DCACHEBSIZE:  0x", hex },
	  [AT_ICACHEBSIZE - 2] =	{ "AT_ICACHEBSIZE:  0x", hex },
	  [AT_UCACHEBSIZE - 2] =	{ "AT_UCACHEBSIZE:  0x", hex },
	  [AT_IGNOREPPC - 2] =		{ "AT_IGNOREPPC", ignore },
	  [AT_SECURE - 2] =		{ "AT_SECURE:       ", dec },
	  [AT_SYSINFO - 2] =		{ "AT_SYSINFO:      0x", hex },
	  [AT_SYSINFO_EHDR - 2] =	{ "AT_SYSINFO_EHDR: 0x", hex },
	};
      unsigned int idx = (unsigned int) (av->a_type - 2);

      if ((unsigned int) av->a_type < 2u || auxvars[idx].form == ignore)
	continue;

      assert (AT_NULL == 0);
      assert (AT_IGNORE == 1);

      if (av->a_type == AT_HWCAP)
	{
	  /* This is handled special.  */
	  if (_dl_procinfo (av->a_un.a_val) == 0)
	    continue;
	}

      if (idx < sizeof (auxvars) / sizeof (auxvars[0])
	  && auxvars[idx].form != unknown)
	{
	  const char *val = av->a_un.a_ptr;

	  if (__builtin_expect (auxvars[idx].form, dec) == dec)
	    val = _itoa ((unsigned long int) av->a_un.a_val,
			 buf + sizeof buf - 1, 10, 0);
	  else if (__builtin_expect (auxvars[idx].form, hex) == hex)
	    val = _itoa ((unsigned long int) av->a_un.a_val,
			 buf + sizeof buf - 1, 16, 0);

	  _dl_printf ("%s%s\n", auxvars[idx].label, val);

	  continue;
	}

      /* Unknown value: print a generic line.  */
      char buf2[17];
      buf[sizeof (buf2) - 1] = '\0';
      const char *val2 = _itoa ((unsigned long int) av->a_un.a_val,
				buf2 + sizeof buf2 - 1, 16, 0);
      const char *val =  _itoa ((unsigned long int) av->a_type,
				buf + sizeof buf - 1, 16, 0);
      _dl_printf ("AT_??? (0x%s): 0x%s\n", val, val2);
    }
}


/* Return an array of useful/necessary hardware capability names.  */
const struct r_strlenpair *
internal_function
_dl_important_hwcaps (const char *platform, size_t platform_len, size_t *sz,
		      size_t *max_capstrlen)
{
  /* Determine how many important bits are set.  */
  unsigned long int masked = GLRO(dl_hwcap) & GLRO(dl_hwcap_mask);
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

#ifdef USE_TLS
  /* For TLS enabled builds always add 'tls'.  */
  ++cnt;
#else
  if (cnt == 0)
    {
      /* If we have platform name and no important capability we only have
	 the base directory to search.  */
      result = (struct r_strlenpair *) malloc (sizeof (*result));
      if (result == NULL)
	goto no_memory;

      result[0].str = (char *) result;	/* Does not really matter.  */
      result[0].len = 0;

      *sz = 1;
      return result;
    }
#endif

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
#ifdef USE_TLS
  temp[m].str = "tls";
  temp[m].len = 3;
  ++m;
#endif
  assert (m == cnt);

  /* Determine the total size of all strings together.  */
  if (cnt == 1)
    total = temp[0].len + 1;
  else
    {
      total = (1UL << (cnt - 2)) * (temp[0].len + temp[cnt - 1].len + 2);
      for (n = 1; n + 1 < cnt; ++n)
	total += (1UL << (cnt - 3)) * (temp[n].len + 1);
    }

  /* The result structure: we use a very compressed way to store the
     various combinations of capability names.  */
  *sz = 1 << cnt;
  result = (struct r_strlenpair *) malloc (*sz * sizeof (*result) + total);
  if (result == NULL)
    {
#ifndef USE_TLS
    no_memory:
#endif
      _dl_signal_error (ENOMEM, NULL, NULL,
			N_("cannot create capability list"));
    }

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
      n = 1 << (cnt - 1);
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
      if ((--n & 1) != 0)
	rp[0].str = rp[-2].str + rp[-2].len;
      else
	rp[0].str = rp[-1].str;
      ++rp;
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
