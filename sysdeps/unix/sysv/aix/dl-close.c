/* Copyright (C) 2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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

#include <stdarg.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>
#include <dlfcn.h>
#include <dlldr.h>

extern int _dl_numso;
extern DL_SODATA *_dl_sotable;

void
_dl_close (void *handle)
{
  if ((int) handle < 0 || (int) handle >= _dl_numso || _dl_sotable == NULL)
    {
      errno = EINVAL;
      return;
    }

  if (_dl_sotable[(int) handle].handle != 0)
    __unload (_dl_sotable[(int) handle].handle);

  _dl_sotable[(int) handle].index = 0;
  _dl_sotable[(int) handle].dataorg = 0;
  _dl_sotable[(int) handle].handle = 0;
  _dl_sotable[(int) handle].type = 0;
}

