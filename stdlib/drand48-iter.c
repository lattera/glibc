/* Copyright (C) 1995 Free Software Foundation, Inc.
This file is part of the GNU C Library.
Contributed by Ulrich Drepper <drepper@gnu.ai.mit.edu>, August 1995.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#include <errno.h>
#include <stdlib.h>


/* Global state for non-reentrent functions.  */
struct drand48_data __libc_drand48_data;


int
__drand48_iterate (xsubi, buffer)
     unsigned short int xsubi[3];
     struct drand48_data *buffer;
{
  /* Be generous for the arguments, detect some errors.  */
  if (xsubi == NULL || buffer == NULL)
    {
      errno = EFAULT;
      return -1;
    }

  /* Initialize buffer, if not yet done.  */
  if (!buffer->init)
    {
      if (sizeof (unsigned short int) == 2)
	{
	  buffer->a[2] = 0x5;
	  buffer->a[1] = 0xdeec;
	  buffer->a[0] = 0xe66d;
	}
      else
	{
	  buffer->a[2] = 0x5deecUL;
	  buffer->a[1] = 0xe66d0000UL;
	  buffer->a[0] = 0;
	}
      buffer->c = 0xb;
      buffer->init = 1;
    }

  /* Do the real work.  We choose a data type which contains at least
     48 bits.  Because we compute the modulus it does not care how
     many bits really are computed.  */

  if (sizeof (long int) >= 6)
    {
      /* The `long' data type is sufficent.  */
      unsigned long int X, a, result;

#define ONE_STEP							    \
      if (sizeof (unsigned short int) == 2)				    \
	{								    \
	  X = (xsubi[2] << 16 | xsubi[1]) << 16 | xsubi[0];		    \
	  a = (buffer->a[2] << 16 | buffer->a[1]) << 16 | buffer->a[0];	    \
									    \
	  result = X * a + buffer->c;					    \
									    \
	  xsubi[0] = result & 0xffff;					    \
	  result >>= 16;						    \
	  xsubi[1] = result & 0xffff;					    \
	  result >>= 16;						    \
	  xsubi[2] = result & 0xffff;					    \
	}								    \
      else								    \
	{								    \
	  X = xsubi[2] << 16 | xsubi[1] >> 16;				    \
	  a = buffer->a[2] << 16 | buffer->a[1] >> 16;			    \
									    \
	  result = X * a + buffer->c;					    \
									    \
	  xsubi[0] = result >> 16 & 0xffffffffl;			    \
	  xsubi[1] = result << 16 & 0xffff0000l;			    \
	}
      ONE_STEP;
    }
  else
    {
      /* We have to use the `long long' data type.  */
      unsigned long long int X, a, result;
      ONE_STEP;
    }

  return 0;
}
