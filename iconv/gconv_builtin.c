/* Table for builtin transformation mapping.
   Copyright (C) 1997, 1998 Free Software Foundation, Inc.
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

#include <limits.h>
#include <string.h>

#include <gconv_int.h>

#include <assert.h>


static struct builtin_map
{
  const char *name;
  gconv_fct fct;
  gconv_init_fct init;
  gconv_end_fct end;

} map[] =
{
#define BUILTIN_TRANSFORMATION(From, ConstPfx, ConstLen, To, Cost, Name, \
			       Fct, Init, End) \
  {									      \
    name: Name,								      \
    fct: Fct,								      \
    init: Init,								      \
    end: End,								      \
  },
#define BUILTIN_ALIAS(From, To)

#include <gconv_builtin.h>
};


void
internal_function
__gconv_get_builtin_trans (const char *name, struct gconv_step *step)
{
  size_t cnt;

  for (cnt = 0; cnt < sizeof (map) / sizeof (map[0]); ++cnt)
    if (strcmp (name, map[cnt].name) == 0)
      break;

  assert (cnt < sizeof (map) / sizeof (map[0]));

  step->fct = map[cnt].fct;
  step->init_fct = map[cnt].init;
  step->end_fct = map[cnt].end;
  step->counter = INT_MAX;
  step->shlib_handle = NULL;
}
