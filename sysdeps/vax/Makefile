# Copyright (C) 1991, 1994, 1997 Free Software Foundation, Inc.
# This file is part of the GNU C Library.

# The GNU C Library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.

# The GNU C Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.

# You should have received a copy of the GNU Lesser General Public
# License along with the GNU C Library; if not, write to the Free
# Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
# 02111-1307 USA.

ifeq	($(subdir),math)
ifndef	math-twiddled

elided-routines := $(elided-routines) acos asin cos sin ceil rint hypot \
		   __copysign __scalb __drem __logb __finite
sysdep_routines := $(sysdep_routines) asincos sincos argred \
		   support exp__E log__L

math-twiddled := t
endif

bsdmath_dirs := $(bsdmath_dirs) vax

endif
