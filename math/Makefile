# Copyright (C) 1991, 1992, 1994, 1995 Free Software Foundation, Inc.
# This file is part of the GNU C Library.

# The GNU C Library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public License as
# published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.

# The GNU C Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.

# You should have received a copy of the GNU Library General Public
# License along with the GNU C Library; see the file COPYING.LIB.  If
# not, write to the Free Software Foundation, Inc., 675 Mass Ave,
# Cambridge, MA 02139, USA.

#
#	Makefile for math.
#
subdir	:= math

headers	:= math.h __math.h huge_val.h nan.h


routines := acos asin atan cos sin tan cosh sinh tanh exp fabs ldexp	\
	    log log10 floor sqrt fmod frexp pow atan2 ceil modf		\
	    isinf isnan finite infnan copysign drem logb		\
	    rint hypot cabs cbrt expm1 log1p acosh asinh atanh		\
	    isinfl isnanl
tests := # test-math
install-lib := libm.a


include ../Rules

$(objpfx)libm.a: $(dep-dummy-lib); $(make-dummy-lib)
lib: $(objpfx)libm.a
