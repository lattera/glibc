/* config.h used by locale and localedef programs in GNU libc.
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

#ifndef _CONFIG_H
#define _CONFIG_H

#define PACKAGE		"libc"
#define VERSION		__libc_version
extern const char __libc_version[];

#define DEFAULT_CHARMAP	"POSIX"


/* These are tested by xmalloc.c and error.c.  */
#define HAVE_VPRINTF 1
#define STDC_HEADERS 1
#define HAVE_STRERROR 1

#define program_name program_invocation_name

typedef unsigned short u16;
typedef int i32;
typedef int u32;


/* Get the global libc configuration info.  */
#include_next <config.h>

#endif /* config.h */
