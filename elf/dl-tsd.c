/* Thread-local data used by error handling for runtime dynamic linker.
   Copyright (C) 2002 Free Software Foundation, Inc.
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
#  if USE___THREAD
  static __thread void *data;
#  else
  static void *data;
#  endif
  return &data;
}
void **(*_dl_error_catch_tsd) (void) __attribute__ ((const))
     = &_dl_initial_error_catch_tsd;

# elif USE___THREAD

/* libpthread sets _dl_error_catch_tsd to point to this function.
   We define it here instead of in libpthread so that it doesn't
   need to have a TLS segment of its own just for this one pointer.  */

void ** __attribute__ ((const))
__libc_dl_error_tsd (void)
{
  static __thread void *data;
  return &data;
}

# endif	/* SHARED */

#endif /* _LIBC_REENTRANT */
