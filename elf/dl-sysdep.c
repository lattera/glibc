/* Operating system support for run-time dynamic linker.  Generic Unix version.
   Copyright (C) 1995-1998,2000-2008,2009,2010
	Free Software Foundation, Inc.
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
#include <_itoa.h>
#include <fpu_control.h>

#include <entry.h>
#include <dl-machine.h>
#include <dl-procinfo.h>
#include <dl-osinfo.h>
#include <hp-timing.h>
#include <tls.h>

#ifdef _DL_FIRST_PLATFORM
# define _DL_FIRST_EXTRA (_DL_FIRST_PLATFORM + _DL_PLATFORMS_COUNT)
#else
# define _DL_FIRST_EXTRA _DL_HWCAP_COUNT
#endif

extern char **_environ attribute_hidden;
extern char _end[] attribute_hidden;

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
void *_dl_random attribute_relro = NULL;

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
				   ElfW(Addr) *user_entry, ElfW(auxv_t) *auxv))
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
	phdr = (void *) av->a_un.a_val;
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
	GLRO(dl_platform) = (void *) av->a_un.a_val;
	break;
      case AT_HWCAP:
	GLRO(dl_hwcap) = (unsigned long int) av->a_un.a_val;
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
	GLRO(dl_sysinfo_dso) = (void *) av->a_un.a_val;
	break;
#endif
      case AT_RANDOM:
	_dl_random = (void *) av->a_un.a_val;
	break;
#ifdef DL_PLATFORM_AUXV
      DL_PLATFORM_AUXV
#endif
      }

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

  if (__sbrk (0) == _end)
    /* The dynamic linker was run as a program, and so the initial break
       starts just after our bss, at &_end.  The malloc in dl-minimal.c
       will consume the rest of this page, so tell the kernel to move the
       break up that far.  When the user program examines its break, it
       will see this new value and not clobber our data.  */
    __sbrk (GLRO(dl_pagesize)
	    - ((_end - (char *) 0) & (GLRO(dl_pagesize) - 1)));

  /* If this is a SUID program we make sure that FDs 0, 1, and 2 are
     allocated.  If necessary we are doing it ourself.  If it is not
     possible we stop the program.  */
  if (__builtin_expect (INTUSE(__libc_enable_secure), 0))
    __libc_check_standard_fds ();

  (*dl_main) (phdr, phnum, &user_entry, _dl_auxv);
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
	const char label[17];
	enum { unknown = 0, dec, hex, str, ignore } form : 8;
      } auxvars[] =
	{
	  [AT_EXECFD - 2] =		{ "EXECFD:       ", dec },
	  [AT_EXECFN - 2] =		{ "EXECFN:       ", str },
	  [AT_PHDR - 2] =		{ "PHDR:         0x", hex },
	  [AT_PHENT - 2] =		{ "PHENT:        ", dec },
	  [AT_PHNUM - 2] =		{ "PHNUM:        ", dec },
	  [AT_PAGESZ - 2] =		{ "PAGESZ:       ", dec },
	  [AT_BASE - 2] =		{ "BASE:         0x", hex },
	  [AT_FLAGS - 2] =		{ "FLAGS:        0x", hex },
	  [AT_ENTRY - 2] =		{ "ENTRY:        0x", hex },
	  [AT_NOTELF - 2] =		{ "NOTELF:       ", hex },
	  [AT_UID - 2] =		{ "UID:          ", dec },
	  [AT_EUID - 2] =		{ "EUID:         ", dec },
	  [AT_GID - 2] =		{ "GID:          ", dec },
	  [AT_EGID - 2] =		{ "EGID:         ", dec },
	  [AT_PLATFORM - 2] =		{ "PLATFORM:     ", str },
	  [AT_HWCAP - 2] =		{ "HWCAP:        ", hex },
	  [AT_CLKTCK - 2] =		{ "CLKTCK:       ", dec },
	  [AT_FPUCW - 2] =		{ "FPUCW:        ", hex },
	  [AT_DCACHEBSIZE - 2] =	{ "DCACHEBSIZE:  0x", hex },
	  [AT_ICACHEBSIZE - 2] =	{ "ICACHEBSIZE:  0x", hex },
	  [AT_UCACHEBSIZE - 2] =	{ "UCACHEBSIZE:  0x", hex },
	  [AT_IGNOREPPC - 2] =		{ "IGNOREPPC", ignore },
	  [AT_SECURE - 2] =		{ "SECURE:       ", dec },
	  [AT_BASE_PLATFORM - 2] =	{ "BASE_PLATFORM:", str },
	  [AT_SYSINFO - 2] =		{ "SYSINFO:      0x", hex },
	  [AT_SYSINFO_EHDR - 2] =	{ "SYSINFO_EHDR: 0x", hex },
	  [AT_RANDOM - 2] =		{ "RANDOM:       0x", hex },
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
	  const char *val = (char *) av->a_un.a_val;

	  if (__builtin_expect (auxvars[idx].form, dec) == dec)
	    val = _itoa ((unsigned long int) av->a_un.a_val,
			 buf + sizeof buf - 1, 10, 0);
	  else if (__builtin_expect (auxvars[idx].form, hex) == hex)
	    val = _itoa ((unsigned long int) av->a_un.a_val,
			 buf + sizeof buf - 1, 16, 0);

	  _dl_printf ("AT_%s%s\n", auxvars[idx].label, val);

	  continue;
	}

      /* Unknown value: print a generic line.  */
      char buf2[17];
      buf2[sizeof (buf2) - 1] = '\0';
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
  uint64_t masked = GLRO(dl_hwcap) & GLRO(dl_hwcap_mask);
  size_t cnt = platform != NULL;
  size_t n, m;
  size_t total;
  struct r_strlenpair *temp;
  struct r_strlenpair *result;
  struct r_strlenpair *rp;
  char *cp;

  /* Count the number of bits set in the masked value.  */
  for (n = 0; (~((1ULL << n) - 1) & masked) != 0; ++n)
    if ((masked & (1ULL << n)) != 0)
      ++cnt;

#if (defined NEED_DL_SYSINFO || defined NEED_DL_SYSINFO_DSO) && defined SHARED
  /* The system-supplied DSO can contain a note of type 2, vendor "GNU".
     This gives us a list of names to treat as fake hwcap bits.  */

  const char *dsocaps = NULL;
  size_t dsocapslen = 0;
  if (GLRO(dl_sysinfo_map) != NULL)
    {
      const ElfW(Phdr) *const phdr = GLRO(dl_sysinfo_map)->l_phdr;
      const ElfW(Word) phnum = GLRO(dl_sysinfo_map)->l_phnum;
      for (uint_fast16_t i = 0; i < phnum; ++i)
	if (phdr[i].p_type == PT_NOTE)
	  {
	    const ElfW(Addr) start = (phdr[i].p_vaddr
				      + GLRO(dl_sysinfo_map)->l_addr);
	    const struct
	    {
	      ElfW(Word) vendorlen;
	      ElfW(Word) datalen;
	      ElfW(Word) type;
	    } *note = (const void *) start;
	    while ((ElfW(Addr)) (note + 1) - start < phdr[i].p_memsz)
	      {
#define ROUND(len) (((len) + sizeof (ElfW(Word)) - 1) & -sizeof (ElfW(Word)))
		if (note->type == NT_GNU_HWCAP
		    && note->vendorlen == sizeof "GNU"
		    && !memcmp ((note + 1), "GNU", sizeof "GNU")
		    && note->datalen > 2 * sizeof (ElfW(Word)) + 2)
		  {
		    const ElfW(Word) *p = ((const void *) (note + 1)
					   + ROUND (sizeof "GNU"));
		    cnt += *p++;
		    ++p;	/* Skip mask word.  */
		    dsocaps = (const char *) p;
		    dsocapslen = note->datalen - sizeof *p * 2;
		    break;
		  }
		note = ((const void *) (note + 1)
			+ ROUND (note->vendorlen) + ROUND (note->datalen));
#undef ROUND
	      }
	    if (dsocaps != NULL)
	      break;
	  }
    }
#endif

  /* For TLS enabled builds always add 'tls'.  */
  ++cnt;

  /* Create temporary data structure to generate result table.  */
  temp = (struct r_strlenpair *) alloca (cnt * sizeof (*temp));
  m = 0;
#if defined NEED_DL_SYSINFO || defined NEED_DL_SYSINFO_DSO
  if (dsocaps != NULL)
    {
      const ElfW(Word) mask = ((const ElfW(Word) *) dsocaps)[-1];
      GLRO(dl_hwcap) |= (uint64_t) mask << _DL_FIRST_EXTRA;
      /* Note that we add the dsocaps to the set already chosen by the
	 LD_HWCAP_MASK environment variable (or default HWCAP_IMPORTANT).
	 So there is no way to request ignoring an OS-supplied dsocap
	 string and bit like you can ignore an OS-supplied HWCAP bit.  */
      GLRO(dl_hwcap_mask) |= (uint64_t) mask << _DL_FIRST_EXTRA;
      size_t len;
      for (const char *p = dsocaps; p < dsocaps + dsocapslen; p += len + 1)
	{
	  uint_fast8_t bit = *p++;
	  len = strlen (p);

	  /* Skip entries that are not enabled in the mask word.  */
	  if (__builtin_expect (mask & ((ElfW(Word)) 1 << bit), 1))
	    {
	      temp[m].str = p;
	      temp[m].len = len;
	      ++m;
	    }
	  else
	    --cnt;
	}
    }
#endif
  for (n = 0; masked != 0; ++n)
    if ((masked & (1ULL << n)) != 0)
      {
	temp[m].str = _dl_hwcap_string (n);
	temp[m].len = strlen (temp[m].str);
	masked ^= 1ULL << n;
	++m;
      }
  if (platform != NULL)
    {
      temp[m].str = platform;
      temp[m].len = platform_len;
      ++m;
    }

  temp[m].str = "tls";
  temp[m].len = 3;
  ++m;

  assert (m == cnt);

  /* Determine the total size of all strings together.  */
  if (cnt == 1)
    total = temp[0].len + 1;
  else
    {
      total = temp[0].len + temp[cnt - 1].len + 2;
      if (cnt > 2)
	{
	  total <<= 1;
	  for (n = 1; n + 1 < cnt; ++n)
	    total += temp[n].len + 1;
	  if (cnt > 3
	      && (cnt >= sizeof (size_t) * 8
		  || total + (sizeof (*result) << 3)
		     >= (1UL << (sizeof (size_t) * 8 - cnt + 3))))
	    _dl_signal_error (ENOMEM, NULL, NULL,
			      N_("cannot create capability list"));

	  total <<= cnt - 3;
	}
    }

  /* The result structure: we use a very compressed way to store the
     various combinations of capability names.  */
  *sz = 1 << cnt;
  result = (struct r_strlenpair *) malloc (*sz * sizeof (*result) + total);
  if (result == NULL)
    _dl_signal_error (ENOMEM, NULL, NULL,
		      N_("cannot create capability list"));

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
	      #1: 0, 1, 3		1101
	      #2: 0, 2, 3		1011
	      #3: 0, 3			1001
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

  /* The second half starts right after the first part of the string of
     the corresponding entry in the first half.  */
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
