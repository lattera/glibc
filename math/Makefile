# Makefile for the math library.

# Copyright (C) 1996 Free Software Foundation, Inc.
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

subdir		:= math

# Installed header files.
headers		:= math.h mathcalls.h __math.h huge_val.h nan.h fpu_control.h

# Internal header files.
distribute	:= math_private.h machine/asm.h machine/endian.h

# FPU support code.
aux		:= fpu_control setfpucw

# Build the -lm library.

extra-libs	:= libm

libm-support = k_standard s_lib_version s_matherr s_signgam
libm-calls = e_acos e_acosh e_asin e_atan2 e_atanh e_cosh e_exp e_fmod	\
	     e_hypot e_j0 e_j1 e_jn e_lgamma_r e_log e_log10 e_pow	\
	     e_rem_pio2 e_remainder e_scalb e_sinh e_sqrt k_cos		\
	     k_rem_pio2 k_sin k_tan s_asinh s_atan s_cbrt		\
	     s_ceil s_copysign s_cos s_erf s_expm1 s_fabs s_finite	\
	     s_floor s_frexp s_ilogb s_ldexp s_log1p s_logb 		\
	     s_modf s_nextafter s_rint s_scalbn s_significand		\
	     s_sin s_tan s_tanh w_acos w_acosh w_asin			\
	     w_atan2 w_atanh w_cabs w_cosh w_drem w_exp w_fmod w_gamma	\
	     w_gamma_r w_hypot w_j0 w_j1 w_jn w_lgamma w_lgamma_r	\
	     w_log w_log10 w_pow w_remainder w_scalb w_sinh w_sqrt
libm-routines = $(libm-support) $(libm-calls) \
		$(patsubst %_rf,%f_r,$(libm-calls:=f))	\
		$(long-m-$(long-double-fcts))
long-m-routines = $(patsubst %_rl,%l_r,$(libm-calls:=l)) # not ready yet
#long-m-yes = $(long-m-routines) # uncomment this when code works
#distribute += $(long-m-routines:=.c) # and this when at least all files exist

# These functions are in libc instead of libm because __printf_fp
# calls them, so any program using printf will need them linked in,
# and we don't want to have to link every program with -lm.
calls = s_isinf s_isnan s_frexp s_ldexp s_modf
routines = $(calls) $(calls:=f) $(long-c-$(long-double-fcts))
long-c-yes = $(calls:=l)
distribute += $(long-c-yes:=.c)


# The -lieee module sets the _LIB_VERSION_ switch to IEEE mode
# for error handling in the -lm functions.
install-lib += libieee.a
non-lib.a += libieee.a
extra-objs += libieee.a ieee-math.o
distribute += ieee-math.c

include ../Rules


# This file defines the default _LIB_VERSION variable that controls
# the error return conventions for the math functions.
CPPFLAGS-s_lib_version.c := -D_POSIX_MODE

# We don't want the fdlibm code to use the inline math functions,
# only the fdlibm code.
CPPFLAGS += -D__NO_MATH_INLINES

ifneq ($(long-double-fcts),yes)
# The `double' and `long double' types are the same on this machine.
# We won't compile the `long double' code at all.  Tell the `double' code
# to define aliases for the `FUNCl' names.
CPPFLAGS += -DNO_LONG_DOUBLE
endif

# The fdlibm code generates a lot of these warnings but is otherwise clean.
override CFLAGS += -Wno-uninitialized -Wno-write-strings

# The -lieee library is actually an object file.
# The module just defines the _LIB_VERSION_ variable.
# It's not a library to make sure it is linked in instead of s_lib_version.o.
$(objpfx)libieee.a: $(objpfx)ieee-math.o
	rm -f $@
	ln $< $@
