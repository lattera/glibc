/* Copyright (C) 2001 Free Software Foundation, Inc.
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

#include <stdarg.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>
#include <dlfcn.h>
#include <dlldr.h>

extern int _dl_numso;
extern DL_SODATA *_dl_sotable;

void *
_dl_sym (void *handle, const char *symbol, void *who)
{
  void *rt_function;

  if ((int) handle < 0 || (int) handle >= _dl_numso || _dl_sotable == NULL)
    {
      errno = EINVAL;
      return NULL;
    }

  switch (_dl_sotable[(int) handle].type)
    {
    case DL_UNIX_SYSCALL:
      rt_function = (void *) __loadx (DL_UNIX_SYSCALL, (void *) symbol);
      break;

    case DL_GETSYM:
      rt_function = (void *) __loadx (DL_GETSYM, (void *) symbol,
				      _dl_sotable[(int) handle].index,
				      _dl_sotable[(int) handle].dataorg);
      break;

    default:
      errno = EINVAL;
      return NULL;
    }

  return rt_function;
}
