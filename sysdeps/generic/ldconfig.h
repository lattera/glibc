/* Copyright (C) 1999, 2000, 2002, 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Andreas Jaeger <aj@suse.de>, 1999.

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

#ifndef _LDCONFIG_H
#define _LDCONFIG_H

#define FLAG_ANY		-1
#define FLAG_TYPE_MASK		0x00ff
#define FLAG_LIBC4		0x0000
#define FLAG_ELF		0x0001
#define FLAG_ELF_LIBC5		0x0002
#define FLAG_ELF_LIBC6		0x0003
#define FLAG_REQUIRED_MASK	0xff00
#define FLAG_SPARC_LIB64	0x0100
#define FLAG_IA64_LIB64		0x0200
#define FLAG_X8664_LIB64	0x0300
#define FLAG_S390_LIB64		0x0400
#define FLAG_POWERPC_LIB64	0x0500
#define FLAG_MIPS64_LIBN32	0x0600
#define FLAG_MIPS64_LIBN64	0x0700

/* Declared in cache.c.  */
extern void print_cache (const char *cache_name);

extern void init_cache (void);

extern void save_cache (const char *cache_name);

extern void add_to_cache (const char *path, const char *lib, int flags,
			  unsigned int osversion, uint64_t hwcap);

/* Declared in readlib.c.  */
extern int process_file (const char *real_file_name, const char *file_name,
			 const char *lib, int *flag, unsigned int *osversion,
			 char **soname, int is_link);

/* Declared in readelflib.c.  */
extern int process_elf_file (const char *file_name, const char *lib, int *flag,
			     unsigned int *osversion, char **soname,
			     void *file_contents, size_t file_length);

/* Declared in chroot_canon.c.  */
extern char *chroot_canon (const char *chroot, const char *name);

/* Declared in ldconfig.c.  */
extern int opt_verbose;

extern int opt_format;

/* Prototypes for a few program-wide used functions.  */
extern void *xmalloc (size_t __n);
extern void *xcalloc (size_t __n, size_t __size);
extern void *xrealloc (void *__p, size_t __n);
extern char *xstrdup (const char *__str);

#endif /* ! _LDCONFIG_H  */
