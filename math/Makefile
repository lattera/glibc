# Copyright (C) 1996,1997,1998,1999,2000,2001 Free Software Foundation, Inc.
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
# License along with the GNU C Library; see the file COPYING.LIB.  If not,
# write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA 02111-1307, USA.

# Makefile for the math library.

subdir		:= math

# Installed header files.
headers		:= math.h bits/mathcalls.h bits/mathinline.h bits/huge_val.h \
		   bits/nan.h fpu_control.h complex.h bits/cmathcalls.h \
		   fenv.h bits/fenv.h bits/fenvinline.h bits/mathdef.h tgmath.h

# Internal header files.
distribute	:= math_ldbl.h math_private.h machine/asm.h

# FPU support code.
aux		:= setfpucw fpu_control

# Build the -lm library.

extra-libs	:= libm
extra-libs-others = $(extra-libs)

libm-support = k_standard s_lib_version s_matherr s_signgam		\
	       fclrexcpt fgetexcptflg fraiseexcpt fsetexcptflg		\
	       ftestexcept fegetround fesetround fegetenv feholdexcpt	\
	       fesetenv feupdateenv t_exp fedisblxcpt feenablxcpt	\
	       fegetexcept
libm-calls = e_acos e_acosh e_asin e_atan2 e_atanh e_cosh e_exp e_fmod	\
	     e_hypot e_j0 e_j1 e_jn e_lgamma_r e_log e_log10 e_pow	\
	     e_rem_pio2 e_remainder e_scalb e_sinh e_sqrt e_gamma_r	\
	     k_cos k_rem_pio2 k_sin k_tan s_asinh s_atan s_cbrt		\
	     s_ceil s_copysign s_cos s_erf s_expm1 s_fabs s_finite	\
	     s_floor s_frexp s_ilogb s_ldexp s_log1p s_logb		\
	     s_modf s_nextafter s_nexttoward s_rint s_scalbn s_scalbln	\
	     s_significand s_sin s_tan s_tanh w_acos w_acosh w_asin	\
	     w_atan2 w_atanh w_cosh w_drem w_exp w_exp2 w_exp10 w_fmod	\
	     w_tgamma w_hypot w_j0 w_j1 w_jn w_lgamma w_lgamma_r	\
	     w_log w_log10 w_pow w_remainder w_scalb w_sinh w_sqrt	\
	     s_signbit s_fpclassify s_fmax s_fmin s_fdim s_nan s_trunc	\
	     s_remquo s_log2 e_exp2 s_round s_nearbyint s_sincos	\
	     conj cimag creal cabs carg s_cexp s_csinh s_ccosh s_clog	\
	     s_catan s_casin s_ccos s_csin s_ctan s_ctanh s_cacos	\
	     s_casinh s_cacosh s_catanh s_csqrt s_cpow s_cproj s_clog10 \
	     s_fma s_lrint s_llrint s_lround s_llround e_exp10
dbl-only-routines := branred doasin dosincos halfulp mpa mpatan2	\
		     mpatan mpexp mplog mpsqrt mptan sincos32 slowexp	\
		     slowpow
libm-routines = $(strip $(libm-support) $(libm-calls) \
			$(patsubst %_rf,%f_r,$(libm-calls:=f))	\
			$(long-m-$(long-double-fcts))) \
		$(dbl-only-routines)
long-m-routines = $(patsubst %_rl,%l_r,$(libm-calls:=l))
long-m-support = t_sincosl k_sincosl
long-m-yes = $(long-m-routines) $(long-m-support)
distribute += $(long-m-yes:=.c)

# These functions are in libc instead of libm because __printf_fp
# calls them, so any program using printf will need them linked in,
# and we don't want to have to link every program with -lm.
calls = s_isinf s_isnan s_finite s_copysign s_modf s_scalbn s_frexp s_ldexp \
	s_signbit
routines = $(calls) $(calls:=f) $(long-c-$(long-double-fcts))
long-c-yes = $(calls:=l)
distribute += $(long-c-yes:=.c)

include ../Makeconfig

# Rules for the test suite.
tests = test-matherr test-fenv atest-exp atest-sincos atest-exp2 basic-test \
	test-misc test-fpucw tst-definitions test-tgmath
# We do the `long double' tests only if this data type is available and
# distinct from `double'.
test-longdouble-yes = test-ldouble test-ildoubl
distribute += $(test-longdouble-yes:=.c)

ifneq (no,$(PERL))
libm-tests = test-float test-double $(test-longdouble-$(long-double-fcts)) \
	test-ifloat test-idouble
libm-tests.o = $(addsuffix .o,$(libm-tests))

tests += $(libm-tests)
libm-tests-generated = libm-test-ulps.h libm-test.c
generated += $(libm-tests-generated) libm-test.stmp

# This is needed for dependencies
before-compile += $(objpfx)libm-test.c

ulps-file = $(firstword $(wildcard $(config-sysdirs:%=$(..)%/libm-test-ulps)))

$(addprefix $(objpfx), $(libm-tests-generated)): $(objpfx)libm-test.stmp

$(objpfx)libm-test.stmp: $(ulps-file) libm-test.inc gen-libm-test.pl
	$(make-target-directory)
	$(PERL) gen-libm-test.pl -u $< -o "$(objpfx)"
	@echo > $@

$(objpfx)test-float.o: $(objpfx)libm-test.stmp
$(objpfx)test-ifloat.o: $(objpfx)libm-test.stmp
$(objpfx)test-double.o: $(objpfx)libm-test.stmp
$(objpfx)test-idouble.o: $(objpfx)libm-test.stmp
$(objpfx)test-ldouble.o: $(objpfx)libm-test.stmp
$(objpfx)test-ildoubl.o: $(objpfx)libm-test.stmp
endif

CFLAGS-test-float.c = -fno-inline -ffloat-store
CFLAGS-test-double.c = -fno-inline -ffloat-store
CFLAGS-test-ldouble.c = -fno-inline -ffloat-store
CFLAGS-test-ifloat.c = -U__LIBC_INTERNAL_MATH_INLINES -D__FAST_MATH__
CFLAGS-test-idouble.c = -U__LIBC_INTERNAL_MATH_INLINES -D__FAST_MATH__
CFLAGS-test-ildoubl.c = -U__LIBC_INTERNAL_MATH_INLINES -D__FAST_MATH__
CFLAGS-test-tgmath.c = -fno-builtin

distribute += libm-test.inc gen-libm-test.pl README.libm-test


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
CPPFLAGS += -D__NO_MATH_INLINES -D__LIBC_INTERNAL_MATH_INLINES

ifneq ($(long-double-fcts),yes)
# The `double' and `long double' types are the same on this machine.
# We won't compile the `long double' code at all.  Tell the `double' code
# to define aliases for the `FUNCl' names.  To avoid type conflicts in
# defining those aliases, tell <math.h> to declare the `FUNCl' names with
# `double' instead of `long double'.
CPPFLAGS += -DNO_LONG_DOUBLE -D_Mlong_double_=double
endif

# The fdlibm code generates a lot of these warnings but is otherwise clean.
override CFLAGS += -Wno-uninitialized

# The -lieee library is actually an object file.
# The module just defines the _LIB_VERSION_ variable.
# It's not a library to make sure it is linked in instead of s_lib_version.o.
$(objpfx)libieee.a: $(objpfx)ieee-math.o
	rm -f $@
	$(patsubst %/,cd % &&,$(objpfx)) \
	$(LN_S) $(<F) $(@F)

ifeq ($(build-shared),yes)
$(addprefix $(objpfx),$(tests)): $(objpfx)libm.so$(libm.so-version)
else
$(addprefix $(objpfx),$(tests)): $(objpfx)libm.a
endif
ifeq ($(build-bounded),yes)
$(tests:%=$(objpfx)%-bp): $(objpfx)libm_b.a
endif

ifeq ($(build-static),yes)
o = .o
else
o = .os
endif
gmp-objs = $(patsubst %,$(common-objpfx)stdlib/%$o,\
		      add_n sub_n cmp addmul_1 mul_1 mul_n divmod_1 \
		      lshift rshift mp_clz_tab udiv_qrnnd inlines)
$(objpfx)atest-exp: $(gmp-objs)
$(objpfx)atest-sincos: $(gmp-objs)
$(objpfx)atest-exp2: $(gmp-objs)

# Depend on libc.so so a DT_NEEDED is generated in the shared objects.
# This ensures they will load libc.so for needed symbols if loaded by
# a statically-linked program that hasn't already loaded it.
$(objpfx)libm.so: $(common-objpfx)libc.so $(common-objpfx)libc_nonshared.a
