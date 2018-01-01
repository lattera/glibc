/* Enqueue and list of read or write requests, 64bit offset version.
   Copyright (C) 1997-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1997.

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

#include <aio.h>
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#include <aio_misc.h>

#define lio_listio lio_listio64
#define __lio_listio_21 __lio_listio64_21
#define __lio_listio_item_notify __lio_listio64_item_notify
#define aiocb aiocb64
#define LIO_OPCODE_BASE 128
#include <lio_listio.c>
