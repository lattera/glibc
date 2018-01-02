# Copyright (C) 1996-2018 Free Software Foundation, Inc.
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
# License along with the GNU C Library; if not, see
# <http://www.gnu.org/licenses/>.

# Makefile for the math library.

subdir		:= math

include ../Makeconfig

# Installed header files.
headers		:= math.h bits/mathcalls.h bits/mathinline.h \
		   fpu_control.h complex.h bits/cmathcalls.h fenv.h \
		   bits/fenv.h bits/fenvinline.h bits/mathdef.h tgmath.h \
		   bits/math-finite.h bits/math-vector.h \
		   bits/libm-simd-decl-stubs.h bits/iscanonical.h \
		   bits/flt-eval-method.h bits/fp-fast.h bits/fp-logb.h \
		   bits/long-double.h bits/mathcalls-helper-functions.h \
		   bits/floatn.h bits/floatn-common.h

# FPU support code.
aux		:= setfpucw fpu_control

# Build the -lm library.

extra-libs	:= libm
extra-libs-others = $(extra-libs)

libm-support = s_lib_version s_matherr s_signgam			\
	       fclrexcpt fgetexcptflg fraiseexcpt fsetexcptflg		\
	       ftestexcept fegetround fesetround fegetenv feholdexcpt	\
	       fesetenv feupdateenv t_exp fedisblxcpt feenablxcpt	\
	       fegetexcept fesetexcept fetestexceptflag fegetmode	\
	       fesetmode

# Wrappers for these functions generated per type using a file named
# <func>_template.c and the appropriate math-type-macros-<TYPE>.h.
gen-libm-calls = cargF conjF cimagF crealF cabsF s_cacosF		  \
	         s_cacoshF s_ccosF s_ccoshF s_casinF s_csinF s_casinhF	  \
		 k_casinhF s_csinhF k_casinhF s_csinhF s_catanhF s_catanF \
		 s_ctanF s_ctanhF s_cexpF s_clogF s_cprojF s_csqrtF	  \
		 s_cpowF s_clog10F s_fdimF s_nextdownF s_fmaxF s_fminF	  \
		 s_nanF s_iseqsigF s_canonicalizeF w_ilogbF w_llogbF	  \
		 w_log1pF w_scalblnF s_fmaxmagF s_fminmagF w_acosF	  \
		 w_acoshF w_asinF w_atan2F w_atanhF w_coshF w_exp10F	  \
		 w_exp2F w_fmodF w_hypotF w_j0F w_j1F w_jnF w_logF	  \
		 w_log10F w_log2F w_powF w_remainderF w_sinhF w_sqrtF	  \
		 w_tgammaF w_lgammaF w_lgammaF_r w_expF e_exp2F

libm-calls =								  \
	e_acosF e_acoshF e_asinF e_atan2F e_atanhF e_coshF e_expF e_fmodF \
	e_hypotF e_j0F e_j1F e_jnF e_lgammaF_r e_logF e_log10F e_powF	  \
	e_rem_pio2F e_remainderF e_scalbF e_sinhF e_sqrtF e_gammaF_r	  \
	e_ilogbF							  \
	k_cosF k_sinF k_tanF s_asinhF s_atanF s_cbrtF			  \
	s_ceilF s_cosF s_erfF s_expm1F s_fabsF				  \
	s_floorF s_log1pF s_logbF				  \
	s_nextafterF s_nexttowardF s_rintF s_scalblnF			  \
	s_significandF s_sinF s_tanF s_tanhF				  \
	s_fpclassifyF s_truncF						  \
	s_remquoF e_log2F s_roundF s_nearbyintF s_sincosF		  \
	s_fmaF s_lrintF s_llrintF s_lroundF s_llroundF e_exp10F		  \
	s_issignalingF $(calls:s_%=m_%) x2y2m1F				  \
	gamma_productF lgamma_negF lgamma_productF			  \
	s_nextupF s_totalorderF s_totalordermagF s_getpayloadF		  \
	s_setpayloadF s_setpayloadsigF s_roundevenF s_fromfpF s_ufromfpF  \
	s_fromfpxF s_ufromfpxF $(gen-libm-calls)

libm-compat-calls =							\
	w_acosF_compat w_acoshF_compat w_asinF_compat w_atan2F_compat	\
	w_atanhF_compat w_coshF_compat w_exp2F_compat w_exp10F_compat	\
	w_fmodF_compat w_hypotF_compat w_j0F_compat w_j1F_compat	\
	w_jnF_compat w_log2F_compat w_log10F_compat w_logF_compat	\
	w_powF_compat w_remainderF_compat w_scalbF_compat		\
	w_sinhF_compat w_sqrtF_compat w_tgammaF_compat			\
	w_lgammaF_r_compat w_lgammaF_compat2 w_expF_compat		\
	w_lgamma_compatF k_standardF

# Type specific routine support.
#
# The following three variables control what is included for each type:
#
# type-floatN-suffix = The suffix of the type
# type-floatN-routines = Type specific support objects
# type-floatN-yes = If the type is supported, evaluates to floatN
#
# Finally, note that types is an intentionally recursive variable.
# We only know the full set of supported types for the target machine
# after the Rules makefile has been parsed.
types-basic = $(type-ldouble-$(long-double-fcts)) double float

# Like types, but includes types whose functions alias those for
# another type.
test-types-basic = ldouble double float

# long double support
type-ldouble-suffix := l
type-ldouble-routines := t_sincosl k_sincosl s_iscanonicall
type-ldouble-yes := ldouble

# double support
type-double-suffix :=
type-double-routines := branred doasin dosincos halfulp mpa mpatan2	\
		       mpatan mpexp mplog mpsqrt mptan sincos32 slowexp	\
		       slowpow sincostab k_rem_pio2

# float support
type-float-suffix := f
type-float-routines := k_rem_pio2f math_errf e_exp2f_data e_logf_data	\
		       e_log2f_data e_powf_log2_data

# _Float128 support
type-float128-suffix := f128
type-float128-routines := t_sincosf128 k_sincosf128
type-float128-yes := float128

# _Float64x may be supported, only as an alias type.
type-float64x-yes := float64x

types = $(types-basic) $(type-float128-$(float128-fcts))
test-types = $(test-types-basic) $(type-float128-$(float128-fcts)) \
	     float32 float64 $(type-float128-$(float128-alias-fcts)) \
	     float32x $(type-float64x-$(float64x-alias-fcts))

# For each of the basic types (float, double, long double), replace the
# occurrences of 'F' in arg 1 with the appropriate suffix for the type.
type-basic-foreach = $(foreach t, $(types-basic), \
		       $(subst F,$(type-$(t)-suffix),$(1)))

# Apply suffix to each type in arg 1
type-foreach = $(foreach t,$(types),$(subst F,$(type-$(t)-suffix),$(1)))

libm-routines = $(strip $(libm-support)					\
			$(call type-basic-foreach,			\
			       $(libm-compat-calls))			\
			$(call type-foreach, $(libm-calls))		\
			$(foreach t, $(types), $(type-$(t)-routines))) 	\

# These functions are in libc instead of libm because __printf_fp
# calls them, so any program using printf will need them linked in,
# and we don't want to have to link every program with -lm.
# In libm-calls (above), list m_foo in place of s_foo for any
# routine that should be compiled separately for its libc and libm versions.
calls = s_isinfF s_isnanF s_finiteF s_copysignF s_modfF s_scalbnF s_frexpF \
	s_signbitF $(gen-calls)
gen-calls = s_ldexpF
generated += $(foreach s,.c .S,$(call type-foreach, $(calls:s_%=m_%$(s))))
routines = $(call type-foreach, $(calls))

ifeq ($(build-mathvec),yes)
# We need to install libm.so and libm.a as linker scripts
# for transparent use of vector math library.
install-lib-ldscripts := libm.so libm.a
install-others = $(inst_libdir)/libm.so $(inst_libdir)/libm.a
$(inst_libdir)/libm.so: $(common-objpfx)format.lds \
			$(libm) \
			$(libmvec) \
			$(+force)
	(echo '/* GNU ld script'; echo '*/';\
	 cat $<; \
	 echo 'GROUP ( $(slibdir)/libm.so$(libm.so-version) ' \
	      'AS_NEEDED ( $(libdir)/libmvec_nonshared.a $(slibdir)/libmvec.so$(libmvec.so-version) ) )' \
	) > $@.new
	mv -f $@.new $@

$(inst_libdir)/libm-$(version).a: $(objpfx)libm.a \
				  $(+force)
	$(do-install)

$(inst_libdir)/libm.a: $(common-objpfx)format.lds \
		       $(inst_libdir)/libm-$(version).a \
		       $(objpfx)../mathvec/libmvec.a \
		       $(+force)
	(echo '/* GNU ld script'; echo '*/';\
	 cat $<; \
	 echo 'GROUP ( $(libdir)/libm-$(version).a $(libdir)/libmvec.a )' \
	) > $@.new
	mv -f $@.new $@
endif

# Rules for the test suite.
tests = test-matherr-3 test-fenv basic-test \
	test-misc test-fpucw test-fpucw-ieee tst-definitions test-tgmath \
	test-tgmath-ret bug-nextafter bug-nexttoward bug-tgmath1 \
	test-tgmath-int test-tgmath2 test-powl tst-CMPLX tst-CMPLX2 test-snan \
	test-fenv-tls test-fenv-preserve test-fenv-return test-fenvinline \
	test-nearbyint-except test-fenv-clear test-signgam-finite \
	test-signgam-finite-c99 test-signgam-finite-c11 \
	test-nearbyint-except-2 test-signgam-uchar test-signgam-uchar-init \
	test-signgam-uint test-signgam-uint-init test-signgam-ullong \
	test-signgam-ullong-init test-nan-overflow test-nan-payload \
	test-fexcept test-fexcept-traps test-fesetexcept \
	test-fesetexcept-traps test-fetestexceptflag test-femode \
	test-femode-traps test-iszero-excess-precision \
	test-iseqsig-excess-precision test-flt-eval-method \
	test-fp-ilogb-constants test-fp-llogb-constants \
	test-fe-snans-always-signal test-finite-macros $(tests-static)
tests-static = test-fpucw-static test-fpucw-ieee-static \
	       test-signgam-uchar-static test-signgam-uchar-init-static \
	       test-signgam-uint-static test-signgam-uint-init-static \
	       test-signgam-ullong-static test-signgam-ullong-init-static
tests-internal = test-matherr test-matherr-2

# These tests use internal (unexported) GMP functions and are linked
# statically to obtain access to these functions.
tests-static += atest-exp atest-sincos atest-exp2

ifneq (,$(CXX))
tests += test-math-isinff test-math-iszero test-math-issignaling \
	 test-math-iscanonical test-math-cxx11 test-math-iseqsig
endif

ifneq (no,$(PERL))
libm-vec-tests = $(addprefix test-,$(libmvec-tests))
libm-test-support = $(foreach t,$(test-types),libm-test-support-$(t))
test-extras += $(libm-test-support)
extra-test-objs += $(addsuffix .o, $(libm-test-support))
libm-vec-test-wrappers = $(addsuffix -wrappers, $(libm-vec-tests))
test-extras += $(libm-vec-test-wrappers)
extra-test-objs += $(addsuffix .o, $(libm-vec-test-wrappers))

ulps-file = $(firstword $(wildcard $(sysdirs:%=%/libm-test-ulps)))

$(objpfx)libm-test-ulps.h: $(ulps-file) gen-libm-test.pl
	$(make-target-directory)
	$(PERL) gen-libm-test.pl -u $< -H $@

libm-test-funcs-auto = acos acosh asin asinh atan atan2 atanh cabs cacos \
		       cacosh carg casin casinh catan catanh cbrt ccos ccosh \
		       cexp clog clog10 cos cosh cpow csin csinh csqrt ctan \
		       ctanh erf erfc exp exp10 exp2 expm1 fma hypot j0 j1 jn \
		       lgamma log log10 log1p log2 pow sin sincos sinh sqrt \
		       tan tanh tgamma y0 y1 yn
libm-test-funcs-noauto = canonicalize ceil cimag conj copysign cproj creal \
			 fabs fdim floor fmax fmaxmag fmin fminmag fmod \
			 fpclassify frexp fromfp fromfpx getpayload ilogb \
			 iscanonical iseqsig isfinite isgreater \
			 isgreaterequal isinf isless islessequal \
			 islessgreater isnan isnormal issignaling issubnormal \
			 isunordered iszero llogb llrint llround logb lrint \
			 lround modf nearbyint nextafter nextdown nexttoward \
			 nextup remainder remquo rint round roundeven scalb \
			 scalbln scalbn setpayload setpayloadsig signbit \
			 significand totalorder totalordermag trunc ufromfp \
			 ufromfpx
libm-test-funcs-all = $(libm-test-funcs-auto) $(libm-test-funcs-noauto)
libm-test-c-auto = $(foreach f,$(libm-test-funcs-auto),libm-test-$(f).c)
libm-test-c-noauto = $(foreach f,$(libm-test-funcs-noauto),libm-test-$(f).c)
generated += libm-test-ulps.h $(libm-test-c-auto) $(libm-test-c-noauto)

libm-tests-base-normal = $(foreach t,$(test-types),test-$(t))
libm-tests-base-finite = $(foreach t,$(test-types),test-$(t)-finite)
libm-tests-base-inline = $(foreach t,$(test-types),test-i$(t))
libm-tests-base = $(libm-tests-base-normal) $(libm-tests-base-finite) \
		  $(libm-tests-base-inline) $(libm-vec-tests)
libm-tests-normal = $(foreach t,$(libm-tests-base-normal),\
				$(foreach f,$(libm-test-funcs-all),\
					    $(t)-$(f)))
libm-tests-finite = $(foreach t,$(libm-tests-base-finite),\
				$(foreach f,$(libm-test-funcs-all),\
					    $(t)-$(f)))
libm-tests-inline = $(foreach t,$(libm-tests-base-inline),\
				$(foreach f,$(libm-test-funcs-all),\
					    $(t)-$(f)))
libm-tests-vector = $(foreach t,$(libmvec-tests),\
				$(foreach f,$($(t)-funcs),test-$(t)-$(f)))
libm-tests = $(libm-tests-normal) $(libm-tests-finite) $(libm-tests-inline) \
	     $(libm-tests-vector)
libm-tests-for-type = $(foreach f,$(libm-test-funcs-all),\
				  test-$(1)-$(f) test-$(1)-finite-$(f) \
				  test-i$(1)-$(f)) \
		      $(filter test-$(1)-%,$(libm-tests-vector))

libm-tests.o = $(addsuffix .o,$(libm-tests))

tests += $(libm-tests)
generated += $(addsuffix .c,$(libm-tests)) \
	     $(foreach t,$(test-types),libm-test-support-$(t).c)

libm-test-c-auto-obj = $(addprefix $(objpfx),$(libm-test-c-auto))
libm-test-c-noauto-obj = $(addprefix $(objpfx),$(libm-test-c-noauto))

$(libm-test-c-noauto-obj): $(objpfx)libm-test%.c: libm-test%.inc \
						  gen-libm-test.pl
	$(make-target-directory)
	$(PERL) gen-libm-test.pl -c $< -a /dev/null -C $@

$(libm-test-c-auto-obj): $(objpfx)libm-test%.c: libm-test%.inc \
						gen-libm-test.pl \
						auto-libm-test-out%
	$(make-target-directory)
	$(PERL) gen-libm-test.pl -c $< -a auto-libm-test-out$* -C $@

libm-test-incs = $(foreach f,$(libm-test-funcs-all),libm-test-$(f).inc)
endif

ifdef PYTHON
tests += test-tgmath3
generated += test-tgmath3.c
CFLAGS-test-tgmath3.c += -fno-builtin

$(objpfx)test-tgmath3.c: gen-tgmath-tests.py
	$(PYTHON) $< > $@
endif

libm-test-fast-math-cflags = -fno-builtin -D__FAST_MATH__ -DTEST_FAST_MATH
libm-test-vec-cflags = $(libm-test-fast-math-cflags) -fno-inline \
		       -ffloat-store -D_OPENMP=201307 -Wno-unknown-pragmas

CFLAGS-test-double-vlen4-wrappers.c += $(double-vlen4-arch-ext-cflags)

CFLAGS-test-double-vlen8-wrappers.c += $(double-vlen8-arch-ext-cflags)

CFLAGS-test-float-vlen8-wrappers.c += $(float-vlen8-arch-ext-cflags)

CFLAGS-test-float-vlen16-wrappers.c += $(float-vlen16-arch-ext-cflags)

# The no-inline tests use -fsignaling-nans, and thereby use the
# versions of classification macros that support sNaNs.  The inline
# function tests use the versions of classification macros that may
# raise spurious exceptions for sNaNs, but also do not test for
# exceptions.  Thus both versions of the classification macros are
# validated.
libm-test-no-inline-cflags = -fno-inline -ffloat-store -fno-builtin \
			     -fsignaling-nans
libm-test-finite-cflags = $(libm-test-no-inline-cflags) \
			  -U__FINITE_MATH_ONLY__ -D__FINITE_MATH_ONLY__=1
libm-test-inline-cflags = $(libm-test-fast-math-cflags)
CFLAGS-test-tgmath.c += -fno-builtin
# The following testcase uses very long lines (>3 million), so it may take a
# while to compile it. See: http://llvm.org/bugs/show_bug.cgi?id=14106 and
# http://gcc.gnu.org/bugzilla/show_bug.cgi?id=54402
CFLAGS-test-tgmath2.c += -fno-builtin
CFLAGS-test-tgmath-ret.c += -fno-builtin
CFLAGS-test-powl.c += -fno-builtin

CFLAGS-test-snan.c += -fsignaling-nans

CFLAGS-test-signgam-finite.c += -ffinite-math-only
CFLAGS-test-signgam-finite-c99.c += -ffinite-math-only -std=c99
CFLAGS-test-signgam-finite-c11.c += -ffinite-math-only -std=c11
CFLAGS-test-signgam-uchar.c += -std=c99
CFLAGS-test-signgam-uchar-init.c += -std=c99
CFLAGS-test-signgam-uchar-static.c += -std=c99
CFLAGS-test-signgam-uchar-init-static.c += -std=c99
CFLAGS-test-signgam-uint.c += -std=c99
CFLAGS-test-signgam-uint-init.c += -std=c99
CFLAGS-test-signgam-uint-static.c += -std=c99
CFLAGS-test-signgam-uint-init-static.c += -std=c99
CFLAGS-test-signgam-ullong.c += -std=c99
CFLAGS-test-signgam-ullong-init.c += -std=c99
CFLAGS-test-signgam-ullong-static.c += -std=c99
CFLAGS-test-signgam-ullong-init-static.c += -std=c99

CFLAGS-test-math-cxx11.cc += -std=c++11

CFLAGS-test-math-isinff.cc += -std=gnu++11
CFLAGS-test-math-iszero.cc += -std=gnu++11
CFLAGS-test-math-issignaling.cc += -std=gnu++11
CFLAGS-test-math-iscanonical.cc += -std=gnu++11

CFLAGS-test-iszero-excess-precision.c += -fexcess-precision=standard
CFLAGS-test-iseqsig-excess-precision.c += -fexcess-precision=standard
CFLAGS-test-flt-eval-method.c += -fexcess-precision=standard

CFLAGS-test-fe-snans-always-signal.c += -fsignaling-nans

CFLAGS-test-finite-macros.c += -ffinite-math-only

include ../Rules

gen-all-calls = $(gen-libm-calls) $(gen-calls)

generated += $(addsuffix .c,$(call type-foreach,$(gen-all-calls))) \
	     gen-libm-templates.stmp

# Create wrappers in the math build directory.
$(objpfx)gen-libm-templates.stmp: Makefile
	$(make-target-directory)
	for gcall in $(gen-all-calls); do                                 \
	  func=$${gcall%F*}$${gcall#*F};                                  \
	  for type in $(foreach t,$(types),$(t)__$(type-$(t)-suffix)); do \
	    suff=$${type#*__};                                            \
	    type=$${type%__*};                                            \
	    file=$(objpfx)$${gcall%F*}$${suff}$${gcall#*F}.c;             \
	    (                                                             \
	      echo "#include <math-type-macros-$${type}.h>";              \
	      echo "#include <$${func}_template.c>";                      \
	    ) > $${file};                                                 \
	  done;                                                           \
	done;                                                             \
	echo > $(@)

# Add dependency to ensure the generator runs prior.
$(foreach t, $(call type-foreach, $(gen-all-calls)), \
             $(objpfx)$(t).c): $(objpfx)gen-libm-templates.stmp

ifneq (no,$(PERL))
# This must come after the inclusion of sysdeps Makefiles via Rules.

$(foreach t,$(libm-tests-normal),$(objpfx)$(t).c): $(objpfx)test-%.c:
	type_func=$*; \
	type=$${type_func%%-*}; \
	func=$${type_func#*-}; \
	( \
	  echo "#include <test-$$type.h>"; \
	  echo "#include <test-math-no-finite.h>"; \
	  echo "#include <test-math-no-inline.h>"; \
	  echo "#include <test-math-errno.h>"; \
	  echo "#include <test-math-scalar.h>"; \
	  echo "#include <libm-test-$$func.c>"; \
	) > $@

$(foreach t,$(libm-tests-finite),$(objpfx)$(t).c): $(objpfx)test-%.c:
	type_func=$*; \
	type=$${type_func%%-*}; \
	func=$${type_func#*-finite-}; \
	( \
	  echo "#include <test-$$type.h>"; \
	  echo "#include <test-math-finite.h>"; \
	  echo "#include <test-math-no-inline.h>"; \
	  echo "#include <test-math-scalar.h>"; \
	  echo "#include <libm-test-$$func.c>"; \
	) > $@

$(foreach t,$(libm-tests-inline),$(objpfx)$(t).c): $(objpfx)test-i%.c:
	type_func=$*; \
	type=$${type_func%%-*}; \
	func=$${type_func#*-}; \
	( \
	  echo "#include <test-$$type.h>"; \
	  echo "#include <test-math-inline.h>"; \
	  echo "#include <test-math-scalar.h>"; \
	  echo "#include <libm-test-$$func.c>"; \
	) > $@

$(foreach t,$(libm-tests-vector),$(objpfx)$(t).c): $(objpfx)test-%.c:
	type_func=$*; \
	type=$${type_func%-*}; \
	func=$${type_func##*-}; \
	func_args=$$(grep ALL_RM_TEST libm-test-$$func.inc \
		     | sed 's/.*RUN_TEST_LOOP_//;s/_.*//'); \
	( \
	  echo "#include <test-$$type.h>"; \
	  echo "WRAPPER_DECL_$$func_args (WRAPPER_NAME (FUNC ($$func)))"; \
	  echo "#include <libm-test-$$func.c>"; \
	) > $@

$(foreach t,$(test-types),\
	    $(objpfx)libm-test-support-$(t).c): $(objpfx)libm-test-support-%.c:
	( \
	  echo "#include <test-$*.h>"; \
	  echo "#include <libm-test-support.c>"; \
	) > $@

$(addprefix $(objpfx), $(libm-tests.o)): $(objpfx)libm-test-ulps.h

define o-iterator-doit
$(foreach f,$(libm-test-funcs-all),\
	    $(objpfx)$(o)-$(f).o): $(objpfx)$(o)%.o: $(objpfx)libm-test%.c
endef
object-suffixes-left := $(libm-tests-base)
include $(o-iterator)

define o-iterator-doit
$(foreach f,$(libm-test-funcs-all),\
	    $(objpfx)$(o)-$(f).o): CFLAGS += $(libm-test-no-inline-cflags)
endef
object-suffixes-left := $(libm-tests-base-normal)
include $(o-iterator)

define o-iterator-doit
$(foreach f,$(libm-test-funcs-all),\
	    $(objpfx)$(o)-$(f).o): CFLAGS += $(libm-test-finite-cflags)
endef
object-suffixes-left := $(libm-tests-base-finite)
include $(o-iterator)

define o-iterator-doit
$(foreach f,$(libm-test-funcs-all),\
	    $(objpfx)$(o)-$(f).o): CFLAGS += $(libm-test-inline-cflags)
endef
object-suffixes-left := $(libm-tests-base-inline)
include $(o-iterator)

define o-iterator-doit
$(foreach f,$($(o)-funcs),\
	    $(objpfx)test-$(o)-$(f).o): CFLAGS += $(libm-test-vec-cflags)
endef
object-suffixes-left := $(libmvec-tests)
include $(o-iterator)

define o-iterator-doit
$(addprefix $(objpfx),\
	    $(call libm-tests-for-type,$(o))): $(objpfx)libm-test-support-$(o).o
endef
object-suffixes-left := $(test-types)
include $(o-iterator)

define o-iterator-doit
$(objpfx)libm-test-support-$(o).o: CFLAGS += $(libm-test-no-inline-cflags)
endef
object-suffixes-left := $(test-types)
include $(o-iterator)

# Run the math programs to automatically generate ULPs files.
.PHONY: regen-ulps

run-regen-ulps = $(test-wrapper-env) \
	    $(run-program-env) \
	    $($*-ENV) $(rtld-prefix) $(objpfx)$${run}

regen-ulps: $(addprefix $(objpfx),$(libm-tests))
	rm -f $(objpfx)ULPs; rm -f $(objpfx)NewUlps; \
	cp $(ulps-file) $(objpfx)libm-test-ulps; \
	for run in $(libm-tests); do \
	  echo "Regenerating ULPs for $${run}"; \
	  $(run-regen-ulps) -u -o $(objpfx); \
	  cat $(objpfx)ULPs >> $(objpfx)libm-test-ulps; \
	  rm $(objpfx)ULPs; \
	done; \
	$(PERL) gen-libm-test.pl -n $(objpfx)NewUlps \
				 -u $(objpfx)libm-test-ulps; \
	echo "Automatic regeneration of ULPs complete."; \
	echo "Difference between the current baseline and the new baseline is:";\
	diff -urN $(ulps-file) $(objpfx)NewUlps; \
	echo "Copy $(objpfx)NewUlps to $(ulps-file) (relative to source)."
else
regen-ulps:
	@echo "Automatic regeneration of ULPs requires perl."; \
	exit 1;
endif

# The generated sysd-rules file defines rules like this for sources
# coming from sysdeps/ directories.  These rules find the generic sources.
define o-iterator-doit
$(objpfx)m_%$o: s_%.c $(before-compile); $$(compile-command.c)
endef
object-suffixes-left := $(all-object-suffixes)
include $(o-iterator)

# Likewise, for those generated files shared with libc.
define o-iterator-doit
$(objpfx)m_%$o: $(objpfx)s_%.c $(before-compile); $$(compile-command.c)
endef
object-suffixes-left := $(all-object-suffixes)
include $(o-iterator)

# We don't want the fdlibm code to use the inline math functions,
# only the fdlibm code.
math-CPPFLAGS += -D__NO_MATH_INLINES -D__LIBC_INTERNAL_MATH_INLINES

ifneq ($(long-double-fcts),yes)
# The `double' and `long double' types are the same on this machine.
# We won't compile the `long double' code at all.  Tell the `double' code
# to define aliases for the `FUNCl' names.
math-CPPFLAGS += -DNO_LONG_DOUBLE
endif

# These files quiet sNaNs in a way that is optimized away without
# -fsignaling-nans.
CFLAGS-s_modf.c += -fsignaling-nans
CFLAGS-s_modff.c += -fsignaling-nans
CFLAGS-s_modfl.c += -fsignaling-nans
CFLAGS-s_modff128.c += -fsignaling-nans

$(addprefix $(objpfx),\
	    $(filter-out $(tests-static) $(libm-tests-vector),\
			 $(tests) $(tests-internal))): $(libm)
$(addprefix $(objpfx),$(tests-static)): $(objpfx)libm.a
define o-iterator-doit
$(foreach f,$($(o)-funcs),\
	    $(objpfx)test-$(o)-$(f)): $(objpfx)test-$(o)-wrappers.o \
				      $(libm) $(libmvec)
endef
object-suffixes-left := $(libmvec-tests)
include $(o-iterator)

$(objpfx)test-fenv-tls: $(shared-thread-library)
