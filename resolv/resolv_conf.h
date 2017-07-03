/* Extended resolver state separate from struct __res_state.
   Copyright (C) 2017 Free Software Foundation, Inc.
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

#ifndef RESOLV_STATE_H
#define RESOLV_STATE_H

#include <stdbool.h>
#include <stddef.h>

/* Extended resolver state associated with res_state objects.  Client
   code can reach this state through a struct resolv_context
   object.  */
struct resolv_conf
{
  /* Used to propagate the effect of res_init across threads.  This
     member is mutable and prevents sharing of the same struct
     resolv_conf object among multiple struct __res_state objects.  */
  unsigned long long int initstamp;

  /* Reference counter.  The object is deallocated once it reaches
     zero.  For internal use within resolv_conf only.  */
  size_t __refcount;
};

/* The functions below are for use by the res_init resolv.conf parser
   and the struct resolv_context facility.  */

struct __res_state;

/* Return the extended resolver state for *RESP, or NULL if it cannot
   be determined.  A call to this function must be paired with a call
   to __resolv_conf_put.  */
struct resolv_conf *__resolv_conf_get (struct __res_state *) attribute_hidden;

/* Converse of __resolv_conf_get.  */
void __resolv_conf_put (struct resolv_conf *) attribute_hidden;

/* Allocate a new struct resolv_conf object and copy the
   pre-configured values from *INIT.  Return NULL on allocation
   failure.  The object must be deallocated using
   __resolv_conf_put.  */
struct resolv_conf *__resolv_conf_allocate (const struct resolv_conf *init)
  attribute_hidden __attribute__ ((nonnull (1), warn_unused_result));

/* Associate an existing extended resolver state with *RESP.  Return
   false on allocation failure.  In addition, update *RESP with the
   overlapping non-extended resolver state.  */
bool __resolv_conf_attach (struct __res_state *, struct resolv_conf *)
  attribute_hidden;

/* Detach the extended resolver state from *RESP.  */
void __resolv_conf_detach (struct __res_state *resp) attribute_hidden;

#endif /* RESOLV_STATE_H */
