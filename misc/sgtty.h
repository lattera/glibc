/* Copyright (C) 1991, 1992 Free Software Foundation, Inc.
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
not, write to the, 1992 Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#ifndef	_SGTTY_H
#define	_SGTYY_H	1
#include <features.h>

#include <sys/ioctl.h>

__BEGIN_DECLS

/* Fill in *PARAMS with terminal parameters associated with FD.  */
extern int gtty __P ((int __fd, struct sgttyb * __params));

/* Set the terminal parameters associated with FD to *PARAMS.  */
extern int stty __P ((int __fd, __const struct sgttyb * __params));


__END_DECLS

#endif /* sgtty.h  */
