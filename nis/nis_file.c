/* Copyright (c) 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Thorsten Kukuk <kukuk@vt.uni-paderborn.de>, 1997.

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rpcsvc/nis.h>
#include <rpcsvc/nislib.h>


static const char cold_start_file[] = "/var/nis/NIS_COLD_START";

directory_obj *
readColdStartFile (void)
{
  XDR xdrs;
  FILE *in;
  directory_obj obj;

  in = fopen (cold_start_file, "rb");
  if (in == NULL)
    {
      printf (_("Error while opening %s for reading: %m"), cold_start_file);
      return NULL;
    }
  memset (&obj, '\0', sizeof (obj));
  xdrstdio_create (&xdrs, in, XDR_DECODE);
  if (!xdr_directory_obj (&xdrs, &obj))
    {
      printf (_("Error while reading %s: %m"), cold_start_file);
      return NULL;
    }

  return nis_clone_directory (&obj, NULL);
}

bool_t
writeColdStartFile (const directory_obj *obj)
{
  XDR xdrs;
  FILE *out;

  out = fopen (cold_start_file, "wb");
  if (out == NULL)
    {
      printf (_("Error while opening %s for writing: %m"), cold_start_file);
      return FALSE;
    }

  xdrstdio_create (&xdrs, out, XDR_ENCODE);
  /* XXX The following cast is bad!  Shouldn't the XDR functions take
     pointers to const objects?  */
  if (!xdr_directory_obj (&xdrs, (directory_obj *) obj))
    {
      printf (_("Error while writing %s: %m"), cold_start_file);
      return FALSE;
    }

  return TRUE;
}

nis_object *
nis_read_obj (const char *name)
{
  XDR xdrs;
  FILE *in;
  nis_object obj;

  in = fopen (name, "rb");
  if (in == NULL)
    return NULL;

  memset (&obj, '\0', sizeof (obj));
  xdrstdio_create (&xdrs, in, XDR_DECODE);
  if (!xdr_nis_object (&xdrs, &obj))
    return NULL;

  return nis_clone_object (&obj, NULL);
}

bool_t
nis_write_obj (const char *name, const nis_object *obj)
{
  XDR xdrs;
  FILE *out;

  out = fopen (name, "wb");
  if (out == NULL)
    return FALSE;

  xdrstdio_create (&xdrs, out, XDR_ENCODE);
  /* XXX The following cast is bad!  Shouldn't the XDR functions take
     pointers to const objects?  */
  if (!xdr_nis_object (&xdrs, (nis_object *) obj))
    return FALSE;

  return TRUE;
}
