/* Copyright (C) 2015-2016 Free Software Foundation, Inc.
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

/* Set error number and return -1.  A target may choose to return the
   internal function, __syscall_error, which sets errno and returns -1.
   We use -1l, instead of -1, so that it can be casted to (void *).  */
#define INLINE_SYSCALL_ERROR_RETURN_VALUE(err)  \
  ({						\
    __set_errno (err);				\
    -1l;					\
  })
