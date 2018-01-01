/* Support functions for handling RES_USE_INET6 in getaddrinfo/nscd.
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

#ifndef _RES_USE_INET6_H
#define _RES_USE_INET6_H

#include <resolv/resolv_context.h>
#include <resolv/resolv-internal.h>

/* Ensure that RES_USE_INET6 is disabled in *CTX.  Return true if
   __resolv_context_enable_inet6 below should enable RES_USE_INET6
   again.  */
static inline bool
__resolv_context_disable_inet6 (struct resolv_context *ctx)
{
  if (ctx != NULL && ctx->resp->options & DEPRECATED_RES_USE_INET6)
    {
      ctx->resp->options &= ~DEPRECATED_RES_USE_INET6;
      return true;
    }
  else
    return false;
}

/* If ENABLE, re-enable RES_USE_INET6 in *CTX.  To be paired with
   __resolv_context_disable_inet6.  */
static inline void
__resolv_context_enable_inet6 (struct resolv_context *ctx, bool enable)
{
  if (ctx != NULL && enable)
    ctx->resp->options |= DEPRECATED_RES_USE_INET6;
}

#endif
