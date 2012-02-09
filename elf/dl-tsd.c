/* Thread-local data used by error handling for runtime dynamic linker.
   Copyright (C) 2002, 2005, 2011 Free Software Foundation, Inc.
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

#ifdef _LIBC_REENTRANT

# include <ldsodefs.h>
# include <tls.h>

# ifndef SHARED

/* _dl_error_catch_tsd points to this for the single-threaded case.
   It's reset by the thread library for multithreaded programs
   if we're not using __thread.  */
void ** __attribute__ ((const))
_dl_initial_error_catch_tsd (void)
{
  static __thread void *data;
  return &data;
}
void **(*_dl_error_catch_tsd) (void) __attribute__ ((const))
     = &_dl_initial_error_catch_tsd;

# else

/* libpthread sets _dl_error_catch_tsd to point to this function.
   We define it here instead of in libpthread so that it doesn't
   need to have a TLS segment of its own just for this one pointer.  */

void ** __attribute__ ((const))
__libc_dl_error_tsd (void)
{
  static __thread void *data attribute_tls_model_ie;
  return &data;
}

# endif	/* SHARED */

#endif /* _LIBC_REENTRANT */
