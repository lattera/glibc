/* Builtin transformations.
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

BUILTIN_ALIAS ("UCS4//", "ISO-10646/UCS4/")
BUILTIN_ALIAS ("UCS-4//", "ISO-10646/UCS4/")
BUILTIN_ALIAS ("ISO-10646//", "ISO-10646/UCS4/")
BUILTIN_ALIAS ("10646-1:1993//", "ISO-10646/UCS4/")
BUILTIN_ALIAS ("10646-1:1993/UCS4/", "ISO-10646/UCS4/")

BUILTIN_TRANSFORMATION (NULL, "INTERNAL", 8,
			"ISO-10646/UCS4/", 1, "=INTERNAL->ucs4",
			__gconv_transform_internal_ucs4, NULL, NULL)
BUILTIN_TRANSFORMATION (NULL, "ISO-10646/UCS4/", 15,
			"INTERNAL", 1, "=ucs4->INTERNAL",
			__gconv_transform_internal_ucs4, NULL, NULL)
/* Please note that we need only one function for both direction.  */

BUILTIN_ALIAS ("UTF8//", "ISO-10646/UTF8/")
BUILTIN_ALIAS ("UTF-8//", "ISO-10646/UTF8/")

BUILTIN_TRANSFORMATION (NULL, "INTERNAL", 8,
			"ISO-10646/UTF8/", 1, "=INTERNAL->utf8",
			__gconv_transform_internal_utf8, NULL, NULL)

BUILTIN_TRANSFORMATION ("ISO-10646/UTF-?8/", "ISO-10646/UTF", 13,
			"INTERNAL", 1, "=utf8->INTERNAL",
			__gconv_transform_utf8_internal, NULL, NULL)

BUILTIN_ALIAS ("UCS2//", "ISO-10646/UCS2/")
BUILTIN_ALIAS ("UCS-2//", "ISO-10646/UCS2/")

BUILTIN_TRANSFORMATION (NULL, "ISO-10646/UCS2/", 15, "INTERNAL",
			1, "=ucs2->INTERNAL",
			__gconv_transform_ucs2_internal, NULL, NULL)

BUILTIN_TRANSFORMATION (NULL, "INTERNAL", 8, "ISO-10646/UCS2/",
			1, "=INTERNAL->ucs2",
			__gconv_transform_internal_ucs2, NULL, NULL)

BUILTIN_TRANSFORMATION ("(.*)", NULL, 0, "\\1", 1, "=dummy",
			__gconv_transform_dummy, NULL, NULL)
