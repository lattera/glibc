/* __frame_state_for unwinder helper function wrapper.
   Copyright (C) 2001-2012 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jakub Jelinek <jakub@redhat.com>, 2001.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <dlfcn.h>
#include <stdlib.h>
#define STATIC static
#define __frame_state_for fallback_frame_state_for
#include <unwind-dw2.c>
#undef __frame_state_for
#include <gnu/lib-names.h>

typedef struct frame_state * (*framesf)(void *pc, struct frame_state *);
struct frame_state *__frame_state_for (void *pc,
				       struct frame_state *frame_state);

struct frame_state *
__frame_state_for (void *pc, struct frame_state *frame_state)
{
  static framesf frame_state_for;

  if (frame_state_for == NULL)
    {
      void *handle = __libc_dlopen (LIBGCC_S_SO);

      if (handle == NULL
	  || (frame_state_for
	      = (framesf) __libc_dlsym (handle, "__frame_state_for")) == NULL)
#ifndef __USING_SJLJ_EXCEPTIONS__
	frame_state_for = fallback_frame_state_for;
#else
      	frame_state_for = abort;
#endif
    }

  return frame_state_for (pc, frame_state);
}
