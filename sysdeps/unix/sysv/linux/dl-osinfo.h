/* Operating system specific code  for generic dynamic loader functions.
   Copyright (C) 2000 Free Software Foundation, Inc.
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

#include <sys/sysctl.h>
#include "kernel-features.h"

#ifndef MIN
# define MIN(a,b) (((a)<(b))?(a):(b))
#endif

/* There is no prototype for __sysctl in that file.  */
extern int __sysctl (int *name, int nlen, void *oldval,
		     size_t *oldlenp, void *newval, size_t newlen);


#ifdef SHARED
/* This is the function used in the dynamic linker to print the fatal error
   message.  */
static inline void
__attribute__ ((__noreturn__))
dl_fatal (const char *str)
{
  _dl_sysdep_output (2, str, NULL);
  _exit (1);
}
#endif


#define DL_SYSDEP_OSCHECK(FATAL) \
  do {									      \
    /* Test whether the kernel is new enough.  This test is only	      \
       performed if the library is not compiled to run on all		      \
       kernels.  */							      \
    if (__LINUX_KERNEL_VERSION > 0)					      \
      {									      \
	static const int sysctl_args[] = { CTL_KERN, KERN_OSRELEASE };	      \
	char buf[64];							      \
	size_t reslen = sizeof (buf);					      \
	unsigned int version;						      \
	int parts;							      \
	char *cp;							      \
									      \
	/* Try reading the number using `sysctl' first.  */		      \
	if (__sysctl ((int *) sysctl_args,				      \
		      sizeof (sysctl_args) / sizeof (sysctl_args[0]),	      \
		      buf, &reslen, NULL, 0) < 0)			      \
	  {								      \
	    /* This was not successful.  Now try reading the /proc	      \
	       filesystem.  */						      \
	    int fd = __open ("/proc/sys/kernel/osrelease", O_RDONLY);	      \
	    if (fd == -1						      \
		|| (reslen = __read (fd, buf, sizeof (buf))) <= 0)	      \
	      /* This also didn't work.  We give up since we cannot	      \
		 make sure the library can actually work.  */		      \
	      FATAL ("FATAL: cannot determine library version\n");	      \
									      \
	    __close (fd);						      \
	  }								      \
	buf[MIN (reslen, sizeof (buf) - 1)] = '\0';			      \
									      \
	/* Now convert it into a number.  The string consists of at most      \
	   three parts.  */						      \
	version = 0;							      \
	parts = 0;							      \
	cp = buf;							      \
	while ((*cp >= '0') && (*cp <= '9'))				      \
	  {								      \
	    unsigned int here = *cp++ - '0';				      \
									      \
	    while ((*cp >= '0') && (*cp <= '9'))			      \
	      {								      \
		here *= 10;						      \
		here += *cp++ - '0';					      \
	      }								      \
									      \
	    ++parts;							      \
	    version <<= 8;						      \
	    version |= here;						      \
									      \
	    if (*cp++ != '.')						      \
	      /* Another part following?  */				      \
	      break;							      \
	  }								      \
									      \
	if (parts < 3)							      \
	  version <<= 8 * (3 - parts);					      \
									      \
	/* Now we can test with the required version.  */		      \
	if (version < __LINUX_KERNEL_VERSION)				      \
	  /* Not sufficent.  */						      \
	  FATAL ("FATAL: kernel too old\n");				      \
      }									      \
  } while (0)
