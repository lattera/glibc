/* Copyright (C) 2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2001.

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

#include <netdb.h>
#include <pthread.h>

#include "gai_misc.h"


int
gai_cancel (struct gaicb *gaicbp)
{
  int result = 0;
  int status;

  /* Request the mutex.  */
  pthread_mutex_lock (&__gai_requests_mutex);

  /* Find the request among those queued but not yet running.  */
  status = __gai_remove_request (gaicbp);
  if (status == 0)
    result = EAI_CANCELED;
  else if (status > 0)
    result = EAI_NOTCANCELED;
  else
    result = EAI_ALLDONE;

  /* Release the mutex.  */
  pthread_mutex_unlock (&__gai_requests_mutex);

  return result;
}
