/* Enqueue and list of read or write requests.
   Copyright (C) 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1997.

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

#include <aio.h>
#include <errno.h>
#include <semaphore.h>

#include "aio_misc.h"


int
lio_listio (mode, list, nent, sig)
     int mode;
     struct aiocb *const list[];
     int nent;
     struct sigevent *sig;
{
  int cnt;
  int total = 0;
  int result = 0;

  /* Check arguments.  */
  if (mode != LIO_WAIT && mode != LIO_NOWAIT)
    {
      __set_errno (EINVAL);
      return -1;
    }

  /* Request the semaphore.  */
  sem_wait (&__aio_requests_sema);

  /* Now we can enqueue all requests.  Since we already acquired the
     semaphore the enqueue function need not do this.  */
  for (cnt = 0; cnt < nent; ++cnt)
    if (list[cnt] != NULL && list[cnt]->aio_lio_opcode != LIO_NOP)
      if (__aio_enqueue_request ((aiocb_union *) list[cnt],
				 list[cnt]->aio_lio_opcode, 0) >= 0)
	/* Successfully enqueued.  */
	++total;
      else
	/* Signal that we've seen an error.  `errno' and the error code
	   of the aiocb will tell more.  */
	result = -1;



  /* Release the semaphore.  */
  sem_post (&__aio_requests_sema);

  return result;
}
