/* Header describing obsolete `ustat' interface.
Copyright (C) 1996 Free Software Foundation, Inc.
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

/* This interface is obsolete.  Use <sys/statfs.h> instead.  */

#ifndef _SYS_USTAT_H
#define _SYS_USTAT_H 1

#include <sys/types.h>
#include <ustatbits.h>

__BEGIN_DECLS

extern int __ustat __P ((dev_t, struct ustat *));
extern int ustat __P ((dev_t, struct ustat *));

__END_DECLS

#endif /* _SYS_USTAT_H */
