/* Network-related functions for internal library use.
   Copyright (C) 2016-2018 Free Software Foundation, Inc.
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

#ifndef _NET_INTERNAL_H
#define _NET_INTERNAL_H 1

#include <arpa/inet.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/time.h>

int __inet6_scopeid_pton (const struct in6_addr *address,
                          const char *scope, uint32_t *result);
libc_hidden_proto (__inet6_scopeid_pton)


/* Deadline handling for enforcing timeouts.

   Code should call __deadline_current_time to obtain the current time
   and cache it locally.  The cache needs updating after every
   long-running or potentially blocking operation.  Deadlines relative
   to the current time can be computed using __deadline_from_timeval.
   The deadlines may have to be recomputed in response to certain
   events (such as an incoming packet), but they are absolute (not
   relative to the current time).  A timeout suitable for use with the
   poll function can be computed from such a deadline using
   __deadline_to_ms.

   The fields in the structs defined belowed should only be used
   within the implementation.  */

/* Cache of the current time.  Used to compute deadlines from relative
   timeouts and vice versa.  */
struct deadline_current_time
{
  struct timespec current;
};

/* Return the current time.  Terminates the process if the current
   time is not available.  */
struct deadline_current_time __deadline_current_time (void) attribute_hidden;

/* Computed absolute deadline.  */
struct deadline
{
  struct timespec absolute;
};


/* For internal use only.  */
static inline bool
__deadline_is_infinite (struct deadline deadline)
{
  return deadline.absolute.tv_nsec < 0;
}

/* Return true if the current time is at the deadline or past it.  */
static inline bool
__deadline_elapsed (struct deadline_current_time current,
                    struct deadline deadline)
{
  return !__deadline_is_infinite (deadline)
    && (current.current.tv_sec > deadline.absolute.tv_sec
        || (current.current.tv_sec == deadline.absolute.tv_sec
            && current.current.tv_nsec >= deadline.absolute.tv_nsec));
}

/* Return the deadline which occurs first.  */
static inline struct deadline
__deadline_first (struct deadline left, struct deadline right)
{
  if (__deadline_is_infinite (right)
      || left.absolute.tv_sec < right.absolute.tv_sec
      || (left.absolute.tv_sec == right.absolute.tv_sec
          && left.absolute.tv_nsec < right.absolute.tv_nsec))
    return left;
  else
    return right;
}

/* Add TV to the current time and return it.  Returns a special
   infinite absolute deadline on overflow.  */
struct deadline __deadline_from_timeval (struct deadline_current_time,
                                         struct timeval tv) attribute_hidden;

/* Compute the number of milliseconds until the specified deadline,
   from the current time in the argument.  The result is mainly for
   use with poll.  If the deadline has already passed, return 0.  If
   the result would overflow an int, return INT_MAX.  */
int __deadline_to_ms (struct deadline_current_time, struct deadline)
  attribute_hidden;

/* Return true if TV.tv_sec is non-negative and TV.tv_usec is in the
   interval [0, 999999].  */
static inline bool
__is_timeval_valid_timeout (struct timeval tv)
{
  return tv.tv_sec >= 0 && tv.tv_usec >= 0 && tv.tv_usec < 1000 * 1000;
}

#endif /* _NET_INTERNAL_H */
