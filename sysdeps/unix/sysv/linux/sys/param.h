/* Copyright (C) 1995 Free Software Foundation, Inc.
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
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#ifndef _SYS_PARAM_H
#define _SYS_PARAM_H

#include <limits.h>
#include <linux/limits.h>
#include <linux/param.h>

#include <sys/types.h>


#ifndef howmany
# define howmany(x, y)	(((x)+((y)-1))/(y))
#endif

#ifndef roundup
# define roundup(x, y)	((((x)+((y)-1))/(y))*(y))
#endif

#define MAXPATHLEN      PATH_MAX
#define NOFILE          OPEN_MAX

/*  Following the information of some of the kernel people I here assume
    that block size (i.e. the value of stat.st_blocks) for all filesystem
    is 512 bytes.  If not tell HJ, Roland, or me.  -- drepper */
#define DEV_BSIZE       512

#endif
