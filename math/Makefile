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
headers		:= math.h mathcalls.h __math.h huge_val.h nan.h

# Internal header files.
distribute	:= math_private.h machine/asm.h machine/endian.h


# Build the -lm library.

extra-libs	:= libm
libm.so-version	:= .0
libm-routines	:= e_acos e_acosf e_acosh e_acoshf e_asin e_asinf	\
		   e_atan2 e_atan2f e_atanh e_atanhf e_cosh e_coshf	\
		   e_exp e_expf e_fmod e_fmodf e_hypot e_hypotf e_j0	\
		   e_j0f e_j1 e_j1f e_jn e_jnf e_lgamma_r e_lgammaf_r	\
		   e_log e_log10 e_log10f e_logf e_pow e_powf		\
		   e_rem_pio2 e_rem_pio2f e_remainder e_remainderf	\
		   e_scalb e_scalbf e_sinh e_sinhf e_sqrt e_sqrtf k_cos	\
		   k_cosf k_rem_pio2 k_rem_pio2f k_sin k_sinf		\
		   k_standard k_tan k_tanf s_asinh s_asinhf s_atan	\
		   s_atanf s_cbrt s_cbrtf s_ceil s_ceilf s_copysign	\
		   s_copysignf s_cos s_cosf s_erf s_erff s_expm1	\
		   s_expm1f s_fabs s_fabsf s_finite s_finitef s_floor	\
		   s_floorf s_frexp s_frexpf s_ilogb s_ilogbf		\
		   s_ldexp s_ldexpf s_lib_version s_log1p s_log1pf	\
		   s_logb s_logbf s_matherr s_modf s_modff s_nextafter	\
		   s_nextafterf s_rint s_rintf s_scalbn s_scalbnf	\
		   s_signgam s_significand s_significandf s_sin s_sinf	\
		   s_tan s_tanf s_tanh s_tanhf w_acos w_acosf w_acosh	\
		   w_acoshf w_asin w_asinf w_atan2 w_atan2f w_atanh	\
		   w_atanhf w_cabs w_cabsf w_cosh w_coshf w_drem	\
		   w_dremf w_exp w_expf w_fmod w_fmodf w_gamma		\
		   w_gamma_r w_gammaf w_gammaf_r w_hypot w_hypotf w_j0	\
		   w_j0f w_j1 w_j1f w_jn w_jnf w_lgamma w_lgamma_r	\
		   w_lgammaf w_lgammaf_r w_log w_log10 w_log10f w_logf	\
		   w_pow w_powf w_remainder w_remainderf w_scalb	\
		   w_scalbf w_sinh w_sinhf w_sqrt w_sqrtf

# These functions are in libc instead of libm because __printf_fp
# calls them, so any program using printf will need them linked in,
# and we don't want to have to link every program with -lm.
routines := s_isinf s_isinff s_isnan s_isnanf isinfl isnanl


include ../Rules


# This file defines the default _LIB_VERSION variable that controls
# the error return conventions for the math functions.
CPPFLAGS-s_lib_version.c := -D_POSIX_MODE

# The fdlibm code generates a lot of these warnings but is otherwise clean.
override CFLAGS += -Wno-uninitialized -Wno-write-strings
