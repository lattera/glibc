/* Definitions of macros to access `dev_t' values.
   Copyright (C) 1996, 1997, 1999 Free Software Foundation, Inc.
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

#ifndef _SYS_SYSMACROS_H
#define _SYS_SYSMACROS_H	1

/* For compatibility we provide alternative names.

   The problem here is that compilers other than GCC probably don't
   have the `long long' type and so `dev_t' is actually an array.  */
#if defined __GNUC__ && __GNUC__ >= 2
# define major(dev) ((int)(((dev) >> 8) & 0xff))
# define minor(dev) ((int)((dev) & 0xff))
# define makedev(major, minor) ((((unsigned int) (major)) << 8) \
				| ((unsigned int) (minor)))
#else
/* We need to know the word order here.  This assumes that the word order
   is consistent with the byte order.  */
# include <endian.h>
# if __BYTE_ORDER == __BIG_ENDIAN
#  define major(dev) (((dev).__val[1] >> 8) & 0xff)
#  define minor(dev) ((dev).__val[1] & 0xff)
#  define makedev(major, minor) { 0, ((((unsigned int) (major)) << 8) \
				      | ((unsigned int) (minor))) }
# else
#  define major(dev) (((dev).__val[0] >> 8) & 0xff)
#  define minor(dev) ((dev).__val[0] & 0xff)
#  define makedev(major, minor) { ((((unsigned int) (major)) << 8) \
				   | ((unsigned int) (minor))), 0 }
# endif
#endif

#endif /* sys/sysmacros.h */
