/* Copyright (C) 1998 Free Software Foundation, Inc.
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

#ifndef _EXECINFO_H
#define _EXECINFO_H 1

#include <features.h>

__BEGIN_DECLS

/* Store up to SIZE return address of the current program state in
   ARRAY and return the exact number of values stored.  */
extern int __backtrace __P ((void **__array, int __size));
extern int backtrace __P ((void **__array, int __size));


/* Return names of functions from the backtrace list in ARRAY in a newly
   malloc()ed memory block.  */
extern char **__backtrace_symbols __P ((void *__const *__array, int __size));
extern char **backtrace_symbols __P ((void *__const *__array, int __size));

__END_DECLS

#endif /* execinfo.h  */
