/* Copyright (C) 1997 Free Software Foundation, Inc.
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

#ifndef _AIO_MISC_H
#define _AIO_MISC_H	1

#include <semaphore.h>

/* Union of the two request types.  */
typedef union
  {
    struct aiocb aiocb;
    struct aiocb64 aiocb64;
  } aiocb_union;

/* List of enqueued requests.  */
extern aiocb_union *__aio_requests;

/* Lock for global I/O list of requests.  */
extern sem_t __aio_requests_sema;


/* Enqueue request.  */
extern int __aio_enqueue_request (aiocb_union *aiocbp, int operation,
				  int require_lock);

/* Send the signal.  */
extern int __aio_sigqueue (int sig, const union sigval val);

#endif /* aio_misc.h */
