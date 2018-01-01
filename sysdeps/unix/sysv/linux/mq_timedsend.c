/* Send a message to a message queue with a timeout.  Linux version.
   Copyright (C) 2017-2018 Free Software Foundation, Inc.
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

#include <mqueue.h>
#include <sysdep-cancel.h>

/* Add message pointed by MSG_PTR to message queue MQDES, stop blocking
   on full message queue if ABS_TIMEOUT expires.  */
int
__mq_timedsend (mqd_t mqdes, const char *msg_ptr, size_t msg_len,
		unsigned int msg_prio, const struct timespec *abs_timeout)
{
  return SYSCALL_CANCEL (mq_timedsend, mqdes, msg_ptr, msg_len, msg_prio,
			 abs_timeout);
}
hidden_def (__mq_timedsend)
weak_alias (__mq_timedsend, mq_timedsend)
hidden_weak (mq_timedsend)
