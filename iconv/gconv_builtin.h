/* Builtin transformations.
   Copyright (C) 1997 Free Software Foundation, Inc.
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

BUILTIN_TRANSFORMATION ("\\([^/]+\\)/UCS4/\\([^/]*\\)", NULL, 0,
			"\\1/UTF8/\\2", 1, "=ucs4->utf8",
			__gconv_transform_ucs4_utf8,
			__gconv_transform_init_rstate,
			__gconv_transform_end_rstate)

BUILTIN_TRANSFORMATION ("\\([^/]+\\)/UTF8/\\([^/]*\\)", NULL, 0,
			"\\1/UCS4/\\2", 1, "=utf8->ucs4",
			__gconv_transform_utf8_ucs4,
			__gconv_transform_init_rstate,
			__gconv_transform_end_rstate)

BUILTIN_TRANSFORMATION ("\\(.*\\)", NULL, 0, "\\1", 1, "=dummy",
			__gconv_transform_dummy, NULL, NULL)
