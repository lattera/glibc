/* Cleanup code for data structures kept by strftime/strptime helper functions.
   Copyright (C) 2002, 2010 Free Software Foundation, Inc.
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

#include "../locale/localeinfo.h"
#include <stdlib.h>

void internal_function
_nl_cleanup_time (struct __locale_data *locale)
{
  struct lc_time_data *const data = locale->private.time;
  if (data != NULL)
    {
      locale->private.time = NULL;
      locale->private.cleanup = NULL;

      free (data->eras);
      free (data->alt_digits);
      free (data->walt_digits);

      free (data);
    }
}
