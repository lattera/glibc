/* Release any resource associated with given conversion descriptor.
   Copyright (C) 1997, 1998, 1999 Free Software Foundation, Inc.
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

#include <stdlib.h>

#include <gconv_int.h>


int
internal_function
__gconv_close (gconv_t cd)
{
  struct gconv_step *srunp;
  struct gconv_step_data *drunp;
  size_t nsteps;

  /* Free all resources by calling destructor functions and release
     the implementations.  */
  srunp = cd->steps;
  nsteps = cd->nsteps;
  drunp = cd->data;
  do
    {
      if (!drunp->is_last && drunp->outbuf != NULL)
	free (drunp->outbuf);
    }
  while (!(drunp++)->is_last);

  /* Free the data allocated for the descriptor.  */
  free (cd);

  /* Close the participating modules.  */
  return __gconv_close_transform (srunp, nsteps);
}
