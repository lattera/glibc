/* uselocale -- fetch and set the current per-thread locale
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

#include <locale.h>
#include "localeinfo.h"

#ifdef SHARED

/* Switch the current thread's locale to DATASET.
   If DATASET is null, instead just return the current setting.
   The special value LC_GLOBAL_LOCALE is the initial setting
   for all threads, and means the thread uses the global
   setting controlled by `setlocale'.  */
locale_t
__uselocale (locale_t newloc)
{
  if (newloc == NULL)
    {
      locale_t loc = __libc_tsd_get (LOCALE);
      return loc == &_nl_global_locale ? LC_GLOBAL_LOCALE : loc;
    }
  if (newloc == LC_GLOBAL_LOCALE)
    {
      __libc_tsd_set (LOCALE, &_nl_global_locale);
      return LC_GLOBAL_LOCALE;
    }
  __libc_tsd_set (LOCALE, newloc);
  return newloc;
}
weak_alias (__uselocale, uselocale)

#else

# warning uselocale not implemented for static linking yet

#endif
