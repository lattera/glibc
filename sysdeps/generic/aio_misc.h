/* Internal declarations for <aio.h> functions implementation.  Stub version.
   Copyright (C) 2001 Free Software Foundation, Inc.
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

#ifndef _AIO_MISC_H
#define _AIO_MISC_H	1

#include <aio.h>


/* Extend the operation enum.  */
enum
{
  LIO_DSYNC = LIO_READ + 1,
  LIO_SYNC,
  LIO_READ64 = LIO_READ | 128,
  LIO_WRITE64 = LIO_WRITE | 128
};

/* Union of the two request types.  */
typedef union
  {
    struct aiocb aiocb;
    struct aiocb64 aiocb64;
  } aiocb_union;


/* Send the signal.  */
extern int __aio_sigqueue (int sig, const union sigval val, pid_t caller_pid)
     internal_function;


#endif /* aio_misc.h */
