/* Copyright (C) 1999, 2000, 2001 Free Software Foundation, Inc.
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

#include <malloc.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>
#include <dlfcn.h>
#include <dlldr.h>

#define NUM_SHARED_OBJECTS 32

int _dl_numso = NUM_SHARED_OBJECTS;
DL_SODATA *_dl_sotable = NULL;

void *
_dl_open (const char *file, int mode, const void *caller)
{
  DL_SODATA *new_so;
  void *handle;
  int entry;
  int bsize = _dl_numso * sizeof (DL_INFO);
  DL_INFO *dl_info = malloc (bsize);

  if (dl_info == NULL)
    return NULL;

  /* 1st time thru initial shared object data table.  */
  if (_dl_sotable == NULL)
    {
      _dl_sotable = (DL_SODATA *) calloc (_dl_numso, sizeof (DL_SODATA));
      if (_dl_sotable == NULL)
	return NULL;

      __loadx (DL_POSTLOADQ, dl_info, bsize, NULL);
      while (!(dl_info[0].dlinfo_xflags & DL_INFO_OK)
	     || dl_info[0].dlinfo_arraylen == 0)
	{
	  bsize *= 2;
	  dl_info = realloc (dl_info, bsize);
	  if (dl_info == NULL)
	    return NULL;

	  __loadx (DL_POSTLOADQ, dl_info, bsize, NULL);
	}
    }

  /* Validate mode bits.  */
  if (!(mode & RTLD_NOW) && !(mode & RTLD_LAZY))
    {
      free (dl_info);
      errno = EINVAL;
      return NULL;
    }

  /* Load the module.  */
  handle = (void *) __loadx (DL_LOAD | DL_LOAD_RTL | DL_LOAD_LDX1,
                             dl_info, bsize, file, NULL);
  if (handle == NULL)
    {
      free (dl_info);
      errno = EINVAL;
      return NULL;
    }

  /* Was dl_info buffer to small to get info.  */
  while (!(dl_info[0].dlinfo_xflags & DL_INFO_OK)
	 || dl_info[0].dlinfo_arraylen == 0)
    {
      bsize *= 2;
      dl_info = realloc (dl_info, bsize);
      if (new_dl_info == NULL)
        {
	  (void) __unload ((void *) handle);
          errno = ENOMEM;
          return NULL;
        }
      __loadx (DL_POSTLOADQ | DL_LOAD_RTL, dl_info, bsize, handle);
    }

  /* Get an empty entry in the shared object table.  */
  for (entry = 0; entry < _dl_numso; ++entry)
    if (_dl_sotable[entry].type == 0)
      break;

  /* See if the table needs to be increased.  */
  if (entry == _dl_numso)
    {
      new_so = (DL_SODATA *) realloc (_dl_sotable,
				      _dl_numso * 2 * sizeof (DL_SODATA));
      if (new_so == NULL)
	return NULL;

      memset (new_so + _dl_numso, '\0', _dl_numso * sizeof (DL_SODATA));
      _dl_numso  *= 2;
      _dl_sotable = new_so;
    }

  /* See if this is syscall (look for /unix in file).  */
  if (strcmp ("/unix", file) == 0)
    {
      _dl_sotable[entry].index = dl_info[1].dlinfo_index;
      _dl_sotable[entry].dataorg = dl_info[1].dlinfo_dataorg;
      _dl_sotable[entry].handle = handle;
      _dl_sotable[entry].type = DL_UNIX_SYSCALL;
    }
  else
    {
      _dl_sotable[entry].index = dl_info[1].dlinfo_index;
      _dl_sotable[entry].dataorg = dl_info[1].dlinfo_dataorg;
      _dl_sotable[entry].handle = handle;
      _dl_sotable[entry].type = DL_GETSYM;
    }

  free (dl_info);
  return (void *) entry;
}
