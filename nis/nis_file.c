/* Copyright (c) 1997, 1998, 1999, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Thorsten Kukuk <kukuk@suse.de>, 1997.

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rpcsvc/nis.h>
#include "nis_xdr.h"

static const char cold_start_file[] = "/var/nis/NIS_COLD_START";

directory_obj *
readColdStartFile (void)
{
  FILE *in = fopen (cold_start_file, "rc");
  if (in == NULL)
    return NULL;

  directory_obj *obj = calloc (1, sizeof (directory_obj));

  if (obj != NULL)
    {
      XDR xdrs;
      xdrstdio_create (&xdrs, in, XDR_DECODE);
      bool_t status = _xdr_directory_obj (&xdrs, obj);
      xdr_destroy (&xdrs);

      if (!status)
	{
	  nis_free_directory (obj);
	  obj = NULL;
	}
    }

  fclose (in);

  return obj;
}
libnsl_hidden_def (readColdStartFile)

bool_t
writeColdStartFile (const directory_obj *obj)
{
  XDR xdrs;
  FILE *out;
  bool_t status;

  out = fopen (cold_start_file, "wb");
  if (out == NULL)
    return FALSE;

  xdrstdio_create (&xdrs, out, XDR_ENCODE);
  status = _xdr_directory_obj (&xdrs, (directory_obj *) obj);
  xdr_destroy (&xdrs);
  fclose (out);

  return status;
}

nis_object *
nis_read_obj (const char *name)
{
  XDR xdrs;
  FILE *in;
  bool_t status;
  nis_object *obj;

  in = fopen (name, "rb");
  if (in == NULL)
    return NULL;

  obj = calloc (1, sizeof (nis_object));
  if (obj == NULL)
    {
      fclose (in);
      return NULL;
    }

  xdrstdio_create (&xdrs, in, XDR_DECODE);
  status =_xdr_nis_object (&xdrs, obj);
  xdr_destroy (&xdrs);
  fclose (in);

  if (status)
    return obj;
  else
    {
      nis_free_object (obj);
      return NULL;
    }
}

bool_t
nis_write_obj (const char *name, const nis_object *obj)
{
  XDR xdrs;
  FILE *out;
  bool_t status;

  out = fopen (name, "wb");
  if (out == NULL)
    return FALSE;

  xdrstdio_create (&xdrs, out, XDR_ENCODE);
  status = _xdr_nis_object (&xdrs, (nis_object *) obj);
  xdr_destroy (&xdrs);
  fclose (out);

  return status;
}
