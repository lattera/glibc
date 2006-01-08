/* Operating system specific code for generic dynamic loader functions.  Linux.
   Copyright (C) 2000,2001,2002,2004,2005,2006 Free Software Foundation, Inc.
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

#include <string.h>
#include <fcntl.h>
#include <sys/sysctl.h>
#include <sys/utsname.h>
#include <kernel-features.h>
#include <dl-sysdep.h>
#include <stdint.h>

#ifndef MIN
# define MIN(a,b) (((a)<(b))?(a):(b))
#endif

#ifdef SHARED
/* This is the function used in the dynamic linker to print the fatal error
   message.  */
static inline void
__attribute__ ((__noreturn__))
dl_fatal (const char *str)
{
  _dl_dprintf (2, str);
  _exit (1);
}
#endif

static inline int __attribute__ ((always_inline))
_dl_discover_osversion (void)
{
#if (defined NEED_DL_SYSINFO || defined NEED_DL_SYSINFO_DSO) && defined SHARED
  if (GLRO(dl_sysinfo_map) != NULL)
    {
      /* If the kernel-supplied DSO contains a note indicating the kernel's
	 version, we don't need to call uname or parse any strings.  */

      static const struct
      {
	ElfW(Word) vendorlen;
	ElfW(Word) datalen;
	ElfW(Word) type;
	char vendor[8];
      } expected_note = { sizeof "Linux", sizeof (ElfW(Word)), 0, "Linux" };
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
		if (!memcmp (note, &expected_note, sizeof expected_note))
		  return *(const ElfW(Word) *) ((const void *) note
						+ sizeof expected_note);
#define ROUND(len) (((len) + sizeof (ElfW(Word)) - 1) & -sizeof (ElfW(Word)))
		note = ((const void *) (note + 1)
			+ ROUND (note->vendorlen) + ROUND (note->datalen));
	      }
	  }
    }
#endif

  char bufmem[64];
  char *buf = bufmem;
  unsigned int version;
  int parts;
  char *cp;
  struct utsname uts;

  /* Try the uname system call.  */
  if (__uname (&uts))
    {
      /* This was not successful.  Now try reading the /proc filesystem.  */
      int fd = __open ("/proc/sys/kernel/osrelease", O_RDONLY);
      if (fd < 0)
	return -1;
      ssize_t reslen = __read (fd, bufmem, sizeof (bufmem));
      __close (fd);
      if (reslen <= 0)
	/* This also didn't work.  We give up since we cannot
	   make sure the library can actually work.  */
	return -1;
      buf[MIN (reslen, (ssize_t) sizeof (bufmem) - 1)] = '\0';
    }
  else
    buf = uts.release;

  /* Now convert it into a number.  The string consists of at most
     three parts.  */
  version = 0;
  parts = 0;
  cp = buf;
  while ((*cp >= '0') && (*cp <= '9'))
    {
      unsigned int here = *cp++ - '0';

      while ((*cp >= '0') && (*cp <= '9'))
	{
	  here *= 10;
	  here += *cp++ - '0';
	}

      ++parts;
      version <<= 8;
      version |= here;

      if (*cp++ != '.')
	/* Another part following?  */
	break;
    }

  if (parts < 3)
    version <<= 8 * (3 - parts);

  return version;
}

#define DL_SYSDEP_OSCHECK(FATAL)					      \
  do {									      \
    /* Test whether the kernel is new enough.  This test is only performed    \
       if the library is not compiled to run on all kernels.  */	      \
									      \
    int version = _dl_discover_osversion ();				      \
    if (__builtin_expect (version >= 0, 1))				      \
      {									      \
	if (__builtin_expect (GLRO(dl_osversion) == 0, 1)		      \
	    || GLRO(dl_osversion) > version)				      \
	  GLRO(dl_osversion) = version;					      \
									      \
	/* Now we can test with the required version.  */		      \
	if (__LINUX_KERNEL_VERSION > 0 && version < __LINUX_KERNEL_VERSION)   \
	  /* Not sufficent.  */						      \
	  FATAL ("FATAL: kernel too old\n");				      \
      }									      \
    else if (__LINUX_KERNEL_VERSION > 0)				      \
      FATAL ("FATAL: cannot determine kernel version\n");		      \
  } while (0)

static inline uintptr_t __attribute__ ((always_inline))
_dl_setup_stack_chk_guard (void)
{
  uintptr_t ret;
#ifdef ENABLE_STACKGUARD_RANDOMIZE
  int fd = __open ("/dev/urandom", O_RDONLY);
  if (fd >= 0)
    {
      ssize_t reslen = __read (fd, &ret, sizeof (ret));
      __close (fd);
      if (reslen == (ssize_t) sizeof (ret))
	return ret;
    }
#endif
  ret = 0;
  unsigned char *p = (unsigned char *) &ret;
  p[sizeof (ret) - 1] = 255;
  p[sizeof (ret) - 2] = '\n';
  return ret;
}
