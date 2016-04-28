/* Definitions of functions to access `dev_t' values.
   Copyright (C) 2003-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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

#include <features.h>

#undef __USE_EXTERN_INLINES
#define __SYSMACROS_NEED_IMPLEMENTATION
#include <sys/sysmacros.h>

#define OUT_OF_LINE_IMPL_TEMPL(rtype, name, proto) \
  rtype gnu_dev_##name proto

__SYSMACROS_DEFINE_MAJOR(OUT_OF_LINE_IMPL_TEMPL)
__SYSMACROS_DEFINE_MINOR(OUT_OF_LINE_IMPL_TEMPL)
__SYSMACROS_DEFINE_MAKEDEV(OUT_OF_LINE_IMPL_TEMPL)
