/* Duplicate handle for selection of locales.
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

#include <locale.h>
#include <libc-lock.h>
#include <stdlib.h>

#include <localeinfo.h>


/* Lock for protecting global data.  */
__libc_lock_define (extern , __libc_setlocale_lock)


__locale_t
__duplocale (__locale_t dataset)
{
  __locale_t result;

  /* We modify global data.  */
  __libc_lock_lock (__libc_setlocale_lock);

  /* Get memory.  */
  result = (__locale_t) malloc (sizeof (struct __locale_t));
  if (result != NULL)
    {
      int cnt;
      for (cnt = 0; cnt < LC_ALL; ++cnt)
	{
	  result->__locales[cnt] = dataset->__locales[cnt];
	  if (result->__locales[cnt]->usage_count != MAX_USAGE_COUNT)
	    ++result->__locales[cnt]->usage_count;
	}
    }

  /* It's done.  */
  __libc_lock_unlock (__libc_setlocale_lock);
}
